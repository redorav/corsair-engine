#include "CrGlobalPaths.h"

#include "GlobalVariables.h" // TODO Rename to GeneratedPaths.h

#include "Core/FileSystem/CrPath.h"

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#endif

// TODO Normalize paths properly

CrString CrGlobalPaths::AppDataDirectory;
CrString CrGlobalPaths::CurrentExecutableDirectory;
CrString CrGlobalPaths::CurrentWorkingDirectory;
CrString CrGlobalPaths::DataRootDirectory;
CrString CrGlobalPaths::ShaderCompilerDirectory;
CrString CrGlobalPaths::ShaderCompilerPath;
CrString CrGlobalPaths::ShaderSourceDirectory;
CrString CrGlobalPaths::TempEngineDirectory;
CrString CrGlobalPaths::TempDirectory;

// We initialize paths that don't depend on anything in the constructor. The rest of the paths
// we need to initialize
CrGlobalPaths constructor;

CrGlobalPaths::CrGlobalPaths()
{
	ShaderCompilerDirectory = GlobalPaths::ShaderCompilerDirectory;
	ShaderCompilerPath = GlobalPaths::ShaderCompilerPath;
	ShaderSourceDirectory = GlobalPaths::ShaderSourceDirectory;

#if defined(_WIN32)

	const uint32_t MaxPathSize = MAX_PATH + 1;

	CrWString wstringTemp;

	WCHAR tempPath[MaxPathSize];

	DWORD cwdLength = GetCurrentDirectory(MaxPathSize, tempPath);

	if (cwdLength > 0)
	{
		wstringTemp = tempPath;
		CurrentWorkingDirectory.assign_convert(wstringTemp);
		CurrentWorkingDirectory += "/";
	}

	// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-gettemppatha
	// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getlongpathnamea
	DWORD shortTempPathLength = GetTempPath(MaxPathSize, tempPath);
	if (shortTempPathLength > 0)
	{
		// From the docs: "You can use the same buffer you used for the lpszShortPath parameter"
		DWORD longTempPathLength = GetLongPathName(tempPath, tempPath, MaxPathSize);
		if (longTempPathLength > 0)
		{
			wstringTemp = tempPath;
			TempDirectory.assign_convert(wstringTemp);
		}
	}

	PWSTR appDataPath;

	// https://stackoverflow.com/questions/5920853/how-to-open-a-folder-in-appdata-with-c
	// 
	// https://docs.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
	// https://docs.microsoft.com/en-us/windows/win32/shell/knownfolderid
	HRESULT hResult = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath);

	if (hResult == S_OK)
	{
		wstringTemp = appDataPath;
		AppDataDirectory.assign_convert(wstringTemp);
		AppDataDirectory += "/";
	}

	CoTaskMemFree(appDataPath);

#endif

	TempEngineDirectory = TempDirectory + "Corsair Engine/";
}

void CrGlobalPaths::SetupGlobalPaths
(
	const char* currentExecutablePath,
	const char* dataRootDirectory
)
{
	CurrentExecutableDirectory = CrPath(currentExecutablePath).parent_path().c_str();
	CurrentExecutableDirectory += "/";
	DataRootDirectory = dataRootDirectory;
	DataRootDirectory += "/";
}

const CrString& CrGlobalPaths::GetTempDirectory()
{
	return TempDirectory;
}

const CrString& CrGlobalPaths::GetCurrentExecutableDirectory()
{
	return CurrentExecutableDirectory;
}

const CrString& CrGlobalPaths::GetCurrentWorkingDirectory()
{
	return CurrentWorkingDirectory;
}

const CrString& CrGlobalPaths::GetAppDataDirectory()
{
	return AppDataDirectory;
}

const CrString& CrGlobalPaths::GetShaderCompilerDirectory()
{
	return ShaderCompilerDirectory;
}

const CrString& CrGlobalPaths::GetShaderCompilerPath()
{
	return ShaderCompilerPath;
}

const CrString& CrGlobalPaths::GetShaderSourceDirectory()
{
	return ShaderSourceDirectory;
}

const CrString& CrGlobalPaths::GetTempEngineDirectory()
{
	return TempEngineDirectory;
}

const CrString& CrGlobalPaths::GetDataRootDirectory()
{
	return DataRootDirectory;
}
