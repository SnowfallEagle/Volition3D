#include <cstdlib>
#include "Common/Platform/Platform.h"
#include "Common/Platform/System.h"
#include "Engine/Core/Config/Arguments.h"
#include "Launcher/Launcher.h"

using namespace Volition;

namespace Game
{

class GLauncher : public VLauncher
{
    static constexpr const char* WindowSizeItems[] = {
        "320 128",
        "480 320",
        "640 480",
        "800 600",
        "1280 720",
        "1280 768",
        "1280 1024",
        "1360 768",
        "1366 768",
        "1440 900",
        "1600 900",
        "1920 1080",
        "2048 1152",
        "2560 1080",
        "3440 1440",
        "3840 2160"
    };

    static constexpr const char* WindowTypeItems[] = {
        "Fullscreen",
        "Borderless",
        "Windowed",
    };

    static constexpr const char* WindowTypeItemsToArgs[] = {
        WindowTypeFullscreenArgShort,
        WindowTypeBorderlessArgShort,
        WindowTypeWindowedArgShort,
    };

private:
    int Width = 800;
    int Height = 600;

    int WindowSizeCurrent = 3;
    int WindowTypeCurrent = 2;

    int FPSLimit = 60;
    int TargetFixedFPS = 60;
    int MaxMipMaps = 8;

    bool bLimitFPS = false;
    bool bBackfaceRemoval = true;

public:
    GLauncher() : VLauncher(L"Launcher", { 800, 600 })
    {}

private:
    virtual void Update() override
    {
        ImGui::Begin("Main Window", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration
        );

        ImGui::SetWindowSize({ (f32)Width, (f32)Height });
        ImGui::SetWindowPos({ 0, 0 });

        const ImVec2 WindowSize = ImGui::GetWindowSize();
        const ImVec2 WorkSize = { WindowSize.x / 2, WindowSize.y / 2 };
        ImGui::SetCursorPos({ (WindowSize.x - WorkSize.x) / 2, (WindowSize.y - WorkSize.y) / 2 });

        ImGui::BeginChild("Work", WorkSize);
        {
            ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x / 2 - 100.0f);
            ImGui::BeginChild("Input Stuff", { 0, 112 });
            {
                ImGui::Combo("Window Size", &WindowSizeCurrent, WindowSizeItems, VLN_ARRAY_SIZE(WindowSizeItems));
                ImGui::Combo("Window Type", &WindowTypeCurrent, WindowTypeItems, VLN_ARRAY_SIZE(WindowTypeItems));

                ImGui::InputInt("FPS Limit", &FPSLimit);
                ImGui::InputInt("Fixed FPS", &TargetFixedFPS);
                ImGui::InputInt("Max Mip Maps", &MaxMipMaps);
            }
            ImGui::EndChild();

            ImGui::SetCursorPos({ ImGui::GetContentRegionAvail().x / 2 - 75.0f, ImGui::GetCursorPosY() + 10.0f });
            ImGui::BeginGroup();
            {
                ImGui::Checkbox("Limit FPS", &bLimitFPS);
                ImGui::Checkbox("Backface Removal", &bBackfaceRemoval);
            }
            ImGui::EndGroup();

            static constexpr const char* RunButtonText = "Run";
            const f32 Size = ImGui::CalcTextSize(RunButtonText).x + ImGui::GetStyle().FramePadding.x * 2.0f;
            const f32 Available = ImGui::GetContentRegionAvail().x;

            ImGui::SetCursorPos({ (Available - Size) / 2, ImGui::GetCursorPosY() + 10.0f });
            if (ImGui::Button(RunButtonText))
            {
                static constexpr i32f BufferSize = 512;
                char Buffer[BufferSize];

                std::snprintf(Buffer, BufferSize,
                    "Game.exe %s "
                    "%s %s %s "
                    "%s %d %s %d %s %d "
                    "%s %d %s %d ",
                    LauncherArgShort,
                    SizeArgShort, WindowSizeItems[WindowSizeCurrent], WindowTypeItemsToArgs[WindowTypeCurrent],

                    TargetFPSArgShort,  FPSLimit,
                    FixedFPSArgShort,   TargetFixedFPS,
                    MaxMipMapsArgShort, MaxMipMaps,

                    LimitFPSArgShort,        bLimitFPS,
                    BackfaceRemovalArgShort, bBackfaceRemoval
                );

                System.OpenProcess(Buffer);
                Stop();
            }
        }
        ImGui::EndChild();

        ImGui::End();
    }
};

}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    Game::GLauncher Launcher;
    return Launcher.Run();
}
