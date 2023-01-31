#include "Engine/Core/DebugLog.h"

namespace EAnsiAttr
{
    enum
    {
        None = 0,
        FgGrey = 90,
        BgRed = 41,
        BgBlue = 44,
    };
}

static constexpr const char LogPath[] = "Log.txt";
VL_DEFINE_LOG_CHANNEL(hLogDebugLog, "DebugLog");

VDebugLog DebugLog;

void VDebugLog::StartUp()
{
    hFile = fopen(LogPath, "w");
    if (!hFile)
    {
        // We couldn't use asserts because asserts depend on already working DebugLog
        perror(LogPath);
        VL_DEBUG_BREAK();
    }

    VL_NOTE(hLogDebugLog, "DebugLog started\n");
}

void VDebugLog::ShutDown()
{
    fclose(hFile);
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
        vsnprintf(MessageBuffer, MessageBufferSize, Format, VarList);
    }
    else
    {
        vsnprintf(TempBuffer, TempBufferSize, Format, VarList);
        snprintf(MessageBuffer, MessageBufferSize, "<%s> %s: %s", Channel, Priority, TempBuffer);
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
    printf("\x1b[%dm%s\x1b[m", Color, MessageBuffer);
    fprintf(hFile, "%s", MessageBuffer);
    fflush(hFile);
}
