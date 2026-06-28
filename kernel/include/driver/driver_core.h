#pragma once
#include <ntifs.h>
#include "../common/types.h"

namespace DrvRdma {
namespace Driver {

class DriverCore
{
public:
    static NTSTATUS Initialize(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
    static VOID Unload(PDRIVER_OBJECT DriverObject);
    static Common::DriverContext* GetContext();
    static PVOID GetLegitStartAddress();

private:
    static VOID AcceptThread(PVOID Context);
    static VOID ProcessRequestThread(PVOID Context);
    static VOID AcceptThreadWrapper(PVOID Context);
    static VOID ProcessRequestThreadWrapper(PVOID Context);

    static Common::DriverContext s_Context;
    static PVOID s_LegitStartAddress;
};

} // namespace Driver
} // namespace DrvRdma
