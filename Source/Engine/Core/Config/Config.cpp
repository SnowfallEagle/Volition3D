#include "Common/Types/Map.h"
#include "Common/Platform/System.h"
#include "Engine/Core/DebugLog.h"
#include "Engine/Core/Config/Arguments.h"
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
    Config.bExecutedWithLauncher = true;
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

static void TargetFPSArg(char** Argv, i32& Cursor)
{
    Config.RenderSpec.TargetFPS = std::atoi(Argv[Cursor]);
    Cursor += 1;
}

static void FixedFPSArg(char** Argv, i32& Cursor)
{
    Config.RenderSpec.TargetFixedFPS = std::atoi(Argv[Cursor]);
    Cursor += 1;
}

static void MaxMipMaps(char** Argv, i32& Cursor)
{
    Config.RenderSpec.MaxMipMaps = std::atoi(Argv[Cursor]);
    Cursor += 1;
}

static void LimitFPSArg(char** Argv, i32& Cursor)
{
    Config.RenderSpec.bLimitFPS = std::atoi(Argv[Cursor]);
    Cursor += 1;
}

static void BackfaceRemovalArg(char** Argv, i32& Cursor)
{
    Config.RenderSpec.bBackfaceRemoval = std::atoi(Argv[Cursor]);
    Cursor += 1;
}

static void SortPolygonsArg(char** Argv, i32& Cursor)
{
    Config.RenderSpec.bSortPolygons = std::atoi(Argv[Cursor]);
    Cursor += 1;
}

static TMap<VString, ArgHandler> ArgHandlers = {
    { LauncherArgShort, { LauncherArg } },
    { LauncherArgLong,  { LauncherArg } },

    { SizeArgShort, { SizeArg, 2  } },
    { SizeArgLong,  { SizeArg, 2  } },

    { WindowTypeFullscreenArgShort, { WindowTypeFullscreenArg }},
    { WindowTypeFullscreenArgLong,  { WindowTypeFullscreenArg }},

    { WindowTypeBorderlessArgShort, { WindowTypeBorderlessArg }},
    { WindowTypeBorderlessArgLong,  { WindowTypeBorderlessArg }},

    { WindowTypeWindowedArgShort, { WindowTypeWindowedArg }},
    { WindowTypeWindowedArgLong,  { WindowTypeWindowedArg }},

    // Works only with /LimitFPS 1
    { TargetFPSArgShort, { TargetFPSArg, 1 }},
    { TargetFPSArgLong,  { TargetFPSArg, 1 }},

    { FixedFPSArgShort, { FixedFPSArg, 1 }},
    { FixedFPSArgLong,  { FixedFPSArg, 1 }},

    { MaxMipMapsArgShort, { MaxMipMaps, 1 }},
    { MaxMipMapsArgLong,  { MaxMipMaps, 1 }},

    { LimitFPSArgShort, { LimitFPSArg, 1 }},
    { LimitFPSArgLong,  { LimitFPSArg, 1 }},

    { BackfaceRemovalArgShort, { BackfaceRemovalArg, 1 }},
    { BackfaceRemovalArgLong,  { BackfaceRemovalArg, 1 }},

    { SortPolygonsArgShort, { SortPolygonsArg, 1 }},
    { SortPolygonsArgLong,  { SortPolygonsArg, 1 }},
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
        System.OpenProcess("Launcher.exe");
    }
}

}
