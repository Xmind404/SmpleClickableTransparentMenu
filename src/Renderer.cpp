#include "renderer.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace Renderer
{
    // ── D3D / window state ────────────────────────────────────────────────────
    HWND                    Wnd    = nullptr;
    ID3D11Device*           Device = nullptr;
    ID3D11DeviceContext*    Ctx    = nullptr;
    IDXGISwapChain*         Chain  = nullptr;
    ID3D11RenderTargetView* RTV    = nullptr;

    static void CreateRTV()
    {
        ID3D11Texture2D* bb = nullptr;
        Chain->GetBuffer(0, IID_PPV_ARGS(&bb));
        if (bb) { Device->CreateRenderTargetView(bb, nullptr, &RTV); bb->Release(); }
    }

    static void DropRTV()
    {
        if (RTV) { RTV->Release(); RTV = nullptr; }
    }

    // ── Window procedure ──────────────────────────────────────────────────────
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
            return true;

        switch (msg)
        {
        case WM_SIZE:
            if (Device && wp != SIZE_MINIMIZED)
            {
                DropRTV();
                Chain->ResizeBuffers(0, LOWORD(lp), HIWORD(lp), DXGI_FORMAT_UNKNOWN, 0);
                CreateRTV();
            }
            return 0;

        case WM_NCHITTEST:
        {
            LRESULT hit = DefWindowProcW(hWnd, msg, wp, lp);
            return (hit == HTCLIENT) ? HTCAPTION : hit;
        }

        case WM_SYSCOMMAND:
            if ((wp & 0xFFF0) == SC_KEYMENU) return 0;
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(hWnd, msg, wp, lp);
    }

    // ── Init ──────────────────────────────────────────────────────────────────
    bool Init(HINSTANCE hInstance)
    {
        // ── Register and create layered topmost window ────────────────────────
        WNDCLASSEXW wc   = {};
        wc.cbSize        = sizeof(wc);
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = hInstance;
        wc.lpszClassName = L"overlay_wnd";
        RegisterClassExW(&wc);

        Wnd = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
            L"overlay_wnd", L"",
            WS_POPUP,
            0, 0,
            GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN),
            nullptr, nullptr, hInstance, nullptr
        );
        if (!Wnd) return false;

        SetLayeredWindowAttributes(Wnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
        ShowWindow(Wnd, SW_SHOW);
        UpdateWindow(Wnd);

        // ── D3D11 device + swap chain ─────────────────────────────────────────
        DXGI_SWAP_CHAIN_DESC sd         = {};
        sd.BufferCount                  = 2;
        sd.BufferDesc.Format            = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage                  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow                 = Wnd;
        sd.SampleDesc.Count             = 1;
        sd.Windowed                     = TRUE;
        sd.SwapEffect                   = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags                        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        const D3D_FEATURE_LEVEL fl[]    = { D3D_FEATURE_LEVEL_11_0 };
        D3D_FEATURE_LEVEL flOut;

        if (FAILED(D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            fl, 1, D3D11_SDK_VERSION,
            &sd, &Chain, &Device, &flOut, &Ctx)))
            return false;

        CreateRTV();

        // ── ImGui context + backends ──────────────────────────────────────────
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io         = ImGui::GetIO();
        io.ConfigFlags     |= ImGuiConfigFlags_NoMouseCursorChange;
        io.IniFilename      = nullptr;

        ImGui::StyleColorsDark();
        ImGuiStyle& style           = ImGui::GetStyle();
        style.WindowRounding        = 6.f;
        style.FrameRounding         = 4.f;
        style.WindowBorderSize      = 1.f;
        style.Colors[ImGuiCol_WindowBg] = { 0.05f, 0.05f, 0.05f, 0.90f };

        ImGui_ImplWin32_Init(Wnd);
        ImGui_ImplDX11_Init(Device, Ctx);

        return true;
    }

    // ── Shutdown ──────────────────────────────────────────────────────────────
    void Shutdown()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        DropRTV();
        if (Chain)  { Chain->Release();  Chain  = nullptr; }
        if (Ctx)    { Ctx->Release();    Ctx    = nullptr; }
        if (Device) { Device->Release(); Device = nullptr; }
        if (Wnd)    { DestroyWindow(Wnd); Wnd   = nullptr; }
    }

    // ── Per-frame ─────────────────────────────────────────────────────────────
    void BeginFrame()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void EndFrame()
    {
        ImGui::Render();

        // ── Clear to black (color-keyed out = transparent) ────────────────────
        constexpr float clear[4] = { 0.f, 0.f, 0.f, 0.f };
        Ctx->OMSetRenderTargets(1, &RTV, nullptr);
        Ctx->ClearRenderTargetView(RTV, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        Chain->Present(1, 0);

        // ── Click-through: transparent when cursor not over any ImGui window ──
        LONG_PTR ex = GetWindowLongPtrW(Wnd, GWL_EXSTYLE);
        if (ImGui::GetIO().WantCaptureMouse)
            ex &= ~WS_EX_TRANSPARENT;
        else
            ex |=  WS_EX_TRANSPARENT;
        SetWindowLongPtrW(Wnd, GWL_EXSTYLE, ex);
    }
}
