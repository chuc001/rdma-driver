#include "../../include/network/wsk_manager.h"
#include "../../include/network/socket_manager.h"

namespace DrvRdma {
namespace Network {

NTSTATUS WskManager::Initialize(Common::DriverContext* context)
{
    RtlZeroMemory(context, sizeof(Common::DriverContext));

    context->WskClientDispatch.Version = MAKE_WSK_VERSION(1, 0);
    context->WskClientDispatch.Reserved = 0;
    context->WskClientDispatch.WskClientEvent = nullptr;

    WSK_CLIENT_NPI wskClientNpi;
    wskClientNpi.ClientContext = nullptr;
    wskClientNpi.Dispatch = &context->WskClientDispatch;

    NTSTATUS status = WskRegister(&wskClientNpi, &context->WskRegistration);
    if (!NT_SUCCESS(status))
        return status;

    status = WskCaptureProviderNPI(&context->WskRegistration, WSK_INFINITE_WAIT, &context->WskProviderNpi);
    if (!NT_SUCCESS(status))
    {
        WskDeregister(&context->WskRegistration);
        return status;
    }

    KeInitializeEvent(&context->WskEvent, NotificationEvent, FALSE);
    return STATUS_SUCCESS;
}

NTSTATUS WskManager::CreateListenSocket(Common::DriverContext* context)
{
    SOCKADDR_IN localAddress = { 0 };
    localAddress.sin_family = AF_INET;
    localAddress.sin_addr.s_addr = INADDR_ANY;
    localAddress.sin_port = RtlUshortByteSwap(Common::SERVER_PORT);

    PIRP irp = IoAllocateIrp(1, FALSE);
    if (!irp)
        return STATUS_INSUFFICIENT_RESOURCES;

    IoSetCompletionRoutine(irp, SocketManager::CompletionRoutine, &context->WskEvent, TRUE, TRUE, TRUE);

    NTSTATUS status = context->WskProviderNpi.Dispatch->WskSocket(
        context->WskProviderNpi.Client, AF_INET, SOCK_STREAM, IPPROTO_TCP,
        WSK_FLAG_LISTEN_SOCKET, nullptr, nullptr, nullptr, nullptr, nullptr, irp);

    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&context->WskEvent, Executive, KernelMode, FALSE, nullptr);
        status = irp->IoStatus.Status;
    }

    if (!NT_SUCCESS(status))
    {
        IoFreeIrp(irp);
        return status;
    }

    context->ListenSocket = static_cast<PWSK_SOCKET>(irp->IoStatus.Information);

    PWSK_PROVIDER_LISTEN_DISPATCH Dispatch = static_cast<PWSK_PROVIDER_LISTEN_DISPATCH>(context->ListenSocket->Dispatch);

    KeClearEvent(&context->WskEvent);
    IoReuseIrp(irp, STATUS_UNSUCCESSFUL);
    IoSetCompletionRoutine(irp, SocketManager::CompletionRoutine, &context->WskEvent, TRUE, TRUE, TRUE);

    status = Dispatch->WskBind(context->ListenSocket, reinterpret_cast<PSOCKADDR>(&localAddress), 0, irp);
    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&context->WskEvent, Executive, KernelMode, FALSE, nullptr);
        status = irp->IoStatus.Status;
    }

    IoFreeIrp(irp);
    return status;
}

VOID WskManager::Cleanup(Common::DriverContext* context)
{
    if (context->ClientSocket)
    {
        PWSK_PROVIDER_BASIC_DISPATCH Dispatch = static_cast<PWSK_PROVIDER_BASIC_DISPATCH>(context->ClientSocket->Dispatch);
        PIRP irp = IoAllocateIrp(1, FALSE);
        if (irp)
        {
            KEVENT event;
            KeInitializeEvent(&event, NotificationEvent, FALSE);
            IoSetCompletionRoutine(irp, SocketManager::CompletionRoutine, &event, TRUE, TRUE, TRUE);
            Dispatch->WskCloseSocket(context->ClientSocket, irp);
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, nullptr);
            IoFreeIrp(irp);
        }
        context->ClientSocket = nullptr;
    }

    if (context->ListenSocket)
    {
        PWSK_PROVIDER_BASIC_DISPATCH Dispatch = static_cast<PWSK_PROVIDER_BASIC_DISPATCH>(context->ListenSocket->Dispatch);
        PIRP irp = IoAllocateIrp(1, FALSE);
        if (irp)
        {
            KEVENT event;
            KeInitializeEvent(&event, NotificationEvent, FALSE);
            IoSetCompletionRoutine(irp, SocketManager::CompletionRoutine, &event, TRUE, TRUE, TRUE);
            Dispatch->WskCloseSocket(context->ListenSocket, irp);
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, nullptr);
            IoFreeIrp(irp);
        }
        context->ListenSocket = nullptr;
    }

    WskReleaseProviderNPI(&context->WskRegistration);
    WskDeregister(&context->WskRegistration);
}

} // namespace Network
} // namespace DrvRdma
