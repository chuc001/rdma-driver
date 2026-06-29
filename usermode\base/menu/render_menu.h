#pragma once
#include <thread>

std::thread g_MemoryThread;

void RenderMenu() {
    ImGui::Begin("Configuracoes", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Checkbox("box", &box_esp);
    ImGui::Checkbox("distance", &distance_esp);
    ImGui::End();
}

WPARAM render_loop() {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    // Inicia thread de leitura de memória
    g_ThreadRunning.store(true);
    g_MemoryThread = std::thread(memory_reader_thread);

    MSG msg = {};
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_Init(my_hwnd);
    ImGui_ImplDX11_Init(p_device, p_device_context);
    imgui_color();

    static bool mp[5] = {};
    static const int mb[5] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };

    while (msg.message != WM_QUIT) {
        // Processa mensagens sem bloquear
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) break;
        }

        // Input do mouse
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(my_hwnd, &pt);
        io.MousePos.x = (float)pt.x;
        io.MousePos.y = (float)pt.y;

        for (int i = 0; i < 5; i++) {
            bool p = (GetAsyncKeyState(mb[i]) & 0x8000) != 0;
            io.MouseDown[i] = p;
            io.MouseClicked[i] = p && !mp[i];
            mp[i] = p;
        }

        // Frame de render
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Usa função otimizada
        draw_esp_optimized();
        RenderMenu();

        ImGui::EndFrame();
        ImGui::Render();

        static const float clr[4] = { 0, 0, 0, 0 };
        p_device_context->OMSetRenderTargets(1, &p_main_render_target_view, NULL);
        p_device_context->ClearRenderTargetView(p_main_render_target_view, clr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // VSync off para máximo FPS (0 = sem limite)
        p_swap_chain->Present(0, 0);

        // SEM SLEEP - roda em full speed
    }

    // Cleanup
    g_ThreadRunning.store(false);
    if (g_MemoryThread.joinable()) {
        g_MemoryThread.join();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    cleanup_device_d3d();
    DestroyWindow(my_hwnd);

    return msg.wParam;
}
