#include "../../include/utils/crypto.h"

namespace DrvRdma {
namespace Utils {

VOID Crypto::XorEncryptDecrypt(PVOID data, SIZE_T size, UCHAR key)
{
    PUCHAR bytes = static_cast<PUCHAR>(data);
    for (SIZE_T i = 0; i < size; i++)
    {
        bytes[i] ^= key;
    }
}

} // namespace Utils
} // namespace DrvRdma
