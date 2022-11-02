#include "Core/DebugLog.h"

static constexpr const char LogPath[] = "Log.txt";
DEFINE_LOG_CHANNEL(hLogLog, "DebugLog");

VDebugLog DebugLog;

void VDebugLog::StartUp()
{
    hFile = fopen(LogPath, "w");
    if (!hFile)
    {
        // We couldn't use asserts because asserts depend on already working DebugLog
        perror(LogPath);
        DEBUG_BREAK();
    }

    VL_NOTE(hLogLog, "Hello, world!!!!!\n");
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

    printf("%s", MessageBuffer);
    fprintf(hFile, "%s", MessageBuffer);
    fflush(hFile);
}
