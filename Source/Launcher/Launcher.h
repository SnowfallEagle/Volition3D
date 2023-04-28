#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define VLN_ASSERTIONS_LOG_ENGINE 0
#include "Engine/Core/Assert.h"
#include "Engine/Core/Types.h"

namespace Volition
{

namespace EReturnCode
{
    enum Type : i32f
    {
        Success = 0,
        Error   = 1
    };
}

class VLauncher
{
public:
    inline static VLauncher* Instance = nullptr;

    u32 ResizeWidth = 0;
    u32 ResizeHeight = 0;

private:
    struct ID3D11Device* D3DDevice = nullptr;
    struct ID3D11DeviceContext* D3DDeviceContext = nullptr;
    struct IDXGISwapChain* SwapChain = nullptr;
    struct ID3D11RenderTargetView* MainRenderTargetView = nullptr;

    WNDCLASSEXW WindowClass;
    HWND hWnd;

    b32 bRunning;

public:
    VLauncher() // @INCOMPLETE: Set window name here, also we need icon for launcher
    {
        VLN_ASSERT(!Instance);
        Instance = this;
    }

    EReturnCode::Type Run();

private:
    b32 StartUp();
    void ShutDown();

    void ProcessEvents();
    virtual void Update();
    void Render();

    b32 StartUpD3D(HWND hWnd);
    void ShutDownD3D();

    void CreateRenderTarget();
    void DestroyRenderTarget();
};

}
