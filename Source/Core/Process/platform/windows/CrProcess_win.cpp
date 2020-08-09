#include "Core/Process/CrProcess.h"

#include <windows.h>

// https://stackoverflow.com/questions/15435994/how-do-i-open-an-exe-from-another-c-exe
CrProcessResult CrProcess::RunExecutable(const CrProcessDescriptor& processDescriptor)
{
	STARTUPINFO startupInfo = {};
	PROCESS_INFORMATION processInfo = {};

	startupInfo.cb = sizeof(startupInfo);

	bool result = CreateProcess
	(
		(LPWSTR)processDescriptor.executablePath.wstring().c_str(),
		(LPWSTR)processDescriptor.commandLine.c_str(),
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&startupInfo,
		&processInfo
	);

	if (result)
	{
		if (processDescriptor.waitForCompletion)
		{
			// If wait timeout is the maximum, we wait forever
			WaitForSingleObject(processInfo.hProcess, (DWORD)processDescriptor.waitTimeout);
		}

		//DWORD exitCode = 0;
		//if (GetExitCodeProcess(processInfo.hProcess, &exitCode) && exitCode != STILL_ACTIVE)
		//{
		//
		//}

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);

		return CrProcessResult::Success;
	}
	else
	{
		DWORD error = GetLastError(); error;
		return CrProcessResult::Error;
	}
}