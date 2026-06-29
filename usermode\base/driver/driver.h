#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define XOR_KEY 0xCB
#define SERVER_PORT 8080

enum REQUEST_TYPE {
    REQUEST_GET_BASE_ADDRESS = 1,
    REQUEST_GET_DTB = 2,
    REQUEST_READ_MEMORY = 3,
    REQUEST_PING = 4
};

#pragma pack(push, 1)
struct DRIVER_REQUEST {
    REQUEST_TYPE type;
    ULONG pid;
    PVOID address;
    SIZE_T size;
    UCHAR data[1024];
};

struct DRIVER_RESPONSE {
    NTSTATUS status;
    PVOID base_address;
    ULONGLONG dtb;
    SIZE_T bytes_read;
    UCHAR data[1024];
};
#pragma pack(pop)

class KernelClient {
private:
    SOCKET sock;
    bool connected;
    DWORD current_pid;  // Armazena PID atual

    void XorEncryptDecrypt(void* data, size_t size, unsigned char key) {
        unsigned char* bytes = (unsigned char*)data;
        for (size_t i = 0; i < size; i++) {
            bytes[i] ^= key;
        }
    }

    bool SendData(void* data, size_t length) {
        if (!connected) return false;

        std::vector<unsigned char> encrypted_data((unsigned char*)data, (unsigned char*)data + length);
        XorEncryptDecrypt(encrypted_data.data(), length, XOR_KEY);

        int result = send(sock, (char*)encrypted_data.data(), (int)length, 0);

        if (result != length) {
            std::cerr << "[!] Send failed: " << WSAGetLastError() << " (sent " << result << "/" << length << " bytes)" << std::endl;
            return false;
        }

        return true;
    }

    bool ReceiveData(void* buffer, size_t length) {
        if (!connected) return false;

        int total_received = 0;
        char* buf = (char*)buffer;

        while (total_received < length) {
            int result = recv(sock, buf + total_received, (int)(length - total_received), 0);

            if (result <= 0) {
                std::cerr << "[!] Recv failed: " << WSAGetLastError() << " (received " << total_received << "/" << length << " bytes)" << std::endl;
                return false;
            }

            total_received += result;
        }

        XorEncryptDecrypt(buffer, length, XOR_KEY);

        return true;
    }

public:
    KernelClient() : sock(INVALID_SOCKET), connected(false), current_pid(0) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "[!] WSAStartup failed" << std::endl;
        }
    }

    ~KernelClient() {
        Disconnect();
        WSACleanup();
    }

    bool Connect(const char* ip) {
        if (connected) return true;

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "[!] Socket creation failed: " << WSAGetLastError() << std::endl;
            return false;
        }

        int flag = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

        DWORD timeout = 10000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

        sockaddr_in server_addr = {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, ip, &server_addr.sin_addr);

        std::cout << "[*] Connecting to " << ip << ":" << SERVER_PORT << "..." << std::endl;

        if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cerr << "[!] Connection failed: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            sock = INVALID_SOCKET;
            return false;
        }

        connected = true;
        std::cout << "[+] Connected successfully!" << std::endl;
        return true;
    }

    void Disconnect() {
        if (sock != INVALID_SOCKET) {
            shutdown(sock, SD_BOTH);
            closesocket(sock);
            sock = INVALID_SOCKET;
        }
        connected = false;
    }

    bool Ping() {
        if (!connected) return false;

        DRIVER_REQUEST request = {};
        request.type = REQUEST_PING;

        DRIVER_RESPONSE response = {};

        if (!SendData(&request, sizeof(request))) {
            std::cerr << "[!] Failed to send ping" << std::endl;
            return false;
        }

        if (!ReceiveData(&response, sizeof(response))) {
            std::cerr << "[!] Failed to receive ping response" << std::endl;
            return false;
        }

        return response.status == 0;
    }

    PVOID GetBaseAddress(DWORD pid) {
        if (!connected) return nullptr;

        DRIVER_REQUEST request = {};
        request.type = REQUEST_GET_BASE_ADDRESS;
        request.pid = pid;

        DRIVER_RESPONSE response = {};

        if (!SendData(&request, sizeof(request))) {
            std::cerr << "[!] Failed to send GetBaseAddress request" << std::endl;
            return nullptr;
        }

        if (!ReceiveData(&response, sizeof(response))) {
            std::cerr << "[!] Failed to receive GetBaseAddress response" << std::endl;
            return nullptr;
        }

        if (response.status != 0) {
            std::cerr << "[!] GetBaseAddress failed with status: 0x" << std::hex << response.status << std::dec << std::endl;
            return nullptr;
        }

        return response.base_address;
    }

    ULONGLONG GetDTB(DWORD pid) {
        if (!connected) return 0;

        current_pid = pid;  // Armazena PID para usar em ReadMemory

        DRIVER_REQUEST request = {};
        request.type = REQUEST_GET_DTB;
        request.pid = pid;

        DRIVER_RESPONSE response = {};

        if (!SendData(&request, sizeof(request))) {
            std::cerr << "[!] Failed to send GetDTB request" << std::endl;
            return 0;
        }

        if (!ReceiveData(&response, sizeof(response))) {
            std::cerr << "[!] Failed to receive GetDTB response" << std::endl;
            return 0;
        }

        if (response.status != 0) {
            std::cerr << "[!] GetDTB failed with status: 0x" << std::hex << response.status << std::dec << std::endl;
            return 0;
        }

        return response.dtb;
    }

    bool ReadMemory(PVOID address, void* buffer, size_t size) {
        if (!connected) return false;

        if (current_pid == 0) {
            std::cerr << "[!] No target process set. Call GetDTB first." << std::endl;
            return false;
        }

        size_t total_read = 0;
        unsigned char* dest = (unsigned char*)buffer;

        while (total_read < size) {
            DRIVER_REQUEST request = {};
            request.type = REQUEST_READ_MEMORY;
            request.pid = current_pid;  // ✅ AGORA ENVIA O PID
            request.address = (PVOID)((uintptr_t)address + total_read);
            request.size = min(sizeof(request.data), size - total_read);

            DRIVER_RESPONSE response = {};

            if (!SendData(&request, sizeof(request))) {
                std::cerr << "[!] Failed to send ReadMemory request" << std::endl;
                return false;
            }

            if (!ReceiveData(&response, sizeof(response))) {
                std::cerr << "[!] Failed to receive ReadMemory response" << std::endl;
                return false;
            }

            if (response.status != 0) {
                //    std::cerr << "[!] ReadMemory failed with status: 0x" << std::hex << response.status << std::dec << std::endl;
                return false;
            }

            memcpy(dest + total_read, response.data, response.bytes_read);
            total_read += response.bytes_read;

            if (response.bytes_read < request.size) {
                break;
            }
        }

        return total_read > 0;
    }

    template<typename T>
    T read(uintptr_t address) {
        T value = {};
        ReadMemory((PVOID)address, &value, sizeof(T));
        return value;
    }

    bool WriteMemory(PVOID address, void* buffer, size_t size) {
        std::cerr << "[!] WriteMemory not implemented yet" << std::endl;
        return false;
    }

    template<typename T>
    bool Write(PVOID address, T value) {
        return WriteMemory(address, &value, sizeof(T));
    }

    bool IsConnected() const {
        return connected;
    }

    DWORD GetCurrentPID() const {
        return current_pid;
    }
    uintptr_t base_addres_a = 0;
};

void PrintHexDump(const unsigned char* data, size_t size, uintptr_t base_address = 0) {
    std::cout << std::hex << std::setfill('0');

    for (size_t i = 0; i < size; i += 16) {
        std::cout << "    0x" << std::setw(12) << (base_address + i) << "  ";

        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                std::cout << std::setw(2) << (int)data[i + j] << " ";
            }
            else {
                std::cout << "   ";
            }

            if (j == 7) std::cout << " ";
        }

        std::cout << " |";

        for (size_t j = 0; j < 16 && i + j < size; j++) {
            unsigned char c = data[i + j];
            std::cout << (char)(c >= 32 && c <= 126 ? c : '.');
        }

        std::cout << "|" << std::endl;
    }

    std::cout << std::dec;
}



inline KernelClient client;

const uintptr_t MIN_VALID_ADDRESS = 0x1000;
const uintptr_t MAX_VALID_ADDRESS = 0x7FFFFFFFFFFF;

bool memory_init()
{


    std::cout << "[*] Enter driver IP (default: 127.0.0.1): ";
    std::string ip_input;
    std::getline(std::cin, ip_input);

    if (ip_input.empty()) {
        ip_input = "127.0.0.1";
    }

    if (!client.Connect(ip_input.c_str())) {
        std::cerr << "[!] Failed to connect to driver" << std::endl;
        std::cerr << "[!] Make sure driver is loaded: sc start YourDriverName" << std::endl;
        system("pause");
        return 1;
    }

    std::cout << std::endl;
    std::cout << "[*] Testing connection..." << std::endl;
    if (client.Ping()) {
        std::cout << "[+] Ping successful!" << std::endl;
    }
    else {
        std::cerr << "[!] Ping failed - connection may be unstable" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "[*] Enter target process ID: ";
    DWORD target_pid;
    std::cin >> target_pid;

    std::cout << std::endl;
    std::cout << "[*] Getting base address for PID " << target_pid << "..." << std::endl;
    PVOID base_address = client.GetBaseAddress(target_pid);

    if (!base_address) {
        std::cerr << "[!] Failed to get base address" << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "[+] Base Address: 0x" << std::hex << (uintptr_t)base_address << std::dec << std::endl;

    std::cout << "[*] Getting DTB for PID " << target_pid << "..." << std::endl;
    ULONGLONG dtb = client.GetDTB(target_pid);

    if (!dtb) {
        std::cerr << "[!] Failed to get DTB" << std::endl;
        system("pause");
        return 1;
    }

    client.base_addres_a = (uintptr_t)base_address;



    std::cout << "\n=== SUCESSO ===" << std::endl;
    std::cout << "PID: " << target_pid << std::endl;
    std::cout << "Base: 0x" << std::hex << (uintptr_t)base_address << std::dec << std::endl;
    std::cout << "DTB: 0x" << std::hex << dtb << std::dec << std::endl;

    std::cout << "\n[*] Testando leitura de memoria..." << std::endl;
    auto read_test = client.read<uint64_t>(client.base_addres_a);
    std::cout << "[+] Primeiro read: 0x" << std::hex << read_test << std::dec << std::endl;

    return true;
}