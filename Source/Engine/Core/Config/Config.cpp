#include "Common/Types/Map.h"
#include "Engine/Core/DebugLog.h"
#include "Engine/Core/Config/Config.h"

namespace Volition
{

VLN_DEFINE_LOG_CHANNEL(hLogConfig, "Config");

struct ArgHandler
{
    using Fun = void (*)(char** Argv, i32& Cursor);

    Fun Function = nullptr;
    i32 MinArgs = 0;
};

static void LauncherArg(char** Argv, i32& Cursor)
{
    // @INCOMPLETE: Mark that we have to execute launcher on ShutDown
}

static void SizeArg(char** Argv, i32& Cursor)
{
    Config.WindowSpec.Size = { std::atoi(Argv[Cursor]), std::atoi(Argv[Cursor + 1]) };
    Cursor += 2;
}

static void WindowTypeFullscreenArg(char** Argv, i32& Cursor)
{
    Config.WindowSpec.Flags &= ~(EWindowSpecificationFlags::Fullscreen | EWindowSpecificationFlags::Borderless | EWindowSpecificationFlags::Windowed);
    Config.WindowSpec.Flags |= EWindowSpecificationFlags::Fullscreen;
}

static void WindowTypeBorderlessArg(char** Argv, i32& Cursor)
{
    Config.WindowSpec.Flags &= ~(EWindowSpecificationFlags::Fullscreen | EWindowSpecificationFlags::Borderless | EWindowSpecificationFlags::Windowed);
    Config.WindowSpec.Flags |= EWindowSpecificationFlags::Borderless;
}

static void WindowTypeWindowedArg(char** Argv, i32& Cursor)
{
    Config.WindowSpec.Flags &= ~(EWindowSpecificationFlags::Fullscreen | EWindowSpecificationFlags::Borderless | EWindowSpecificationFlags::Windowed);
    Config.WindowSpec.Flags |= EWindowSpecificationFlags::Windowed;
}

static TMap<VString, ArgHandler> ArgHandlers = {
    { "/l", { LauncherArg } },
    { "/s", { SizeArg, 2  } },
    { "/wtf", { WindowTypeFullscreenArg }},
    { "/wtb", { WindowTypeBorderlessArg }},
    { "/wtw", { WindowTypeWindowedArg }}
};

void VConfig::StartUp(i32 Argc, char** Argv)
{
    for (i32f Cursor = 1; Cursor < Argc; )
    {
        VLN_NOTE(hLogConfig, "Arg %d: %s\n", Cursor, Argv[Cursor]);

        if (Argv[Cursor][0] != '/')
        {
            ++Cursor;
            continue;
        }

        const auto It = ArgHandlers.find(Argv[Cursor]);
        if (It == ArgHandlers.end())
        {
            VLN_WARNING(hLogConfig, "Unknown argument <%s>!\n", Argv[Cursor]);
            ++Cursor;
        }
        else
        {
            const ArgHandler& Handler = It->second;
            if (Argc - Cursor <= Handler.MinArgs)
            {
                VLN_WARNING(hLogConfig, "Too few params for arg <%s>: %d expected!\n", Argv[Cursor], Handler.MinArgs);
                ++Cursor;
                continue;
            }

            ++Cursor;

            if (!Handler.Function)
            {
                VLN_WARNING(hLogConfig, "<%s> handler function is null!\n", Argv[Cursor]);
                continue;
            }
            Handler.Function(Argv, Cursor);
        }
    }
}

void VConfig::ShutDown()
{
    if (bExecutedWithLauncher)
    {
        // @INCOMPLETE
        // System.CreateProcess("Launcher.exe")
    }
}

}
