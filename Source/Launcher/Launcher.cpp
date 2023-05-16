#include <tchar.h>
#include <d3d11.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "Launcher/Launcher.h"
#include "resource.h"

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace Volition;

namespace Volition
{

EReturnCode::Type VLauncher::Run()
{
    if (!StartUp())
    {
        return EReturnCode::Error;
    }

    bRunning = true;
    while (bRunning)
    {
        ProcessEvents();
        Update();
        Render();
    }

    return EReturnCode::Success;
}

b32 VLauncher::StartUp()
{
    WindowClass = { sizeof(WindowClass), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Launcher Window", nullptr};
    RegisterClassExW(&WindowClass);
    hWnd = CreateWindowW(WindowClass.lpszClassName, WindowTitle, WS_OVERLAPPEDWINDOW, 100, 100, WindowSize.X, WindowSize.Y, nullptr, nullptr, WindowClass.hInstance, nullptr);

    HICON Icon = (HICON)LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 192, 192, 0);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)Icon);
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)Icon);

    if (!StartUpD3D(hWnd))
    {
        ShutDownD3D();
        UnregisterClassW(WindowClass.lpszClassName, WindowClass.hInstance);
        return false;
    }

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO IO = ImGui::GetIO();
    IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    IO.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(D3DDevice, D3DDeviceContext);

    return true;
}

void VLauncher::ShutDown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    ShutDownD3D();
    DestroyWindow(hWnd);
    UnregisterClassW(WindowClass.lpszClassName, WindowClass.hInstance);
}

void VLauncher::ProcessEvents()
{
    MSG Msg;
    while (PeekMessage(&Msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
        if (Msg.message == WM_QUIT)
        {
            bRunning = false;
        }
    }

    if (ResizeWidth != 0 && ResizeHeight != 0)
    {
        DestroyRenderTarget();
        SwapChain->ResizeBuffers(0, ResizeWidth, ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        ResizeWidth = ResizeHeight = 0;
        CreateRenderTarget();
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void VLauncher::Render()
{
    ImGui::Render();
    D3DDeviceContext->OMSetRenderTargets(1, &MainRenderTargetView, nullptr);
    static constexpr f32 ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    D3DDeviceContext->ClearRenderTargetView(MainRenderTargetView, ClearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    SwapChain->Present(1, 0); // Present with vsync
}

b32 VLauncher::StartUpD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    ZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.BufferDesc.Width = 0;
    SwapChainDesc.BufferDesc.Height = 0;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.OutputWindow = hWnd;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.SampleDesc.Quality = 0;
    SwapChainDesc.Windowed = TRUE;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT CreateDeviceFlags = 0;
    D3D_FEATURE_LEVEL FeatureLevel;
    const D3D_FEATURE_LEVEL FeatureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

    HRESULT Res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, CreateDeviceFlags, FeatureLevelArray, 2, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &D3DDevice, &FeatureLevel, &D3DDeviceContext);
    if (Res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
    {
        Res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, CreateDeviceFlags, FeatureLevelArray, 2, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &D3DDevice, &FeatureLevel, &D3DDeviceContext);
    }
    if (Res != S_OK)
    {
        return false;
    }

    CreateRenderTarget();
    return true;
}

void VLauncher::ShutDownD3D()
{
    DestroyRenderTarget();

    if (SwapChain)
    {
        SwapChain->Release();
        SwapChain = nullptr;
    }

    if (D3DDeviceContext)
    {
        D3DDeviceContext->Release();
        D3DDeviceContext = nullptr;
    }

    if (D3DDevice)
    {
        D3DDevice->Release();
        D3DDevice = nullptr;
    }
}

void VLauncher::CreateRenderTarget()
{
    ID3D11Texture2D* BackBuffer;
    SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
    D3DDevice->CreateRenderTargetView(BackBuffer, nullptr, &MainRenderTargetView);
    BackBuffer->Release();
}

void VLauncher::DestroyRenderTarget()
{
    if (MainRenderTargetView)
    {
        MainRenderTargetView->Release();
        MainRenderTargetView = nullptr;
    }
}

}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
    {
        return true;
    }

    switch (Msg)
    {
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
        {
            return 0;
        }

        VLauncher::Instance->ResizeWidth = (UINT)LOWORD(lParam);
        VLauncher::Instance->ResizeHeight = (UINT)HIWORD(lParam);
    } return 0;

    case WM_SYSCOMMAND:
    {
        if ((wParam & 0xfff0) == SC_KEYMENU)
        {
            return 0;
        }
    } break;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
    } return 0;
    }

    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

