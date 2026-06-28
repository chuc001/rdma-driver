#include "../../include/utils/system_utils.h"

namespace DrvRdma {
namespace Utils {

PVOID SystemUtils::GetSystemRoutineAddress(LPCWSTR functionName)
{
    UNICODE_STRING uFunctionName;
    RtlInitUnicodeString(&uFunctionName, functionName);
    return MmGetSystemRoutineAddress(&uFunctionName);
}

VOID SystemUtils::SpoofThreadStartAddress(PVOID NewStartAddress)
{
    PETHREAD currentThread = PsGetCurrentThread();
    PULONG64 StartAddress = reinterpret_cast<PULONG64>(reinterpret_cast<ULONG_PTR>(currentThread) + 0x6A0);
    *StartAddress = reinterpret_cast<ULONG64>(NewStartAddress);
    
    PULONG64 Win32StartAddress = reinterpret_cast<PULONG64>(reinterpret_cast<ULONG_PTR>(currentThread) + 0x6B0);
    *Win32StartAddress = reinterpret_cast<ULONG64>(NewStartAddress);
}

} // namespace Utils
} // namespace DrvRdma
