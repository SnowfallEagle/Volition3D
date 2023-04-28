#include <cstdlib>
#include "Launcher/Launcher.h"

using namespace Volition;

namespace Game
{

class GLauncher : public VLauncher
{
    virtual void Update()
    {
        ImGui::Begin("Hello", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration
        );

        const ImGuiViewport* Viewport = ImGui::GetMainViewport();
        ImGui::SetWindowPos(Viewport->Pos);
        ImGui::SetWindowSize(Viewport->Size);

        if (ImGui::Button("Play"))
        {
            STARTUPINFOA StartUpInfo;
            ZeroMemory(&StartUpInfo, sizeof(StartUpInfo));
            StartUpInfo.cb = sizeof(StartUpInfo);

            PROCESS_INFORMATION ProcessInformation;
            ZeroMemory(&ProcessInformation, sizeof(ProcessInformation));

            // @TODO: Put this in Volition System class, open launcher after exiting game if was opened through it
            // @TODO: UI
            CreateProcessA(nullptr, (LPSTR)"Game.exe /l /s 720", nullptr, nullptr, false, 0, nullptr, nullptr, &StartUpInfo, &ProcessInformation);

            Stop();
        }

        ImGui::End();
    }
};

}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    Game::GLauncher Launcher;
    return Launcher.Run();
}
