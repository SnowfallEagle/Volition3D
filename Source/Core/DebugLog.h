#ifndef CORE_DEBUGLOG_H_

/* TODO(sean):
	- VFileSystem?
 */

#include <stdio.h>
#include "Core/Platform.h"

class VDebugLog
{
	static constexpr const char LogPath[] = "Log.txt";

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

		// TODO(sean): Note(...)
	}
	void ShutDown()
	{
		fclose(hFile);
	}
};

extern VDebugLog DebugLog;

#define CORE_DEBUGLOG_H_
#endif