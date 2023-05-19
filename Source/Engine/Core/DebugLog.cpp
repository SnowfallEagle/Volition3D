#include "Engine/Core/DebugLog.h"

namespace Volition
{

namespace EAnsiAttr
{
    enum Type
    {
        None   = 0,
        FgGrey = 90,
        BgRed  = 41,
        BgBlue = 44,
    };
}

static constexpr const char LogPath[] = "Log.txt";
VLN_DEFINE_LOG_CHANNEL(hLogDebugLog, "DebugLog");

void VDebugLog::StartUp()
{
    hFile = std::fopen(LogPath, "w");
    if (!hFile)
    {
        // We couldn't use asserts because asserts depend on already working DebugLog
        std::perror(LogPath);
        VLN_DEBUG_BREAK();
    }

    VLN_NOTE(hLogDebugLog, "DebugLog started\n");
}

void VDebugLog::ShutDown()
{
    std::fclose(hFile);
}

void VDebugLog::Output(const char* Channel, const char* Priority, const char* Format, ...)
{
    va_list VarList;
    va_start(VarList, Format);

    VarOutput(Channel, Priority, Format, VarList);

    va_end(VarList);
}

void VDebugLog::VarOutput(const char* Channel, const char* Priority, const char* Format, va_list VarList)
{
    static constexpr i32f MessageBufferSize = 512;
    static constexpr i32f TempBufferSize = 420;

    // Fill message buffer
    char MessageBuffer[MessageBufferSize];
    char TempBuffer[TempBufferSize];

    if (Channel[0] == 0)
    {
        std::vsnprintf(MessageBuffer, MessageBufferSize, Format, VarList);
    }
    else
    {
        std::vsnprintf(TempBuffer, TempBufferSize, Format, VarList);
        std::snprintf(MessageBuffer, MessageBufferSize, "<%s> %s: %s", Channel, Priority, TempBuffer);
    }

    // Set color
    i32f Color;
    switch (Priority[0])
    {
    case 'W': { Color = EAnsiAttr::BgBlue; } break;
    case 'E': { Color = EAnsiAttr::BgRed; } break;
    default: { Color = EAnsiAttr::None; } break;
    }

    // Output
    std::printf("\x1b[%dm%s\x1b[m", Color, MessageBuffer);
    std::fprintf(hFile, "%s", MessageBuffer);
    std::fflush(hFile);
}

}
