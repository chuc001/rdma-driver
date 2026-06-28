#pragma once
#include <ntifs.h>
#include "../common/types.h"

namespace DrvRdma {
namespace Network {

class WskManager
{
public:
    static NTSTATUS Initialize(Common::DriverContext* context);
    static NTSTATUS CreateListenSocket(Common::DriverContext* context);
    static VOID Cleanup(Common::DriverContext* context);
};

} // namespace Network
} // namespace DrvRdma
