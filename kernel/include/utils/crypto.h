#pragma once
#include <ntifs.h>

namespace DrvRdma {
namespace Utils {

class Crypto
{
public:
    static VOID XorEncryptDecrypt(PVOID data, SIZE_T size, UCHAR key);
};

} // namespace Utils
} // namespace DrvRdma
