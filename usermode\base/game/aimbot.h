struct vector2 {
    double x, y;
};

void moveto(float x, float y, int smooth)
{
    auto screenCenterX = (float)GetSystemMetrics(SM_CXSCREEN) ;
    auto screenCenterY = (float)GetSystemMetrics(SM_CYSCREEN);


    vector2 ScreenCenter = { (double)screenCenterX / 2.0, (double)screenCenterY / 2.0 };
    vector2 Target = { 0, 0 };

    if (x != 0)
    {
        if (x > ScreenCenter.x)
        {
            Target.x = -(ScreenCenter.x - x);
            Target.x /= smooth;
            if (Target.x + ScreenCenter.x > ScreenCenter.x * 2) Target.x = 0;
        }

        if (x < ScreenCenter.x)
        {
            Target.x = x - ScreenCenter.x;
            Target.x /= smooth;
            if (Target.x + ScreenCenter.x < 0) Target.x = 0;
        }
    }

    if (y != 0)
    {
        if (y > ScreenCenter.y)
        {
            Target.y = -(ScreenCenter.y - y);
            Target.y /= smooth;
            if (Target.y + ScreenCenter.y > ScreenCenter.y * 2) Target.y = 0;
        }

        if (y < ScreenCenter.y)
        {
            Target.y = y - ScreenCenter.y;
            Target.y /= smooth;
            if (Target.y + ScreenCenter.y < 0) Target.y = 0;
        }
    }

    // MOUSE EVENT AQUI - Mover mouse usando input do sistema
    kmNet_mouse_move(Target.x, Target.y);
}

void aimbot_thread()
{
    kmNet_monitor(1); // Habilita monitoramento (pode colocar no init também)

    while (true)
    {
        if (settings::aimbot::enabled)
        {
            // Substitui GetAsyncKeyState(VK_RBUTTON)
            if (kmNet_monitor_mouse_right()) // Verifica se botão direito está pressionado
            {
                CPed* Ped = GetClosestPed();
                if (!Ped) {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                    continue;
                }

                D3DXVECTOR3 HeadPos = Ped->GetBonePosDefault(0 /*Head*/);
                D3DXVECTOR2 ScreenHeadPos = WorldToScreen(HeadPos);

                if (IsOnScreen(ScreenHeadPos))
                {
                    auto screenCenterX = (float)GetSystemMetrics(SM_CXSCREEN) / 2.0f;
                    auto screenCenterY = (float)GetSystemMetrics(SM_CYSCREEN) / 2.0f;

                    int Fov = std::hypot(ScreenHeadPos.x - screenCenterX, ScreenHeadPos.y - screenCenterY);

                    if (Fov < settings::aimbot::fov_size)
                    {
                        moveto(ScreenHeadPos.x, ScreenHeadPos.y, settings::aimbot::smoothness);
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
