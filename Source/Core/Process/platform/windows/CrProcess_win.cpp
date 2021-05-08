#include "Core/Process/CrProcess.h"

#include <windows.h>

#include "Core/Logging/ICrDebug.h"

void PrintLastWindowsError()
{
	DWORD error = GetLastError();

	LPTSTR messageBuffer;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&messageBuffer, 0, NULL);

	OutputDebugString(messageBuffer);

	LocalFree(messageBuffer);
}

// https://stackoverflow.com/questions/15435994/how-do-i-open-an-exe-from-another-c-exe
CrProcessResult CrProcess::RunExecutable(const CrProcessDescriptor& processDescriptor)
{
	STARTUPINFO startupInfo = {};
	PROCESS_INFORMATION processInfo = {};

	startupInfo.cb = sizeof(startupInfo);

	// Convert to wchar_t
	// CreateProcessW can modify the incoming string so we need a temporary buffer
	CrFixedWString512 convertedCommandLine;
	convertedCommandLine.append_convert(processDescriptor.commandLine);

	HANDLE stdOutRead = NULL;
	HANDLE stdOutWrite = NULL;

	SECURITY_ATTRIBUTES securityAttributes = {}; 
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.lpSecurityDescriptor = NULL;

	// Pipe process's std out to a std out we can read later
	if (CreatePipe(&stdOutRead, &stdOutWrite, &securityAttributes, 0))
	{
		if (SetHandleInformation(stdOutRead, HANDLE_FLAG_INHERIT, 0))
		{
			startupInfo.hStdOutput = stdOutWrite;
			startupInfo.dwFlags |= STARTF_USESTDHANDLES;
		}
	}

	bool result = CreateProcess
	(
		nullptr,
		&convertedCommandLine[0],
		NULL,             // Process handle not inheritable
		NULL,             // Thread handle not inheritable
		TRUE,             // Handles are inherited
		CREATE_NO_WINDOW, // Don't create a window
		NULL,             // Use parent's environment block
		NULL,             // Use parent's starting directory 
		&startupInfo,
		&processInfo
	);

	if (result)
	{
		// TODO If not wait for completion we can pass a callback in of some sort such that
		// the code can get called in the background when it's done with it. For now we'll
		// keep it simple
		if (processDescriptor.waitForCompletion)
		{
			// If wait timeout is the maximum, we wait forever
			WaitForSingleObject(processInfo.hProcess, (DWORD)processDescriptor.waitTimeout);

			// If we wait, we can get the exit code immediately after
			DWORD exitCode = 0;
			if (GetExitCodeProcess(processInfo.hProcess, &exitCode) && exitCode != STILL_ACTIVE)
			{
				
			}
		}

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
		CloseHandle(startupInfo.hStdOutput);

		while(true)
		{
			static const int BufferSize = 2048;
			DWORD bytesRead = 0;
			CHAR messageBuffer[BufferSize + 1]; // +1 for null terminator

			bool success = ReadFile(stdOutRead, messageBuffer, BufferSize, &bytesRead, NULL);

			if (success && bytesRead > 0)
			{
				messageBuffer[bytesRead] = 0; // Null terminate the buffers
				CrLog(messageBuffer);
			}
			else
			{
				PrintLastWindowsError();
				break;
			}
		}

		return CrProcessResult::Success;
	}
	else
	{
		PrintLastWindowsError();
		return CrProcessResult::Error;
	}
}