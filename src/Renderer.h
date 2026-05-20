#pragma once
#include <Windows.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

namespace Renderer
{
    // ── Public D3D objects ────────────────────────────────────────────────────
    extern HWND                     Wnd;
    extern ID3D11Device*            Device;
    extern ID3D11DeviceContext*     Ctx;
    extern IDXGISwapChain*          Chain;
    extern ID3D11RenderTargetView*  RTV;

    bool Init(HINSTANCE hInstance);
    void Shutdown();
    void BeginFrame();
    void EndFrame();
}
