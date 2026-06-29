#pragma once
#include <dwmapi.h>
#include "../librays/Encrypt/safe_call.hpp"
#include "../librays/Encrypt/xorstr.hpp"
HWND my_hwnd = NULL;
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // Deixa transparente - não desenha nada aqui
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool CreateOverlayWindow()
{
    const wchar_t* className = L"OverlayWindowClass";
    const wchar_t* windowName = L"Overlay";

    // Registrar classe da janela
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    wc.lpszClassName = className;

    if (!RegisterClassExW(&wc))
    {
        MessageBoxW(NULL, L"Falha ao registrar classe da janela", L"Erro", MB_OK | MB_ICONERROR);
        return false;
    }

    // Obter dimensões da tela
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Criar janela com estilo para overlay
    my_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,  // Estilos estendidos
        className,
        windowName,
        WS_POPUP,  // Sem bordas
        0, 0,
        screenWidth, screenHeight,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!my_hwnd)
    {
        MessageBoxW(NULL, L"Falha ao criar janela", L"Erro", MB_OK | MB_ICONERROR);
        return false;
    }

    // Tornar janela transparente ao clique (permite interação com janelas abaixo)
    SetLayeredWindowAttributes(my_hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY | LWA_ALPHA);

    // Estender frame para criar efeito de vidro
    MARGINS margins = { -1, -1, -1, -1 };
    if (DwmExtendFrameIntoClientArea(my_hwnd, &margins) != S_OK)
    {
        MessageBoxW(NULL, L"Falha ao estender frame", L"Erro", MB_OK | MB_ICONERROR);
        return false;
    }

    // Garantir que a janela fique sempre no topo
    SetWindowPos(my_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

    // Mostrar janela
    ShowWindow(my_hwnd, SW_SHOW);
    UpdateWindow(my_hwnd);

    return true;
}