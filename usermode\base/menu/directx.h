#pragma once
static ID3D11Device* p_device = nullptr;
static ID3D11DeviceContext* p_device_context = nullptr;
static IDXGISwapChain* p_swap_chain = nullptr;
static ID3D11RenderTargetView* p_main_render_target_view = nullptr;
void create_render_target()
{
    ID3D11Texture2D* p_back_buffer;
    p_swap_chain->GetBuffer(0, IID_PPV_ARGS(&p_back_buffer));
    p_device->CreateRenderTargetView(p_back_buffer, NULL, &p_main_render_target_view);
    p_back_buffer->Release();
}
void cleanup_render_target()
{
    if (p_main_render_target_view)
    {
        p_main_render_target_view->Release();
        p_main_render_target_view = nullptr;
    }
}
void cleanup_device_d3d()
{
    cleanup_render_target();
    if (p_swap_chain)
    {
        p_swap_chain->Release();
        p_swap_chain = nullptr;
    }
    if (p_device_context)
    {
        p_device_context->Release();
        p_device_context = nullptr;
    }
    if (p_device)
    {
        p_device->Release();
        p_device = nullptr;
    }
}
bool directx()
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = my_hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    UINT create_device_flags = 0;
    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL feature_level_array[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, create_device_flags, feature_level_array, 2, D3D11_SDK_VERSION, &sd, &p_swap_chain, &p_device, &feature_level, &p_device_context) != S_OK)
    {
        return 1;
    }
    create_render_target();
}
void imgui_color()
{
    srand(static_cast<unsigned>(time(0)));

    ImGuiStyle* style = &ImGui::GetStyle();
    auto RandomColor = []() -> ImColor {
        return ImColor(
            rand() % 256, // R (0-255)
            rand() % 256, // G (0-255)
            rand() % 256, // B (0-255)
            255           // A (opacidade total)
        );
        };

    // Atribui cores aleatórias para cada propriedade
    style->Colors[ImGuiCol_TitleBg] = RandomColor();           // Título
    style->Colors[ImGuiCol_TitleBgActive] = RandomColor();     // Título ativo
    style->Colors[ImGuiCol_TitleBgCollapsed] = RandomColor();  // Título colapsado
    style->Colors[ImGuiCol_WindowBg] = RandomColor();          // Fundo da janela
    style->Colors[ImGuiCol_PopupBg] = RandomColor();           // Fundo do popup
    style->Colors[ImGuiCol_Button] = RandomColor();            // Botão
    style->Colors[ImGuiCol_ButtonHovered] = RandomColor();     // Botão ao passar o mouse
    style->Colors[ImGuiCol_ButtonActive] = RandomColor();      // Botão ao clicar
    style->Colors[ImGuiCol_FrameBg] = RandomColor();           // Fundo dos checkboxes
    style->Colors[ImGuiCol_FrameBgHovered] = RandomColor();    // Fundo ao passar o mouse
    style->Colors[ImGuiCol_FrameBgActive] = RandomColor();     // Fundo ao clicar
    style->Colors[ImGuiCol_CheckMark] = RandomColor();         // Marca de seleção
    style->Colors[ImGuiCol_SliderGrab] = RandomColor();        // Pega do slider
    style->Colors[ImGuiCol_SliderGrabActive] = RandomColor();  // Pega do slider ao arrastar
    style->Colors[ImGuiCol_Header] = RandomColor();            // Cabeçalho
    style->Colors[ImGuiCol_HeaderHovered] = RandomColor();     // Cabeçalho ao passar o mouse
    style->Colors[ImGuiCol_HeaderActive] = RandomColor();      // Cabeçalho ativo
    style->Colors[ImGuiCol_Border] = RandomColor();            // Borda
    style->Colors[ImGuiCol_BorderShadow] = RandomColor();      // Sombra da borda
    style->Colors[ImGuiCol_Text] = RandomColor();              // Texto
}
