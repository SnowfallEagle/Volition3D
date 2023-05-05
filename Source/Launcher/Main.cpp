/* @TODO
    - Combo Fullscreen, Borderless, Windowed
    - Int TargetFPS, TargetFixedFPS, MaxMipMaps
    - Bool bLimitFPS, bRenderSolid, bBackfaceRemoval, bSortPolygons
*/

#include <cstdlib>
#include "Common/Platform/Platform.h"
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
        "/wtf",
        "/wtb",
        "/wtw",
    };

private:
    int Width = 800;
    int Height = 600;

    int WindowSizeCurrent = 4;
    int WindowTypeCurrent = 0;

private:
    virtual void Update() override
    {
        const ImGuiStyle& Style = ImGui::GetStyle();
        const ImGuiViewport* Viewport = ImGui::GetMainViewport();

        ImGui::Begin("Main Window", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration
        );

        ImGui::SetWindowPos(Viewport->Pos);
        ImGui::SetWindowSize(Viewport->Size);

        const ImVec2 WorkSize = { Viewport->Size.x / 4, Viewport->Size.x / 4 };
        const ImVec2 WorkPosition = { (Viewport->Size.x - WorkSize.x) / 2, (Viewport->Size.y - WorkSize.y) / 2 };
        ImGui::SetCursorPos(WorkPosition);

        ImGui::BeginChild("Work", WorkSize);
        {
            static constexpr f32 ComboboxOffset = 50.0f;

            ImGui::SetCursorPosX(ComboboxOffset);
            ImGui::Combo("##ComboSize", &WindowSizeCurrent, WindowSizeItems, VLN_ARRAY_SIZE(WindowSizeItems));

            ImGui::SetCursorPosX(ComboboxOffset);
            ImGui::Combo("##ComboType", &WindowTypeCurrent, WindowTypeItems, VLN_ARRAY_SIZE(WindowTypeItems));

            static constexpr const char* PlayButtonText = "Play";

            const f32 Size = ImGui::CalcTextSize(PlayButtonText).x + Style.FramePadding.x * 2.0f;
            const f32 Available = ImGui::GetContentRegionAvail().x;

            ImGui::SetCursorPosX((Available - Size) / 2);
            if (ImGui::Button(PlayButtonText))
            {
                static constexpr i32f BufferSize = 512;
                char Buffer[BufferSize];

                std::snprintf(Buffer, BufferSize, "Game.exe /l /s %s %s", WindowSizeItems[WindowSizeCurrent], WindowTypeItemsToArgs[WindowTypeCurrent]);

                {
                    // @TODO: Put this in Volition System class
                    // @TODO: Open launcher after exiting game if was opened through it

                    STARTUPINFOA StartUpInfo;
                    ZeroMemory(&StartUpInfo, sizeof(StartUpInfo));
                    StartUpInfo.cb = sizeof(StartUpInfo);

                    PROCESS_INFORMATION ProcessInformation;
                    ZeroMemory(&ProcessInformation, sizeof(ProcessInformation));

                    CreateProcessA(nullptr, (LPSTR)Buffer, nullptr, nullptr, false, 0, nullptr, nullptr, &StartUpInfo, &ProcessInformation);
                }

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
