#include "../../include/memory/physical_memory.h"
#include "../../include/memory/memory_types.h"

extern "C" {
    NTKERNELAPI PVOID NTAPI MmGetVirtualForPhysical(PHYSICAL_ADDRESS PhysicalAddress);
    NTKERNELAPI PVOID NTAPI MmMapIoSpaceEx(PHYSICAL_ADDRESS PhysicalAddress, SIZE_T NumberOfBytes, ULONG Protect);
    NTKERNELAPI VOID NTAPI MmUnmapIoSpace(PVOID BaseAddress, SIZE_T NumberOfBytes);
    NTKERNELAPI NTSTATUS NTAPI MmCopyMemory(PVOID TargetAddress, MM_COPY_ADDRESS SourceAddress, SIZE_T NumberOfBytes, ULONG Flags, PSIZE_T NumberOfBytesTransferred);
}

namespace DrvRdma {
namespace Memory {

PVOID PhysicalMemory::s_MmPfnDatabase = nullptr;

NTSTATUS PhysicalMemory::Read(PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_read)
{
    MM_COPY_ADDRESS to_read = { 0 };
    to_read.PhysicalAddress.QuadPart = reinterpret_cast<LONGLONG>(target_address);
    return MmCopyMemory(buffer, to_read, size, MM_COPY_MEMORY_PHYSICAL, bytes_read);
}

NTSTATUS PhysicalMemory::Write(PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_written)
{
    if (!target_address)
        return STATUS_UNSUCCESSFUL;

    PHYSICAL_ADDRESS to_write = { 0 };
    to_write.QuadPart = reinterpret_cast<LONGLONG>(target_address);
    
    PVOID mapped_memory = MmMapIoSpaceEx(to_write, size, PAGE_READWRITE);
    if (!mapped_memory)
        return STATUS_UNSUCCESSFUL;

    RtlCopyMemory(mapped_memory, buffer, size);
    *bytes_written = size;
    
    MmUnmapIoSpace(mapped_memory, size);
    return STATUS_SUCCESS;
}

PVOID PhysicalMemory::FindPattern(PVOID base, SIZE_T size, const UCHAR* pattern, SIZE_T pattern_size)
{
    const UCHAR* search_base = static_cast<const UCHAR*>(base);
    
    for (SIZE_T i = 0; i <= size - pattern_size; i++)
    {
        SIZE_T j;
        for (j = 0; j < pattern_size; j++)
        {
            if (search_base[i + j] != pattern[j])
                break;
        }
        if (j == pattern_size)
            return reinterpret_cast<PVOID>(const_cast<UCHAR*>(&search_base[i]));
    }
    
    return nullptr;
}

NTSTATUS PhysicalMemory::InitializeMmPfnDatabase()
{
    static const UCHAR pattern[] = {
        0x48, 0x8B, 0xC1,
        0x48, 0xC1, 0xE8, 0x0C,
        0x48, 0x8D, 0x14, 0x40,
        0x48, 0x03, 0xD2,
        0x48, 0xB8
    };

    UCHAR* func = reinterpret_cast<UCHAR*>(MmGetVirtualForPhysical);
    if (!func)
        return STATUS_PROCEDURE_NOT_FOUND;

    UCHAR* found = static_cast<UCHAR*>(FindPattern(func, 0x20, pattern, sizeof(pattern)));
    if (!found)
        return STATUS_UNSUCCESSFUL;

    found += sizeof(pattern);
    s_MmPfnDatabase = *reinterpret_cast<PVOID*>(found);
    s_MmPfnDatabase = PAGE_ALIGN(s_MmPfnDatabase);
    
    return STATUS_SUCCESS;
}

PVOID PhysicalMemory::GetMmPfnDatabase()
{
    return s_MmPfnDatabase;
}

} // namespace Memory
} // namespace DrvRdma
