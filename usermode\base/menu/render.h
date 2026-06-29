#pragma once
#include <thread>

std::thread g_MemoryThread;
bool show_menu = true;
int tab = 0;
void RenderMenu() {
    if (GetAsyncKeyState(VK_INSERT) & 1) show_menu = !show_menu;
    if (show_menu)
    {
        ImGui::SetNextWindowSize({ 320, 250 });
        ImGui::Begin("rdma fivem", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
        if (ImGui::Button("Aimbot", { 100, 20 })) tab = 0;
        ImGui::SameLine();
        if (ImGui::Button("Visuals", { 100, 20 })) tab = 1;
        switch (tab)
        {
        case 0:
        {
           ImGui::Checkbox("Enable", &settings::aimbot::enabled);
            ImGui::Checkbox("Show Fov", &settings::aimbot::fov_circle);
            ImGui::SliderFloat("##Fov", &settings::aimbot::fov_size, 50, 300, "Fov: %.2f");
            ImGui::SliderFloat("##Smoothness", &settings::aimbot::smoothness, 1, 10, "Smoothness: %.2f");
            break;
        }
        case 1:
        {
            ImGui::Checkbox("Box", &settings::visual::box);
            ImGui::Checkbox("head", &settings::visual::headdd);
            ImGui::Checkbox("lines", &settings::visual::lines);
            break;
        }
        case 2:
        {
            if (ImGui::Button("Unload", { 120, 20 })) exit(0);
            break;
        }
        }
        ImGui::End();
    }

   

}

WPARAM render_loop() {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    g_Running.store(true);
    g_MemoryThread = std::thread(memory_reader_thread);

    MSG msg = {};
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplWin32_Init(my_hwnd);
    ImGui_ImplDX11_Init(p_device, p_device_context);
 //   imgui_color();

    static bool mp[5] = {};
    static const int mb[5] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };

    while (msg.message != WM_QUIT) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) break;
        }

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

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        draw_esp_optimized();
        RenderMenu();

        ImGui::EndFrame();
        ImGui::Render();

        static const float clr[4] = { 0, 0, 0, 0 };
        p_device_context->OMSetRenderTargets(1, &p_main_render_target_view, NULL);
        p_device_context->ClearRenderTargetView(p_main_render_target_view, clr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        p_swap_chain->Present(0, 0);
    }

    g_Running.store(false);
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
