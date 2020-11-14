#include "Core/Process/CrProcess.h"

#include <windows.h>

// https://stackoverflow.com/questions/15435994/how-do-i-open-an-exe-from-another-c-exe
CrProcessResult CrProcess::RunExecutable(const CrProcessDescriptor& processDescriptor)
{
	STARTUPINFO startupInfo = {};
	PROCESS_INFORMATION processInfo = {};

	startupInfo.cb = sizeof(startupInfo);

	// Convert to wchar_t
	CrFixedWString512 convertedCommandLine;
	convertedCommandLine.append_convert(processDescriptor.commandLine);

	bool result = CreateProcess
	(
		(LPWSTR)processDescriptor.executablePath.wstring().c_str(),
		&convertedCommandLine[0], // Apparently CreateProcess can modify the incoming string
		NULL,             // Process handle not inheritable
		NULL,             // Thread handle not inheritable
		FALSE,            // Set handle inheritance to FALSE
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
		DWORD error = GetLastError();

		LPTSTR lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

		LocalFree(lpMsgBuf);

		return CrProcessResult::Error;
	}
}