#pragma once
#include <ntifs.h>
#include <wsk.h>

namespace DrvRdma {
namespace Network {

class SocketManager
{
public:
    static NTSTATUS Send(PWSK_SOCKET socket, PVOID data, SIZE_T length);
    static NTSTATUS Receive(PWSK_SOCKET socket, PVOID buffer, SIZE_T length, SIZE_T* bytesReceived);
    static NTSTATUS NTAPI CompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);
};

} // namespace Network
} // namespace DrvRdma
