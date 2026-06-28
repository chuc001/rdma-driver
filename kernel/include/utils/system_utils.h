#pragma once
#include <ntifs.h>

namespace DrvRdma {
namespace Utils {

class SystemUtils
{
public:
    static PVOID GetSystemRoutineAddress(LPCWSTR functionName);
    static VOID SpoofThreadStartAddress(PVOID NewStartAddress);
};

} // namespace Utils
} // namespace DrvRdma
