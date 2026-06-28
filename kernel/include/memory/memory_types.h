#pragma once
#include <ntifs.h>

namespace DrvRdma {
namespace Memory {

union VirtAddr
{
    PVOID value;
    struct
    {
        ULONG64 offset : 12;
        ULONG64 pt_index : 9;
        ULONG64 pd_index : 9;
        ULONG64 pdpt_index : 9;
        ULONG64 pml4_index : 9;
        ULONG64 reserved : 16;
    };
};

struct MmPteHardware
{
    struct
    {
        ULONG64 Valid : 1;
        ULONG64 Dirty1 : 1;
        ULONG64 Owner : 1;
        ULONG64 WriteThrough : 1;
        ULONG64 CacheDisable : 1;
        ULONG64 Accessed : 1;
        ULONG64 Dirty : 1;
        ULONG64 LargePage : 1;
        ULONG64 Global : 1;
        ULONG64 CopyOnWrite : 1;
        ULONG64 Unused : 1;
        ULONG64 Write : 1;
        ULONG64 PageFrameNumber : 40;
        ULONG64 ReservedForSoftware : 4;
        ULONG64 WsleAge : 4;
        ULONG64 WsleProtection : 3;
        ULONG64 NoExecute : 1;
    };
};

struct MmPte
{
    union
    {
        ULONG64 Long;
        MmPteHardware Hard;
    } u;
};

struct MmPfn
{
    UCHAR padding[0x28];
    union
    {
        struct
        {
            ULONG64 PteFrame : 40;
            ULONG64 ResidentPage : 1;
            ULONG64 Unused1 : 1;
            ULONG64 Unused2 : 1;
            ULONG64 Partition : 10;
            ULONG64 FileOnly : 1;
            ULONG64 PfnExists : 1;
            ULONG64 NodeFlinkHigh : 5;
            ULONG64 PageIdentity : 3;
            ULONG64 PrototypePte : 1;
        };
        ULONG64 EntireField;
    } u4;
};

} // namespace Memory
} // namespace DrvRdma
