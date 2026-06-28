#include "../../include/memory/virtual_memory.h"
#include "../../include/memory/memory_types.h"
#include "../../include/memory/physical_memory.h"

extern "C" {
    NTKERNELAPI PPHYSICAL_MEMORY_RANGE NTAPI MmGetPhysicalMemoryRanges(VOID);
}

namespace DrvRdma {
namespace Memory {

ULONG64 VirtualMemory::TranslateLinear(ULONG64 directory_base, ULONG64 address)
{
    directory_base &= ~0xf;
    
    auto virt_addr = address & ~(~0ul << 12);
    auto pte = ((address >> 12) & (0x1ffll));
    auto pt = ((address >> 21) & (0x1ffll));
    auto pd = ((address >> 30) & (0x1ffll));
    auto pdp = ((address >> 39) & (0x1ffll));
    auto p_mask = ((~0xfull << 8) & 0xfffffffffull);
    
    SIZE_T read_size = 0;
    ULONG64 pdpe = 0;
    PhysicalMemory::Read(reinterpret_cast<PVOID>(directory_base + 8 * pdp), &pdpe, sizeof(pdpe), &read_size);
    if (~pdpe & 1)
        return 0;

    ULONG64 pde = 0;
    PhysicalMemory::Read(reinterpret_cast<PVOID>((pdpe & p_mask) + 8 * pd), &pde, sizeof(pde), &read_size);
    if (~pde & 1)
        return 0;
    
    if (pde & 0x80)
        return (pde & (~0ull << 42 >> 12)) + (address & ~(~0ull << 30));

    ULONG64 pte_addr = 0;
    PhysicalMemory::Read(reinterpret_cast<PVOID>((pde & p_mask) + 8 * pt), &pte_addr, sizeof(pte_addr), &read_size);
    if (~pte_addr & 1)
        return 0;
    
    if (pte_addr & 0x80)
        return (pte_addr & p_mask) + (address & ~(~0ull << 21));

    address = 0;
    PhysicalMemory::Read(reinterpret_cast<PVOID>((pte_addr & p_mask) + 8 * pte), &address, sizeof(address), &read_size);
    address &= p_mask;
    
    if (!address)
        return 0;
    
    return address + virt_addr;
}

ULONGLONG VirtualMemory::GetDirbaseFromBaseAddress(PVOID base)
{
    if (!PhysicalMemory::GetMmPfnDatabase() && !NT_SUCCESS(PhysicalMemory::InitializeMmPfnDatabase()))
    {
        return 0;
    }

    VirtAddr virt_base;
    virt_base.value = base;
    SIZE_T read_size;
    
    PPHYSICAL_MEMORY_RANGE ranges = MmGetPhysicalMemoryRanges();
    if (!ranges)
        return 0;

    for (int i = 0; ; i++)
    {
        PPHYSICAL_MEMORY_RANGE elem = &ranges[i];
        if (!elem->BaseAddress.QuadPart || !elem->NumberOfBytes.QuadPart)
            break;

        ULONG64 current_phys = elem->BaseAddress.QuadPart;
        for (int j = 0; j < (elem->NumberOfBytes.QuadPart / 0x1000); j++, current_phys += 0x1000)
        {
            MmPfn* pfn_info = reinterpret_cast<MmPfn*>(
                reinterpret_cast<ULONG64>(PhysicalMemory::GetMmPfnDatabase()) + 
                (current_phys >> 12) * sizeof(MmPfn)
            );

            if (pfn_info->u4.PteFrame == (current_phys >> 12))
            {
                MmPte pml4e = { 0 };
                if (!NT_SUCCESS(PhysicalMemory::Read(
                    reinterpret_cast<PVOID>(current_phys + 8 * virt_base.pml4_index), 
                    &pml4e, 8, &read_size)))
                    continue;
                
                if (!pml4e.u.Hard.Valid)
                    continue;

                MmPte pdpte = { 0 };
                if (!NT_SUCCESS(PhysicalMemory::Read(
                    reinterpret_cast<PVOID>((pml4e.u.Hard.PageFrameNumber << 12) + 8 * virt_base.pdpt_index),
                    &pdpte, 8, &read_size)))
                    continue;
                
                if (!pdpte.u.Hard.Valid)
                    continue;

                MmPte pde = { 0 };
                if (!NT_SUCCESS(PhysicalMemory::Read(
                    reinterpret_cast<PVOID>((pdpte.u.Hard.PageFrameNumber << 12) + 8 * virt_base.pd_index),
                    &pde, 8, &read_size)))
                    continue;
                
                if (!pde.u.Hard.Valid)
                    continue;

                MmPte pte = { 0 };
                if (!NT_SUCCESS(PhysicalMemory::Read(
                    reinterpret_cast<PVOID>((pde.u.Hard.PageFrameNumber << 12) + 8 * virt_base.pt_index),
                    &pte, 8, &read_size)))
                    continue;
                
                if (!pte.u.Hard.Valid)
                    continue;

                return current_phys;
            }
        }
    }
    
    return 0;
}

ULONG VirtualMemory::GetWindowsVersion()
{
    RTL_OSVERSIONINFOW ver = { 0 };
    ver.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    RtlGetVersion(&ver);

    switch (ver.dwBuildNumber)
    {
    case 17134:
    case 17763:
        return 0x0278;
    case 18362:
    case 18363:
        return 0x0280;
    case 19041:
    case 19042:
    case 19043:
    case 19044:
    case 19045:
        return 0x0388;
    case 22000:
        return 0x0388;
    case 22621:
    case 22631:
    case 26100:
        return 0x0388;
    default:
        if (ver.dwBuildNumber >= 22000)
            return 0x0388;
        return 0x0388;
    }
}

uintptr_t VirtualMemory::GetProcessCr3(PEPROCESS pprocess)
{
    if (!pprocess)
        return 0;

    uintptr_t process_dirbase = *reinterpret_cast<uintptr_t*>(
        reinterpret_cast<UINT8*>(pprocess) + 0x28
    );

    if (process_dirbase == 0)
    {
        ULONG user_diroffset = GetWindowsVersion();
        process_dirbase = *reinterpret_cast<uintptr_t*>(
            reinterpret_cast<UINT8*>(pprocess) + user_diroffset
        );
    }

    process_dirbase &= ~0xFFF;
    return process_dirbase;
}

} // namespace Memory
} // namespace DrvRdma
