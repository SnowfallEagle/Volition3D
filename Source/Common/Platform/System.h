#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class VSystem
{
public:
    void OpenProcess(const char* Prompt)
    {
        STARTUPINFOA StartUpInfo;
        ZeroMemory(&StartUpInfo, sizeof(StartUpInfo));
        StartUpInfo.cb = sizeof(StartUpInfo);

        PROCESS_INFORMATION ProcessInformation;
        ZeroMemory(&ProcessInformation, sizeof(ProcessInformation));

        CreateProcessA(nullptr, (LPSTR)Prompt, nullptr, nullptr, false, 0, nullptr, nullptr, &StartUpInfo, &ProcessInformation);
    }
};

inline VSystem System;