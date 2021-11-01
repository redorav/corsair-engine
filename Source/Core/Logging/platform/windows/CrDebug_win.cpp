#include "CrDebug_win.h"

#include <cstdio>
#include <windows.h>
#include <psapi.h>

// For documentation on Visual Studio friendly output see the following
// https://windowscecleaner.blogspot.com/2013/04/debug-output-tricks-for-visual-studio.html

CrDebugWindows::CrDebugWindows()
{

}

void CrDebugWindows::Log(const char* file, unsigned long line, const char* func, const char* format...) const
{
	// Expand variadic arguments
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	char finalString[4096];
	sprintf(finalString, "%s (%ld): %s : %s\n", file, line, func, buffer);

	OutputDebugStringA(finalString);
}

void CrDebugWindows::AssertMsg(bool condition, const char* file, unsigned long line, const char* func, const char* format...) const
{
	if (!condition)
	{
		if (IsDebuggerPresent())
		{
			Log(file, line, func, format);
			__debugbreak();
		}
		else
		{
			MessageBoxA(nullptr, format, "Error", MB_RETRYCANCEL | MB_ICONERROR);
		}
	}
}

void CrDebugWindows::PrintCurrentProcessMemory(const char* file, unsigned long line, const char* func, const char* format...) const
{
	// Expand variadic arguments
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	DWORD processID = GetCurrentProcessId();
	HANDLE hProcess = hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		uint64_t memory = pmc.WorkingSetSize;
		const char* units = "bytes";

		if (pmc.WorkingSetSize > 1024 * 1024)
		{
			memory = ConvertToMegabytes(pmc.WorkingSetSize);
			units = "MB";
		}
		else if (pmc.WorkingSetSize > 1024)
		{
			memory = ConvertToKilobytes(pmc.WorkingSetSize);
			units = "KB";
		}

		char finalString[4096];
		sprintf(finalString, "%s (%ld): %s : [%s] Memory: %lld %s\n", file, line, func, buffer, memory, units);

		OutputDebugStringA(finalString);
	}

	CloseHandle(hProcess);
}

// Create the global object for debug
const ICrDebug* Debug = new CrDebugWindows();