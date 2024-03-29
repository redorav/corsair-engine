#include "Core/CrCore_pch.h"

#include "Core/Process/CrProcess.h"

#include <windows.h>

#include "Core/Logging/ICrDebug.h"

void PrintLastWindowsError()
{
	DWORD error = GetLastError();

	LPSTR messageBuffer;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_ENGLISH_US), (LPSTR)&messageBuffer, 0, NULL);

	OutputDebugStringA(messageBuffer);

	LocalFree(messageBuffer);
}

static void CloseHandleSafe(void*& handle)
{
	if (handle)
	{
		CloseHandle(handle);
		handle = nullptr;
	}
}

// https://stackoverflow.com/questions/15435994/how-do-i-open-an-exe-from-another-c-exe
CrProcess::CrProcess(const CrProcessDescriptor& processDescriptor) : CrProcess()
{
	STARTUPINFO startupInfo = {};
	PROCESS_INFORMATION processInfo = {};

	startupInfo.cb = sizeof(startupInfo);

	// Convert to wchar_t
	// CreateProcessW can modify the incoming string so we need a temporary buffer
	CrFixedWString2048 convertedCommandLine;
	convertedCommandLine.append_convert(processDescriptor.commandLine);

	HANDLE stdOutRead = nullptr;
	HANDLE stdOutWrite = nullptr;

	SECURITY_ATTRIBUTES securityAttributes = {};
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = true;
	securityAttributes.lpSecurityDescriptor = nullptr;

	// Pipe process's std out to a std out we can read later
	if (CreatePipe(&stdOutRead, &stdOutWrite, &securityAttributes, 0))
	{
		if (SetHandleInformation(stdOutRead, HANDLE_FLAG_INHERIT, 0))
		{
			// Redirect both std out and std error to the same location
			startupInfo.hStdError  = stdOutWrite;
			startupInfo.hStdOutput = stdOutWrite;
			startupInfo.dwFlags |= STARTF_USESTDHANDLES;
		}
	}

	bool processResult = CreateProcess
	(
		nullptr,
		&convertedCommandLine[0],
		nullptr,          // Process handle not inheritable
		nullptr,          // Thread handle not inheritable
		TRUE,             // Handles are inherited
		CREATE_NO_WINDOW, // Don't create a window
		nullptr,          // Use parent's environment block
		nullptr,          // Use parent's starting directory 
		&startupInfo,
		&processInfo
	);

	if (processResult)
	{
		CloseHandle(startupInfo.hStdOutput);
		CloseHandle(processInfo.hThread);
		result     = CrProcessResult::Success;
		hProcess   = processInfo.hProcess;
		hStdInput  = stdOutRead;
	}
	else
	{
		result = CrProcessResult::Error;
		PrintLastWindowsError();
	}
}

CrProcess::~CrProcess()
{
	CloseHandleSafe(hProcess);
	CloseHandleSafe(hStdInput);
}

void CrProcess::Wait(uint64_t timeoutMilliseconds)
{
	if (result == CrProcessResult::Success)
	{
		// If wait timeout is the maximum, we wait until it finishes
		WaitForSingleObject(hProcess, (DWORD)timeoutMilliseconds);

		// If we wait, we can get the exit code immediately after
		DWORD processExitCode = 0;
		GetExitCodeProcess(hProcess, &processExitCode);

		returnValue = processExitCode;

		if (processExitCode && processExitCode != STILL_ACTIVE)
		{

		}
	}
}

void CrProcess::ReadStdOut(char* buffer, size_t bufferSize)
{
	if (bufferSize > 0)
	{
		while (true)
		{
			DWORD bytesRead = 0;
			bool success = ReadFile(hStdInput, buffer, (DWORD)(bufferSize - 1), &bytesRead, NULL);

			if (success && bytesRead > 0)
			{
				buffer[bytesRead] = 0; // Null terminate the buffers
			}
			else
			{
				break;
			}
		}
	}
}
