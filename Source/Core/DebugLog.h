#ifndef CORE_DEBUGLOG_H_

/* TODO(sean):
    - VFileSystem?
 */

#include <stdio.h>
#include <stdarg.h>
#include "Core/Types.h"
#include "Core/Platform.h"

#define DEFINE_LOG_CHANNEL(VAR, NAME) static constexpr const char VAR[] = NAME

DEFINE_LOG_CHANNEL(hLogLog, "DebugLog");

class VDebugLog
{
private:
    static constexpr const char LogPath[] = "Log.txt";

private:
    FILE* hFile;

public:
    void StartUp()
    {
        hFile = fopen(LogPath, "w");
        if (!hFile)
        {
            // We couldn't use asserts because asserts depend on us
            perror(LogPath);
            DEBUG_BREAK();
        }

        Note(hLogLog, "Hello, world!!!!!\n");
    }
    void ShutDown()
    {
        fclose(hFile);
    }

    void Note(const char* Channel, const char* Format, ...)
    {
        va_list VarList;
        va_start(VarList, Format);

        VarOutput(Channel, "Note", Format, VarList);

        va_end(VarList);
    }
    void Warning(const char* Channel, const char* Format, ...)
    {
        va_list VarList;
        va_start(VarList, Format);

        VarOutput(Channel, "Warning", Format, VarList);

        va_end(VarList);
    }
    void Error(const char* Channel, const char* Format, ...)
    {
        va_list VarList;
        va_start(VarList, Format);

        VarOutput(Channel, "Error", Format, VarList);

        va_end(VarList);
    }
    void Print(const char* Format, ...)
    {
        va_list VarList;
        va_start(VarList, Format);

        VarOutput("", "", Format, VarList);

        va_end(VarList);
    }

private:
    void VarOutput(const char* Channel, const char* Priority, const char* Format, va_list VarList)
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
};

extern VDebugLog DebugLog;

#define CORE_DEBUGLOG_H_
#endif