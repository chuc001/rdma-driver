#pragma once
#include <ntifs.h>

namespace DrvRdma {
namespace Memory {

class VirtualMemory
{
public:
    static ULONG64 TranslateLinear(ULONG64 directory_base, ULONG64 address);
    static ULONGLONG GetDirbaseFromBaseAddress(PVOID base);
    static ULONG GetWindowsVersion();
    static uintptr_t GetProcessCr3(PEPROCESS process);
};

} // namespace Memory
} // namespace DrvRdma
