#include "../../include/process/process_manager.h"
#include "../../include/memory/virtual_memory.h"
#include "../../include/memory/physical_memory.h"

extern "C" {
    NTKERNELAPI PVOID NTAPI PsGetProcessSectionBaseAddress(PEPROCESS Process);
}

namespace DrvRdma {
namespace Process {

PVOID ProcessManager::GetBaseAddress(Common::DriverRequest* request)
{
    if (!request->pid)
        return nullptr;

    PEPROCESS process = nullptr;
    NTSTATUS status = PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(request->pid), &process);
    if (!NT_SUCCESS(status) || !process)
        return nullptr;

    PVOID image_base = PsGetProcessSectionBaseAddress(process);
    ObDereferenceObject(process);
    
    return image_base;
}

ULONGLONG ProcessManager::GetDtb(Common::DriverRequest* request)
{
    if (!request->pid)
        return 0;

    PEPROCESS process = nullptr;
    NTSTATUS status = PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(request->pid), &process);
    if (!NT_SUCCESS(status) || !process)
        return 0;

    PVOID base = PsGetProcessSectionBaseAddress(process);
    ULONGLONG dtb = Memory::VirtualMemory::GetDirbaseFromBaseAddress(base);
    ObDereferenceObject(process);
    
    return dtb;
}

NTSTATUS ProcessManager::ReadMemory(Common::DriverRequest* request, Common::DriverResponse* response, ULONGLONG stored_dtb)
{
    if (!request->pid || !request->address || !request->size)
        return STATUS_INVALID_PARAMETER;

    ULONG64 physical_address = Memory::VirtualMemory::TranslateLinear(stored_dtb, reinterpret_cast<ULONG64>(request->address));
    if (!physical_address)
        return STATUS_UNSUCCESSFUL;

    SIZE_T final_size = min(PAGE_SIZE - (physical_address & 0xFFF), request->size);
    if (final_size > sizeof(response->data))
        final_size = sizeof(response->data);

    SIZE_T bytes_read = 0;
    NTSTATUS status = Memory::PhysicalMemory::Read(reinterpret_cast<PVOID>(physical_address), response->data, final_size, &bytes_read);
    
    if (NT_SUCCESS(status))
    {
        response->bytes_read = bytes_read;
    }
    
    return status;
}

NTSTATUS ProcessManager::ReadMemoryNoCr3(Common::DriverRequest* request, Common::DriverResponse* response)
{
    PEPROCESS process = nullptr;
    RtlZeroMemory(response, sizeof(Common::DriverResponse));

    NTSTATUS status = PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(static_cast<ULONG_PTR>(request->pid)), &process);
    if (!NT_SUCCESS(status) || !process)
    {
        DbgPrint("[!] PsLookupProcessByProcessId failed: 0x%X\n", status);
        response->status = STATUS_INVALID_PARAMETER;
        return STATUS_INVALID_PARAMETER;
    }

    uintptr_t process_cr3 = Memory::VirtualMemory::GetProcessCr3(process);
    if (!process_cr3)
    {
        DbgPrint("[!] Failed to get CR3 for PID %d\n", request->pid);
        ObDereferenceObject(process);
        response->status = STATUS_UNSUCCESSFUL;
        return STATUS_UNSUCCESSFUL;
    }

    DbgPrint("[*] Process CR3: 0x%llX, Reading from: 0x%p, Size: %zu\n", process_cr3, request->address, request->size);

    uintptr_t physical_address = Memory::VirtualMemory::TranslateLinear(process_cr3, reinterpret_cast<uintptr_t>(request->address));
    ObDereferenceObject(process);

    if (!physical_address)
    {
        DbgPrint("[!] TranslateLinear failed for address 0x%p\n", request->address);
        response->status = STATUS_UNSUCCESSFUL;
        return STATUS_UNSUCCESSFUL;
    }

    DbgPrint("[*] Physical address: 0x%llX\n", physical_address);

    uintptr_t page_offset = physical_address & 0xFFF;
    SIZE_T max_size = PAGE_SIZE - page_offset;
    SIZE_T read_size = min(max_size, request->size);
    read_size = min(read_size, sizeof(response->data));

    SIZE_T bytes_read = 0;
    status = Memory::PhysicalMemory::Read(reinterpret_cast<PVOID>(physical_address), response->data, read_size, &bytes_read);

    if (!NT_SUCCESS(status))
    {
        DbgPrint("[!] ReadPhysical failed: 0x%X\n", status);
        response->status = status;
        response->bytes_read = 0;
        return status;
    }

    DbgPrint("[+] Successfully read %zu bytes\n", bytes_read);
    response->status = STATUS_SUCCESS;
    response->bytes_read = bytes_read;
    
    return STATUS_SUCCESS;
}

NTSTATUS ProcessManager::BatchReadMemory(Common::DriverRequest* request, Common::DriverResponse* response, ULONGLONG stored_dtb)
{
    if (!request->pid || !request->batch_count || request->batch_count > Common::MAX_BATCH_SIZE)
        return STATUS_INVALID_PARAMETER;

    response->batch_count = request->batch_count;

    for (ULONG i = 0; i < request->batch_count; i++)
    {
        RtlZeroMemory(&response->batch_results[i], sizeof(Common::BatchReadResult));

        if (!request->batch_entries[i].address || !request->batch_entries[i].size)
        {
            response->batch_results[i].status = STATUS_INVALID_PARAMETER;
            continue;
        }

        ULONG64 physical_address = Memory::VirtualMemory::TranslateLinear(stored_dtb, reinterpret_cast<ULONG64>(request->batch_entries[i].address));
        if (!physical_address)
        {
            response->batch_results[i].status = STATUS_UNSUCCESSFUL;
            continue;
        }

        SIZE_T final_size = min(PAGE_SIZE - (physical_address & 0xFFF), request->batch_entries[i].size);
        if (final_size > sizeof(response->batch_results[i].data))
            final_size = sizeof(response->batch_results[i].data);

        SIZE_T bytes_read = 0;
        NTSTATUS status = Memory::PhysicalMemory::Read(reinterpret_cast<PVOID>(physical_address), response->batch_results[i].data, final_size, &bytes_read);

        response->batch_results[i].status = status;
        response->batch_results[i].bytes_read = NT_SUCCESS(status) ? bytes_read : 0;
    }

    return STATUS_SUCCESS;
}

NTSTATUS ProcessManager::BatchReadMemoryNoCr3(Common::DriverRequest* request, Common::DriverResponse* response)
{
    PEPROCESS process = nullptr;

    if (!request->pid || !request->batch_count || request->batch_count > Common::MAX_BATCH_SIZE)
        return STATUS_INVALID_PARAMETER;

    NTSTATUS status = PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(static_cast<ULONG_PTR>(request->pid)), &process);
    if (!NT_SUCCESS(status) || !process)
    {
        response->status = STATUS_INVALID_PARAMETER;
        return STATUS_INVALID_PARAMETER;
    }

    uintptr_t process_cr3 = Memory::VirtualMemory::GetProcessCr3(process);
    ObDereferenceObject(process);

    if (!process_cr3)
    {
        response->status = STATUS_UNSUCCESSFUL;
        return STATUS_UNSUCCESSFUL;
    }

    response->batch_count = request->batch_count;

    for (ULONG i = 0; i < request->batch_count; i++)
    {
        RtlZeroMemory(&response->batch_results[i], sizeof(Common::BatchReadResult));

        if (!request->batch_entries[i].address || !request->batch_entries[i].size)
        {
            response->batch_results[i].status = STATUS_INVALID_PARAMETER;
            continue;
        }

        uintptr_t physical_address = Memory::VirtualMemory::TranslateLinear(process_cr3, reinterpret_cast<uintptr_t>(request->batch_entries[i].address));

        if (!physical_address)
        {
            response->batch_results[i].status = STATUS_UNSUCCESSFUL;
            continue;
        }

        uintptr_t page_offset = physical_address & 0xFFF;
        SIZE_T max_size = PAGE_SIZE - page_offset;
        SIZE_T read_size = min(max_size, request->batch_entries[i].size);
        read_size = min(read_size, sizeof(response->batch_results[i].data));

        SIZE_T bytes_read = 0;
        status = Memory::PhysicalMemory::Read(reinterpret_cast<PVOID>(physical_address), response->batch_results[i].data, read_size, &bytes_read);

        response->batch_results[i].status = status;
        response->batch_results[i].bytes_read = NT_SUCCESS(status) ? bytes_read : 0;
    }

    return STATUS_SUCCESS;
}

} // namespace Process
} // namespace DrvRdma
