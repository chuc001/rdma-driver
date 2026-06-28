#pragma once
#include <ntifs.h>
#include <wsk.h>

namespace DrvRdma {
namespace Common {

constexpr UCHAR XOR_KEY = 0xCB;
constexpr USHORT SERVER_PORT = 8080;
constexpr ULONG POOL_TAG = 'tsoP';
constexpr ULONG MAX_BATCH_SIZE = 64;

enum class RequestType : ULONG
{
    GetBaseAddress = 1,
    GetDtb = 2,
    ReadMemory = 3,
    ReadMemoryNoCr3 = 4,
    Ping = 5,
    BatchRead = 6,
    BatchReadNoCr3 = 7
};

#pragma pack(push, 1)
struct BatchReadEntry
{
    PVOID address;
    SIZE_T size;
};

struct DriverRequest
{
    RequestType type;
    ULONG pid;
    PVOID address;
    SIZE_T size;
    ULONG batch_count;
    BatchReadEntry batch_entries[MAX_BATCH_SIZE];
    UCHAR data[1024];
};

struct BatchReadResult
{
    NTSTATUS status;
    SIZE_T bytes_read;
    UCHAR data[1024];
};

struct DriverResponse
{
    NTSTATUS status;
    PVOID base_address;
    ULONGLONG dtb;
    SIZE_T bytes_read;
    ULONG batch_count;
    BatchReadResult batch_results[MAX_BATCH_SIZE];
    UCHAR data[1024];
};
#pragma pack(pop)

struct DriverContext
{
    PWSK_SOCKET ListenSocket;
    PWSK_SOCKET ClientSocket;
    WSK_REGISTRATION WskRegistration;
    WSK_PROVIDER_NPI WskProviderNpi;
    WSK_CLIENT_DISPATCH WskClientDispatch;
    BOOLEAN IsConnected;
    KEVENT WskEvent;
    HANDLE AcceptThreadHandle;
    HANDLE ProcessThreadHandle;
    BOOLEAN StopThreads;
    KSPIN_LOCK SocketLock;
    ULONGLONG StoredDtb;
};

} // namespace Common
} // namespace DrvRdma
