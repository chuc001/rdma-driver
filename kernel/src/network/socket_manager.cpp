#include "../../include/network/socket_manager.h"
#include "../../include/utils/crypto.h"
#include "../../include/common/types.h"

namespace DrvRdma {
namespace Network {

NTSTATUS NTAPI SocketManager::CompletionRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);
    
    PKEVENT Event = static_cast<PKEVENT>(Context);
    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS SocketManager::Send(PWSK_SOCKET socket, PVOID data, SIZE_T length)
{
    if (!socket || !data || !length)
        return STATUS_INVALID_PARAMETER;

    Utils::Crypto::XorEncryptDecrypt(data, length, Common::XOR_KEY);

    PWSK_PROVIDER_CONNECTION_DISPATCH Dispatch = static_cast<PWSK_PROVIDER_CONNECTION_DISPATCH>(socket->Dispatch);
    PMDL mdl = IoAllocateMdl(data, static_cast<ULONG>(length), FALSE, FALSE, nullptr);
    if (!mdl)
    {
        Utils::Crypto::XorEncryptDecrypt(data, length, Common::XOR_KEY);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    __try
    {
        MmBuildMdlForNonPagedPool(mdl);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        IoFreeMdl(mdl);
        Utils::Crypto::XorEncryptDecrypt(data, length, Common::XOR_KEY);
        return STATUS_UNSUCCESSFUL;
    }

    WSK_BUF WskBuffer;
    WskBuffer.Mdl = mdl;
    WskBuffer.Offset = 0;
    WskBuffer.Length = length;

    PIRP sendIrp = IoAllocateIrp(1, FALSE);
    if (!sendIrp)
    {
        IoFreeMdl(mdl);
        Utils::Crypto::XorEncryptDecrypt(data, length, Common::XOR_KEY);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KEVENT sendEvent;
    KeInitializeEvent(&sendEvent, SynchronizationEvent, FALSE);
    IoSetCompletionRoutine(sendIrp, CompletionRoutine, &sendEvent, TRUE, TRUE, TRUE);

    NTSTATUS status = Dispatch->WskSend(socket, &WskBuffer, 0, sendIrp);
    if (status == STATUS_PENDING)
    {
        LARGE_INTEGER timeout;
        timeout.QuadPart = -50000000LL;
        status = KeWaitForSingleObject(&sendEvent, Executive, KernelMode, FALSE, &timeout);
        if (status == STATUS_TIMEOUT)
        {
            IoCancelIrp(sendIrp);
            KeWaitForSingleObject(&sendEvent, Executive, KernelMode, FALSE, nullptr);
        }
        status = sendIrp->IoStatus.Status;
    }

    IoFreeIrp(sendIrp);
    IoFreeMdl(mdl);
    Utils::Crypto::XorEncryptDecrypt(data, length, Common::XOR_KEY);

    return status;
}

NTSTATUS SocketManager::Receive(PWSK_SOCKET socket, PVOID buffer, SIZE_T length, SIZE_T* bytesReceived)
{
    if (!socket || !buffer || !length)
        return STATUS_INVALID_PARAMETER;

    PWSK_PROVIDER_CONNECTION_DISPATCH Dispatch = static_cast<PWSK_PROVIDER_CONNECTION_DISPATCH>(socket->Dispatch);
    PMDL mdl = IoAllocateMdl(buffer, static_cast<ULONG>(length), FALSE, FALSE, nullptr);
    if (!mdl)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    __try
    {
        MmBuildMdlForNonPagedPool(mdl);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        IoFreeMdl(mdl);
        return STATUS_UNSUCCESSFUL;
    }

    WSK_BUF WskBuffer;
    WskBuffer.Mdl = mdl;
    WskBuffer.Offset = 0;
    WskBuffer.Length = length;

    PIRP recvIrp = IoAllocateIrp(1, FALSE);
    if (!recvIrp)
    {
        IoFreeMdl(mdl);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KEVENT recvEvent;
    KeInitializeEvent(&recvEvent, SynchronizationEvent, FALSE);
    IoSetCompletionRoutine(recvIrp, CompletionRoutine, &recvEvent, TRUE, TRUE, TRUE);

    NTSTATUS status = Dispatch->WskReceive(socket, &WskBuffer, 0, recvIrp);
    if (status == STATUS_PENDING)
    {
        LARGE_INTEGER timeout;
        timeout.QuadPart = -100000000LL;
        status = KeWaitForSingleObject(&recvEvent, Executive, KernelMode, FALSE, &timeout);
        if (status == STATUS_TIMEOUT)
        {
            IoCancelIrp(recvIrp);
            KeWaitForSingleObject(&recvEvent, Executive, KernelMode, FALSE, nullptr);
        }
        status = recvIrp->IoStatus.Status;
    }

    if (NT_SUCCESS(status))
    {
        *bytesReceived = recvIrp->IoStatus.Information;
        Utils::Crypto::XorEncryptDecrypt(buffer, *bytesReceived, Common::XOR_KEY);
    }
    else
    {
        *bytesReceived = 0;
    }

    IoFreeIrp(recvIrp);
    IoFreeMdl(mdl);

    return status;
}

} // namespace Network
} // namespace DrvRdma
