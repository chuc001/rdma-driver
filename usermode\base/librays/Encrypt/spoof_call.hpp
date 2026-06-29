#include <windows.h>
#include <vector>
#include <map>
#include <random>
#include <type_traits>

namespace SpoofCall
{
    struct SectionInfo {
        DWORD virtualAddress;
        DWORD virtualSize;
        DWORD characteristics;
    };
    std::vector<SectionInfo> GetExecutableSections(HMODULE module) {
        std::vector<SectionInfo> sections;
        auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
        auto ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(module) + dosHeader->e_lfanew);
        auto section = IMAGE_FIRST_SECTION(ntHeader);
        for (WORD i = 0; i < ntHeader->FileHeader.NumberOfSections; i++, section++) {
            if (!(section->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) &&
                (section->Characteristics & IMAGE_SCN_MEM_EXECUTE)) {
                sections.push_back({
                    section->VirtualAddress,
                    section->Misc.VirtualSize,
                    section->Characteristics
                    });
            }
        }
        return sections;
    }
    std::map<int32_t, std::vector<uintptr_t>> FindReturnAddresses(HMODULE module, const std::vector<SectionInfo>& sections) {
        std::map<int32_t, std::vector<uintptr_t>> returnAddresses;
        MEMORY_BASIC_INFORMATION mbi;
        std::random_device rd;
        std::mt19937 gen(rd());

        for (const auto& section : sections) {
            auto address = reinterpret_cast<uintptr_t>(module) + section.virtualAddress;
            auto sectionEnd = address + section.virtualSize;

            while (address < sectionEnd) {
                if (VirtualQuery(reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi)) != sizeof(mbi)) {
                    break;
                }

                if (mbi.State == MEM_COMMIT &&
                    (mbi.Protect == PAGE_EXECUTE_READ ||
                        mbi.Protect == PAGE_EXECUTE_READWRITE ||
                        mbi.Protect == PAGE_EXECUTE_WRITECOPY)) {

                    auto page = reinterpret_cast<uint8_t*>(mbi.BaseAddress);
                    for (size_t i = 0; i < mbi.RegionSize - 5; i++) {
                        if (page[i] == 0x48 && page[i + 1] == 0x83 &&
                            page[i + 2] == 0xC4 && page[i + 4] == 0xC3) {
                            int32_t stackAdjustment = page[i + 3];
                            returnAddresses[stackAdjustment].push_back(
                                reinterpret_cast<uintptr_t>(page + i)
                            );
                        }
                    }
                }
                address = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
            }
        }
        return returnAddresses;
    }
    struct CallStackData {
        std::vector<uintptr_t> fakeStack;
        std::vector<uintptr_t> returnAddresses;
    };
    CallStackData Initialize(HMODULE module, size_t maxFakeStack = 16) {
        CallStackData result;
        auto sections = GetExecutableSections(module);
        auto returnAddresses = FindReturnAddresses(module, sections);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uintptr_t> dist(0, reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(module) + reinterpret_cast<PIMAGE_DOS_HEADER>(module)->e_lfanew)->OptionalHeader.SizeOfImage);
        std::vector<int32_t> validKeys;
        for (const auto& [stackAdj, addresses] : returnAddresses) {
            if (stackAdj / sizeof(uintptr_t) < 10 && (stackAdj / sizeof(uintptr_t)) % 2 == 1) {
                validKeys.push_back(stackAdj);
                result.returnAddresses.push_back(addresses[gen() % addresses.size()]);
            }
        }
        while (result.fakeStack.size() < maxFakeStack) {
            auto stackAdj = validKeys[gen() % validKeys.size()];
            auto& addresses = returnAddresses[stackAdj];
            result.fakeStack.push_back(addresses[gen() % addresses.size()]);

            size_t paramCount = stackAdj / sizeof(uintptr_t);
            for (size_t i = 0; i < paramCount; i++) {
                result.fakeStack.push_back(reinterpret_cast<uintptr_t>(module) + dist(gen));
            }
        }
        return result;
    }
    template<typename T>
    uintptr_t ConvertToUintptr(T arg) {
        if constexpr (std::is_same_v<T, bool>) {
            return static_cast<uintptr_t>(arg ? 1 : 0); // Convert bool to 0 or 1
        }
        else if constexpr (std::is_pointer_v<T> || std::is_integral_v<T>) {
            return reinterpret_cast<uintptr_t>(arg);
        }
        else {
            static_assert(std::is_pointer_v<T> || std::is_integral_v<T>, "Unsupported argument type");
            return 0;
        }
    }
    template<typename Ret, typename... Args>
    Ret SpoofCall(void* function, Args... args) {
        static CallStackData callData;
        static bool initialized = false;
        static HMODULE module = safe_call(GetModuleHandleA)(nullptr);
        if (!initialized) {
            callData = Initialize(module);
            initialized = true;
        }
        std::vector<uintptr_t> stackFrame;
        stackFrame.reserve(16);
        (stackFrame.push_back(ConvertToUintptr(args)), ...);
        while (stackFrame.size() % 2 != 0) {
            stackFrame.push_back(0);
        }
        for (const auto& addr : callData.fakeStack) {
            stackFrame.push_back(addr);
        }
        auto returnAddr = callData.returnAddresses[
            std::min<size_t>(stackFrame.size() / 2, callData.returnAddresses.size() - 1)
        ];
        auto stackSize = stackFrame.size() * sizeof(uintptr_t);
        auto* newStack = static_cast<uintptr_t*>(_alloca(stackSize + sizeof(uintptr_t)));
        newStack[stackSize / sizeof(uintptr_t)] = returnAddr;
        std::memcpy(newStack, stackFrame.data(), stackSize);
        auto* funcPtr = reinterpret_cast<Ret(*)(Args...)>(function);
        return funcPtr(args...);
    }
}
