#include "Core/CrCore_pch.h"

#include "CrCommandLine.h"

#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#endif

#include "Core/CrMacros.h"

namespace crcore
{
	CrCommandLineParser CommandLine;
}

CrCommandLineParser::CrCommandLineParser(int argcInput, char* argvInput[])
{
	// Unix systems work in UTF-8 by default and have no problem inputting a properly encoded argv.
	// On Windows, however, directories do not come in as UTF-8 so we need to translate the wchar
	// version of the parameters to a Unicode (multibyte in Windows parlance) version. Because of
	// that, we also need to manage the memory for that fixed command line
#if defined(_WIN32)

	unused_parameter(argvInput);

	int argc = argcInput;
	char** argv = new char* [argcInput + 1];
	argv[argcInput] = nullptr;

	// Get the wide command line provided by Windows
	int wargc;
	wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);

	// Convert to UTF-8 and store locally
	for (int i = 0; i < wargc; ++i)
	{
		// Returns size required for the UTF-8 string
		int size = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);

		// Allocate new string (taking care to add the null terminator)
		argv[i] = new char[size];
		argv[i][size - 1] = 0;

		// Convert wide char to multibyte
		WideCharToMultiByte(CP_UTF8, 0, wargv[i], (int)wcslen(wargv[i]), argv[i], size, nullptr, nullptr);
	}

	LocalFree(wargv);

#else

	argc = argcInput;
	argv = argvInput;

#endif

	for (int i = 0; i < argc; ++i)
	{
		// If it starts with an option character
		if (argv[i][0] == '-')
		{
			// If it has a value and the value is not an option string
			if ((i + 1) < argc && argv[i + 1][0] != '-')
			{
				commandLineArgs.insert(CrString(argv[i]), CrString(argv[i + 1])); // Insert value option
				++i; // Skip next argument
			}
			else
			{
				commandLineArgs.insert(CrString(argv[i]), CrString());
			}
		}
	}

#if defined(_WIN32)

	// Delete each string first
	for (int i = 0; i < argc; ++i)
	{
		delete argv[i];
	}

	// Delete the array
	delete[] argv;

#endif
}