#include "../../include/driver/driver_core.h"
#include "../../include/network/wsk_manager.h"
#include "../../include/network/socket_manager.h"
#include "../../include/process/process_manager.h"
#include "../../include/utils/system_utils.h"

namespace DrvRdma {
namespace Driver {

Common::DriverContext DriverCore::s_Context = { 0 };
PVOID DriverCore::s_LegitStartAddress = nullptr;

Common::DriverContext* DriverCore::GetContext()
{
    return &s_Context;
}

PVOID DriverCore::GetLegitStartAddress()
{
    return s_LegitStartAddress;
}

VOID DriverCore::ProcessRequestThread(PVOID Context)
{
    UNREFERENCED_PARAMETER(Context);

    Common::DriverRequest* request = static_cast<Common::DriverRequest*>(
        ExAllocatePoolWithTag(NonPagedPool, sizeof(Common::DriverRequest), Common::POOL_TAG)
    );
    
    Common::DriverResponse* response = static_cast<Common::DriverResponse*>(
        ExAllocatePoolWithTag(NonPagedPool, sizeof(Common::DriverResponse), Common::POOL_TAG)
    );

    if (!request || !response)
    {
        if (request) ExFreePoolWithTag(request, Common::POOL_TAG);
        if (response) ExFreePoolWithTag(response, Common::POOL_TAG);
        PsTerminateSystemThread(STATUS_INSUFFICIENT_RESOURCES);
        return;
    }

    while (!s_Context.StopThreads)
    {
        KIRQL oldIrql;
        KeAcquireSpinLock(&s_Context.SocketLock, &oldIrql);
        BOOLEAN isConnected = s_Context.IsConnected;
        PWSK_SOCKET clientSocket = s_Context.ClientSocket;
        KeReleaseSpinLock(&s_Context.SocketLock, oldIrql);

        if (!isConnected || !clientSocket)
        {
            LARGE_INTEGER interval;
            interval.QuadPart = -10000000LL;
            KeDelayExecutionThread(KernelMode, FALSE, &interval);
            continue;
        }

        RtlZeroMemory(request, sizeof(Common::DriverRequest));
        RtlZeroMemory(response, sizeof(Common::DriverResponse));

        SIZE_T bytesReceived = 0;
        NTSTATUS status = Network::SocketManager::Receive(clientSocket, request, sizeof(Common::DriverRequest), &bytesReceived);

        if (!NT_SUCCESS(status) || bytesReceived == 0)
        {
            KeAcquireSpinLock(&s_Context.SocketLock, &oldIrql);
            s_Context.IsConnected = FALSE;
            KeReleaseSpinLock(&s_Context.SocketLock, oldIrql);
            
            LARGE_INTEGER delay;
            delay.QuadPart = -10000000LL;
            KeDelayExecutionThread(KernelMode, FALSE, &delay);
            continue;
        }

        switch (request->type)
        {
        case Common::RequestType::GetBaseAddress:
            response->base_address = Process::ProcessManager::GetBaseAddress(request);
            response->status = response->base_address ? STATUS_SUCCESS : STATUS_NOT_FOUND;
            break;

        case Common::RequestType::GetDtb:
            response->dtb = Process::ProcessManager::GetDtb(request);
            if (response->dtb)
            {
                s_Context.StoredDtb = response->dtb;
                response->status = STATUS_SUCCESS;
            }
            else
            {
                response->status = STATUS_NOT_FOUND;
            }
            break;

        case Common::RequestType::ReadMemory:
            response->status = Process::ProcessManager::ReadMemory(request, response, s_Context.StoredDtb);
            break;

        case Common::RequestType::ReadMemoryNoCr3:
            response->status = Process::ProcessManager::ReadMemoryNoCr3(request, response);
            break;

        case Common::RequestType::BatchRead:
            response->status = Process::ProcessManager::BatchReadMemory(request, response, s_Context.StoredDtb);
            break;

        case Common::RequestType::BatchReadNoCr3:
            response->status = Process::ProcessManager::BatchReadMemoryNoCr3(request, response);
            break;

        case Common::RequestType::Ping:
            response->status = STATUS_SUCCESS;
            break;

        default:
            response->status = STATUS_NOT_SUPPORTED;
            break;
        }

        status = Network::SocketManager::Send(clientSocket, response, sizeof(Common::DriverResponse));

        if (!NT_SUCCESS(status))
        {
            KeAcquireSpinLock(&s_Context.SocketLock, &oldIrql);
            s_Context.IsConnected = FALSE;
            KeReleaseSpinLock(&s_Context.SocketLock, oldIrql);
            
            LARGE_INTEGER delay;
            delay.QuadPart = -10000000LL;
            KeDelayExecutionThread(KernelMode, FALSE, &delay);
        }
    }

    if (request) ExFreePoolWithTag(request, Common::POOL_TAG);
    if (response) ExFreePoolWithTag(response, Common::POOL_TAG);

    PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID DriverCore::AcceptThread(PVOID Context)
{
    UNREFERENCED_PARAMETER(Context);

    PWSK_PROVIDER_LISTEN_DISPATCH Dispatch = static_cast<PWSK_PROVIDER_LISTEN_DISPATCH>(s_Context.ListenSocket->Dispatch);

    while (!s_Context.StopThreads)
    {
        PIRP acceptIrp = IoAllocateIrp(1, FALSE);
        if (!acceptIrp) break;

        KEVENT acceptEvent;
        KeInitializeEvent(&acceptEvent, NotificationEvent, FALSE);
        IoSetCompletionRoutine(acceptIrp, Network::SocketManager::CompletionRoutine, &acceptEvent, TRUE, TRUE, TRUE);

        NTSTATUS status = Dispatch->WskAccept(s_Context.ListenSocket, 0, nullptr, nullptr, nullptr, nullptr, acceptIrp);
        if (status == STATUS_PENDING)
        {
            KeWaitForSingleObject(&acceptEvent, Executive, KernelMode, FALSE, nullptr);
            status = acceptIrp->IoStatus.Status;
        }

        if (NT_SUCCESS(status) && !s_Context.StopThreads)
        {
            KIRQL oldIrql;
            KeAcquireSpinLock(&s_Context.SocketLock, &oldIrql);

            if (s_Context.ClientSocket)
            {
                PWSK_PROVIDER_BASIC_DISPATCH oldDispatch = static_cast<PWSK_PROVIDER_BASIC_DISPATCH>(s_Context.ClientSocket->Dispatch);
                PIRP closeIrp = IoAllocateIrp(1, FALSE);
                if (closeIrp)
                {
                    KEVENT closeEvent;
                    KeInitializeEvent(&closeEvent, NotificationEvent, FALSE);
                    IoSetCompletionRoutine(closeIrp, Network::SocketManager::CompletionRoutine, &closeEvent, TRUE, TRUE, TRUE);
                    oldDispatch->WskCloseSocket(s_Context.ClientSocket, closeIrp);
                    KeReleaseSpinLock(&s_Context.SocketLock, oldIrql);
                    KeWaitForSingleObject(&closeEvent, Executive, KernelMode, FALSE, nullptr);
                    IoFreeIrp(closeIrp);
                    KeAcquireSpinLock(&s_Context.SocketLock, &oldIrql);
                }
            }

            s_Context.ClientSocket = static_cast<PWSK_SOCKET>(acceptIrp->IoStatus.Information);
            s_Context.IsConnected = TRUE;
            KeReleaseSpinLock(&s_Context.SocketLock, oldIrql);
        }

        IoFreeIrp(acceptIrp);
        if (s_Context.StopThreads) break;
    }

    PsTerminateSystemThread(STATUS_SUCCESS);
}

VOID DriverCore::ProcessRequestThreadWrapper(PVOID Context)
{
    if (s_LegitStartAddress)
    {
        Utils::SystemUtils::SpoofThreadStartAddress(s_LegitStartAddress);
    }
    ProcessRequestThread(Context);
}

VOID DriverCore::AcceptThreadWrapper(PVOID Context)
{
    if (s_LegitStartAddress)
    {
        Utils::SystemUtils::SpoofThreadStartAddress(s_LegitStartAddress);
    }
    AcceptThread(Context);
}

NTSTATUS DriverCore::Initialize(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    s_LegitStartAddress = Utils::SystemUtils::GetSystemRoutineAddress(L"ExpWorkerThread");
    if (!s_LegitStartAddress)
    {
        s_LegitStartAddress = Utils::SystemUtils::GetSystemRoutineAddress(L"PspSystemThreadStartup");
    }

    NTSTATUS status = Network::WskManager::Initialize(&s_Context);
    if (!NT_SUCCESS(status))
        return status;

    status = Network::WskManager::CreateListenSocket(&s_Context);
    if (!NT_SUCCESS(status))
    {
        Network::WskManager::Cleanup(&s_Context);
        return status;
    }

    KeInitializeSpinLock(&s_Context.SocketLock);
    s_Context.StopThreads = FALSE;
    s_Context.StoredDtb = 0;

    status = PsCreateSystemThread(&s_Context.AcceptThreadHandle, THREAD_ALL_ACCESS, nullptr, nullptr, nullptr, AcceptThreadWrapper, nullptr);
    if (!NT_SUCCESS(status))
    {
        Network::WskManager::Cleanup(&s_Context);
        return status;
    }

    status = PsCreateSystemThread(&s_Context.ProcessThreadHandle, THREAD_ALL_ACCESS, nullptr, nullptr, nullptr, ProcessRequestThreadWrapper, nullptr);
    if (!NT_SUCCESS(status))
    {
        s_Context.StopThreads = TRUE;
        Network::WskManager::Cleanup(&s_Context);
        return status;
    }

    DriverObject->DriverUnload = Unload;
    return STATUS_SUCCESS;
}

VOID DriverCore::Unload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    s_Context.StopThreads = TRUE;

    PVOID threadObject;
    if (s_Context.ProcessThreadHandle)
    {
        if (NT_SUCCESS(ObReferenceObjectByHandle(s_Context.ProcessThreadHandle, THREAD_ALL_ACCESS, nullptr, KernelMode, &threadObject, nullptr)))
        {
            KeWaitForSingleObject(threadObject, Executive, KernelMode, FALSE, nullptr);
            ObDereferenceObject(threadObject);
            ZwClose(s_Context.ProcessThreadHandle);
        }
    }

    if (s_Context.AcceptThreadHandle)
    {
        if (NT_SUCCESS(ObReferenceObjectByHandle(s_Context.AcceptThreadHandle, THREAD_ALL_ACCESS, nullptr, KernelMode, &threadObject, nullptr)))
        {
            KeWaitForSingleObject(threadObject, Executive, KernelMode, FALSE, nullptr);
            ObDereferenceObject(threadObject);
            ZwClose(s_Context.AcceptThreadHandle);
        }
    }

    Network::WskManager::Cleanup(&s_Context);
}

} // namespace Driver
} // namespace DrvRdma
