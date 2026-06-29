#pragma once
#include <mutex>
#include <thread>
#include <vector>
#include <unordered_map>
#include <iostream>
void update_pointer()
{
    while (true)
    {
        pointers::pWorld = client.read<CPedFactory*>(offsets::m_World);
        pointers::pLocalPlayer = pointers::pWorld->GetLocalPlayer();
        pointers::pReplayInterFace = client.read<CReplayInterFace*>(offsets::m_ReplayInterFace);
        pointers::pViewPort = client.read<uintptr_t>(offsets::m_ViewPort);
        pointers::pCamGamePlayDirector = client.read<uintptr_t>(offsets::m_CamGameplayDirector);

        std::cout << "pointers::pWorld 0x" << std::hex << pointers::pWorld << std::endl;
        std::cout << "pointers::pLocalPlayer 0x" << std::hex << pointers::pLocalPlayer << std::endl;
        std::cout << "pointers::pReplayInterFace 0x" << std::hex << pointers::pReplayInterFace << std::endl;
        std::cout << "pointers::pViewPort 0x" << std::hex << pointers::pViewPort << std::endl;
        std::cout << "pointers::pCamGamePlayDirector 0x" << std::hex << pointers::pCamGamePlayDirector << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}


namespace Core
{
    namespace Threads
    {
        std::mutex EntityListMutex;
        std::vector<EntityStruct> EntityList;

        D3DXVECTOR3 GetBonePos(uintptr_t pedAddr, int boneIndex)
        {
            D3DXMATRIX Matrix = client.read<D3DXMATRIX>(pedAddr + 0x60);
            D3DXVECTOR3 BonePos = client.read<D3DXVECTOR3>(pedAddr + (offsets::CurrentBuild >= 2802 ? 0x410 : 0x430) + (boneIndex * 0x10));

            D3DXVECTOR4 Transform;
            D3DXVec3Transform(&Transform, &BonePos, &Matrix);
            return D3DXVECTOR3(Transform.x, Transform.y, Transform.z);
        }

        void Update()
        {
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                if (!pointers::pReplayInterFace || !pointers::pLocalPlayer)
                    continue;

                uintptr_t ReplayAddr = (uintptr_t)pointers::pReplayInterFace;
                uintptr_t PedInterfaceAddr = client.read<uintptr_t>(ReplayAddr + 0x18);

                if (!PedInterfaceAddr)
                    continue;

                uintptr_t PedListAddr = client.read<uintptr_t>(PedInterfaceAddr + 0x100);

                if (!PedListAddr)
                    continue;

                int MaxPeds = client.read<int>(PedInterfaceAddr + 0x108);

                if (MaxPeds <= 0 || MaxPeds > 300)
                    continue;

                std::vector<EntityStruct> TempList;
                uintptr_t LocalPlayerAddr = (uintptr_t)pointers::pLocalPlayer;
                D3DXVECTOR3 LocalPos = client.read<D3DXVECTOR3>(LocalPlayerAddr + 0x90);

                for (int i = 0; i < MaxPeds; i++)
                {
                    uintptr_t PedAddr = client.read<uintptr_t>(PedListAddr + (i * 0x10));

                    if (!PedAddr || PedAddr == LocalPlayerAddr || PedAddr < 0x10000 || PedAddr == 0xFFFFFFFFFFFFFFFF)
                        continue;

                    EntityStruct Entity;
                    Entity.PedAddr = PedAddr;
                    Entity.Id = i;
                    Entity.Index = i;
                    Entity.Pos = client.read<D3DXVECTOR3>(PedAddr + 0x90);
                    Entity.Health = client.read<float>(PedAddr + 0x280);
                    Entity.MaxHealth = client.read<float>(PedAddr + offsets::m_MaxHealth);
                    Entity.Armor = client.read<float>(PedAddr + offsets::m_Armor);
                    Entity.PedType = client.read<uint32_t>(PedAddr + offsets::m_EntityType) << 11 >> 25;

                    float dx = Entity.Pos.x - LocalPos.x;
                    float dy = Entity.Pos.y - LocalPos.y;
                    Entity.Distance = std::sqrtf(dx * dx + dy * dy);

                    Entity.HeadBone = GetBonePos(PedAddr, 0);
                    Entity.PelvisBone = GetBonePos(PedAddr, 8);

                    TempList.push_back(Entity);
                }

                std::cout << "[DEBUG] Entidades: " << TempList.size() << std::endl;

                {
                    std::lock_guard<std::mutex> Guard(EntityListMutex);
                    EntityList = TempList;
                }
            }
        }
    }
}
