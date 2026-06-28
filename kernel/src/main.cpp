#include "../include/driver/driver_core.h"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    return DrvRdma::Driver::DriverCore::Initialize(DriverObject, RegistryPath);
}
