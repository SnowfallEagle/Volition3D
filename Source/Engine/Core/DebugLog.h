#pragma once

#include <cstdio>
#include <cstdarg>
#include "Common/Types/Common.h"
#include "Common/Platform/Platform.h"

namespace Volition
{

#define VLN_NOTE(CHANNEL, FORMAT, ...) DebugLog.Output(CHANNEL, "Note", FORMAT, __VA_ARGS__)
#define VLN_WARNING(CHANNEL, FORMAT, ...) DebugLog.Output(CHANNEL, "Warning", FORMAT, __VA_ARGS__)
#define VLN_ERROR(CHANNEL, FORMAT, ...) DebugLog.Output(CHANNEL, "Error", FORMAT, __VA_ARGS__)
#define VLN_LOG(FORMAT, ...) DebugLog.Output("", "", FORMAT, __VA_ARGS__)

#if 1
    #define VLN_LOG_VERBOSE(FORMAT, ...) DebugLog.Output("", "", FORMAT, __VA_ARGS__)
#else
    #define VLN_LOG_VERBOSE(...) 
#endif

#define VLN_DEFINE_LOG_CHANNEL(VAR, NAME) static constexpr const char VAR[] = NAME

class VDebugLog
{
    std::FILE* hFile = nullptr;

public:
    void StartUp();
    void ShutDown();

    void Output(const char* Channel, const char* Priority, const char* Format, ...);

private:
    void VarOutput(const char* Channel, const char* Priority, const char* Format, std::va_list VarList);
};

inline VDebugLog DebugLog;

}