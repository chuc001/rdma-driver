#pragma once
#include <ntifs.h>
#include "../common/types.h"

namespace DrvRdma {
namespace Process {

class ProcessManager
{
public:
    static PVOID GetBaseAddress(Common::DriverRequest* request);
    static ULONGLONG GetDtb(Common::DriverRequest* request);
    static NTSTATUS ReadMemory(Common::DriverRequest* request, Common::DriverResponse* response, ULONGLONG stored_dtb);
    static NTSTATUS ReadMemoryNoCr3(Common::DriverRequest* request, Common::DriverResponse* response);
    static NTSTATUS BatchReadMemory(Common::DriverRequest* request, Common::DriverResponse* response, ULONGLONG stored_dtb);
    static NTSTATUS BatchReadMemoryNoCr3(Common::DriverRequest* request, Common::DriverResponse* response);
};

} // namespace Process
} // namespace DrvRdma
