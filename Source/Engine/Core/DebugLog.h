#pragma once

#include <stdio.h>
#include <stdarg.h>
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"

#define VL_NOTE(CHANNEL, FORMAT, ...) DebugLog.Output(CHANNEL, "Note", FORMAT, __VA_ARGS__)
#define VL_WARNING(CHANNEL, FORMAT, ...) DebugLog.Output(CHANNEL, "Warning", FORMAT, __VA_ARGS__)
#define VL_ERROR(CHANNEL, FORMAT, ...) DebugLog.Output(CHANNEL, "Error", FORMAT, __VA_ARGS__)
#define VL_LOG(FORMAT, ...) DebugLog.Output("", "", FORMAT, __VA_ARGS__)

#define VL_DEFINE_LOG_CHANNEL(VAR, NAME) static constexpr const char VAR[] = NAME

class VDebugLog
{
    FILE* hFile = nullptr;

public:
    void StartUp();
    void ShutDown();

    void Output(const char* Channel, const char* Priority, const char* Format, ...);
private:
    void VarOutput(const char* Channel, const char* Priority, const char* Format, va_list VarList);
};

extern VDebugLog DebugLog;
