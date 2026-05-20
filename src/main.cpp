#include "main.h"
#include "renderer.h"
#include "menu.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    if (!Renderer::Init(hInstance))
        return 1;

    // ── Main loop ─────────────────────────────────────────────────────────────
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        Renderer::BeginFrame();
        Menu::Draw();
        Renderer::EndFrame();
    }

    Renderer::Shutdown();
    return static_cast<int>(msg.wParam);
}
