#include "Core/Logging/windows/CrDebug_win.h"

#include <windows.h>

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
	char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	std::ostringstream os;
	os << file << "(" << line << "): " << func << " : " << buffer << "\n";

	OutputDebugStringA(os.str().c_str());
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

// Create the global object for debug
const ICrDebug& Debug = CrDebugWindows();