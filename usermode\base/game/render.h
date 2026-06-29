#pragma once
#include <vector>
#include <atomic>
#include <thread>
#include <unordered_map>

// ===== ESTRUTURAS MINIMALISTAS =====
struct PedScreenData {
    float x, y;
    bool valid;
    uintptr_t pedAddr;  // **ADICIONE ISSO**

};

struct GameCache {
    uintptr_t pWorld;
    uintptr_t pLocalPlayer;
    uintptr_t pReplayInterFace;
    uintptr_t pViewPort;
    uintptr_t pCamGamePlayDirector;
    uintptr_t LocalPlayerAddr;

    PedScreenData peds[50];
    int pedCount;
    bool ready;
};

GameCache g_Cache[2];
std::atomic<int> g_ActiveBuffer{ 0 };
std::atomic<bool> g_Running{ true };

constexpr int MAX_PEDS = 50;
constexpr int MAX_READS_PER_FRAME = 15; // **AUMENTADO**: 15 leituras para atualização mais frequente

// ===== CACHE COM INTERPOLAÇÃO =====
struct PedCacheData {
    bool isPlayer;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 lastPos;
    int framesSinceUpdate;
    bool posValid;
};

std::unordered_map<uintptr_t, PedCacheData> g_PedCache;
int g_RoundRobinOffset = 0;

// Interpolação linear simples
inline D3DXVECTOR3 LerpPosition(const D3DXVECTOR3& from, const D3DXVECTOR3& to, float t) {
    D3DXVECTOR3 result;
    result.x = from.x + (to.x - from.x) * t;
    result.y = from.y + (to.y - from.y) * t;
    result.z = from.z + (to.z - from.z) * t;
    return result;
}

// ===== THREAD DE LEITURA COM INTERPOLAÇÃO =====
void memory_reader_thread() {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    int writeBuffer = 1;

    uintptr_t pedAddresses[100];

    while (g_Running.load(std::memory_order_relaxed)) {
        GameCache& cache = g_Cache[writeBuffer];
        cache.ready = false;

        cache.pWorld = client.read<uintptr_t>(offsets::m_World);
        cache.pReplayInterFace = client.read<uintptr_t>(offsets::m_ReplayInterFace);
        cache.pViewPort = client.read<uintptr_t>(offsets::m_ViewPort);
        cache.pCamGamePlayDirector = client.read<uintptr_t>(offsets::m_CamGameplayDirector);

        pointers::pWorld = (CPedFactory*)cache.pWorld;
        pointers::pReplayInterFace = (CReplayInterFace*)cache.pReplayInterFace;
        pointers::pViewPort = cache.pViewPort;
        pointers::pCamGamePlayDirector = cache.pCamGamePlayDirector;

        if (cache.pWorld) {
            cache.pLocalPlayer = client.read<uintptr_t>(cache.pWorld);
            if (!cache.pLocalPlayer) {
                cache.pLocalPlayer = client.read<uintptr_t>(cache.pWorld + 0x8);
            }
            pointers::pLocalPlayer = (CPed*)cache.pLocalPlayer;
            cache.LocalPlayerAddr = cache.pLocalPlayer;
        }

        if (!cache.pReplayInterFace || !cache.pLocalPlayer) {
            cache.pedCount = 0;
            cache.ready = true;
            g_ActiveBuffer.store(writeBuffer, std::memory_order_release);
            writeBuffer = 1 - writeBuffer;
            continue;
        }

        uintptr_t ReplayAddr = cache.pReplayInterFace;
        uintptr_t PedInterfaceAddr = client.read<uintptr_t>(ReplayAddr + 0x18);

        if (!PedInterfaceAddr) {
            cache.pedCount = 0;
            cache.ready = true;
            g_ActiveBuffer.store(writeBuffer, std::memory_order_release);
            writeBuffer = 1 - writeBuffer;
            continue;
        }

        uintptr_t PedListAddr = client.read<uintptr_t>(PedInterfaceAddr + 0x100);
        int MaxPeds = client.read<int>(PedInterfaceAddr + 0x108);

        if (!PedListAddr || MaxPeds <= 0 || MaxPeds > 300) {
            cache.pedCount = 0;
            cache.ready = true;
            g_ActiveBuffer.store(writeBuffer, std::memory_order_release);
            writeBuffer = 1 - writeBuffer;
            continue;
        }

        // Batch read dos endereços
        int pedsToRead = min(MaxPeds, 100);
        if (!client.ReadMemory((PVOID)PedListAddr, pedAddresses, pedsToRead * sizeof(uintptr_t))) {
            cache.pedCount = 0;
            cache.ready = true;
            g_ActiveBuffer.store(writeBuffer, std::memory_order_release);
            writeBuffer = 1 - writeBuffer;
            continue;
        }

        // **RENDERIZA COM INTERPOLAÇÃO**
        int pedIndex = 0;
        for (int i = 0; i < pedsToRead && pedIndex < MAX_PEDS; i++) {
            uintptr_t pedAddr = pedAddresses[i];

            if (pedAddr < 0x10000 || pedAddr == cache.LocalPlayerAddr) {
                continue;
            }

            auto cacheIt = g_PedCache.find(pedAddr);
            if (cacheIt != g_PedCache.end()) {
                PedCacheData& cached = cacheIt->second;

                // Usa cache apenas se for player e posição válida
                if (cached.isPlayer && cached.posValid && cached.framesSinceUpdate < 6) {
                    // **INTERPOLAÇÃO**: Suaviza movimento entre frames
                    float t = min(cached.framesSinceUpdate / 3.0f, 1.0f); // Interpola em 3 frames
                    D3DXVECTOR3 interpolatedPos = LerpPosition(cached.lastPos, cached.pos, t);

                    D3DXVECTOR2 screen = WorldToScreen(interpolatedPos);

                  
                    // Dentro do loop: "**RENDERIZA COM INTERPOLAÇÃO**"
                    if (screen.x > 0 && screen.y > 0) {
                        cache.peds[pedIndex].x = screen.x;
                        cache.peds[pedIndex].y = screen.y;
                        cache.peds[pedIndex].pedAddr = pedAddr;  // **ADICIONE ISSO**
                        cache.peds[pedIndex].valid = true;
                        pedIndex++;
                    }

                }

                cached.framesSinceUpdate++;
            }
        }

        // **ATUALIZA 15 PEDS POR FRAME**
        int readsThisFrame = 0;
        for (int loop = 0; loop < pedsToRead && readsThisFrame < MAX_READS_PER_FRAME; loop++) {
            int i = (g_RoundRobinOffset + loop) % pedsToRead;
            uintptr_t pedAddr = pedAddresses[i];

            if (pedAddr < 0x10000 || pedAddr == cache.LocalPlayerAddr) {
                continue;
            }

            auto cacheIt = g_PedCache.find(pedAddr);

            // Atualiza se necessário (cache antigo ou novo ped)
            if (cacheIt != g_PedCache.end()) {
                // Só atualiza se tiver 3+ frames desde última atualização
                if (cacheIt->second.framesSinceUpdate < 3) {
                    continue;
                }
            }

            bool needsTypeCheck = (cacheIt == g_PedCache.end());
            bool isPlayer = false;

            if (needsTypeCheck) {
                // Lê EntityType
                uint32_t entityType = client.read<uint32_t>(pedAddr + offsets::m_EntityType);
                uint32_t PedType = (entityType << 11) >> 25;
                readsThisFrame++;

                isPlayer = (PedType == 2);

                // Cria entrada no cache
                if (cacheIt == g_PedCache.end()) {
                    PedCacheData newEntry;
                    newEntry.isPlayer = isPlayer;
                    newEntry.posValid = false;
                    newEntry.framesSinceUpdate = 0;
                    newEntry.lastPos = D3DXVECTOR3(0, 0, 0);
                    newEntry.pos = D3DXVECTOR3(0, 0, 0);
                    g_PedCache[pedAddr] = newEntry;
                    cacheIt = g_PedCache.find(pedAddr);
                }

                if (!isPlayer) {
                    continue;
                }
            }
            else {
                isPlayer = cacheIt->second.isPlayer;
                if (!isPlayer) {
                    continue;
                }
            }

            // Lê nova posição
            D3DXVECTOR3 newPos = client.read<D3DXVECTOR3>(pedAddr + 0x90);
            readsThisFrame++;

            if (newPos.x != 0 || newPos.y != 0 || newPos.z != 0) {
                // **SALVA POSIÇÃO ANTERIOR PARA INTERPOLAÇÃO**
                cacheIt->second.lastPos = cacheIt->second.pos;
                cacheIt->second.pos = newPos;
                cacheIt->second.posValid = true;
                cacheIt->second.framesSinceUpdate = 0;
            }
        }

        // Incrementa offset
        g_RoundRobinOffset = (g_RoundRobinOffset + 15) % pedsToRead;

        cache.pedCount = pedIndex;
        cache.ready = true;

        g_ActiveBuffer.store(writeBuffer, std::memory_order_release);
        writeBuffer = 1 - writeBuffer;
    }
}

// ===== RENDERIZAÇÃO OTIMIZADA =====
void draw_esp_optimized() {
    int activeIdx = g_ActiveBuffer.load(std::memory_order_acquire);
    const GameCache& cache = g_Cache[activeIdx];

    if (!cache.ready || cache.pedCount == 0) {
        return;
    }

    ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

    static const ImU32 COL_RED = IM_COL32(255, 0, 0, 255);
    static const float RADIUS = 6.0f;

    int count = cache.pedCount;
    for (int i = 0; i < count; i++) {
        DrawList->AddCircleFilled(ImVec2(cache.peds[i].x, cache.peds[i].y), RADIUS, COL_RED, 6);
    }

    static int debugFrameCount = 0;
    if (++debugFrameCount % 60 == 0) {
        printf("[ESP] PEDs: %d | Cached: %zu\n", count, g_PedCache.size());
    }
}

void draw_esp() {
    draw_esp_optimized();
}
inline CPed* GetClosestPed()
{
    int activeIdx = g_ActiveBuffer.load(std::memory_order_acquire);
    const GameCache& cache = g_Cache[activeIdx];

    if (!cache.ready || cache.pedCount == 0) {
        return nullptr;
    }

    static float screenCenterX = (float)GetSystemMetrics(SM_CXSCREEN) / 2.0f;
    static float screenCenterY = (float)GetSystemMetrics(SM_CYSCREEN) / 2.0f;

    CPed* ClosestPed = nullptr;
    float ClosestDist = FLT_MAX;

    for (int i = 0; i < cache.pedCount; i++) {
        const PedScreenData& ped = cache.peds[i];

        if (!ped.valid) continue;

        float dx = ped.x - screenCenterX;
        float dy = ped.y - screenCenterY;
        float distSq = (dx * dx) + (dy * dy);  // **Evita sqrt desnecessário**

        if (distSq < ClosestDist) {
            ClosestDist = distSq;
            ClosestPed = (CPed*)ped.pedAddr;
        }
    }

    // Verifica MaxDistance apenas no final (usando sqrt uma vez)
    return ClosestPed;


    return nullptr;
}
