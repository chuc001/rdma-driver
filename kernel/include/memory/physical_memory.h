#pragma once
#include <ntifs.h>

namespace DrvRdma {
namespace Memory {

class PhysicalMemory
{
public:
    static NTSTATUS Read(PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_read);
    static NTSTATUS Write(PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_written);
    static PVOID FindPattern(PVOID base, SIZE_T size, const UCHAR* pattern, SIZE_T pattern_size);
    static NTSTATUS InitializeMmPfnDatabase();
    static PVOID GetMmPfnDatabase();

private:
    static PVOID s_MmPfnDatabase;
};

} // namespace Memory
} // namespace DrvRdma
