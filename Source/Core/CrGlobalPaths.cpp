#include "Core/CrCore_pch.h"

#include "CrGlobalPaths.h"

#include "GlobalVariables.h" // TODO Rename to GeneratedPaths.h

#include "Core/FileSystem/CrFixedPath.h"

#include "crstl/filesystem.h"

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#endif

// TODO Normalize paths properly

crstl::string CrGlobalPaths::AppDataDirectory;
crstl::string CrGlobalPaths::CurrentExecutableDirectory;
crstl::string CrGlobalPaths::CurrentWorkingDirectory;
crstl::string CrGlobalPaths::DataRootDirectory;
crstl::string CrGlobalPaths::ShaderCompilerPath;
crstl::string CrGlobalPaths::ShaderSourceDirectory;
crstl::string CrGlobalPaths::TempEngineDirectory;
crstl::string CrGlobalPaths::TempDirectory;

// We initialize paths that don't depend on anything in the constructor. The rest of the paths
// we need to initialize
CrGlobalPaths constructor;

CrGlobalPaths::CrGlobalPaths()
{
#if defined(_WIN32)

	crstl::wstring wstringTemp;

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

	CurrentWorkingDirectory = crstl::current_directory_path().c_str();

	TempDirectory = crstl::temp_directory_path().c_str();

	TempEngineDirectory = TempDirectory + "Corsair Engine/";
}

void CrGlobalPaths::SetupGlobalPaths
(
	const char* currentExecutablePath,
	const char* dataRootDirectory
)
{
	CrFixedPath currentExecutableDirectory = CrFixedPath(currentExecutablePath).parent_path();

	CurrentExecutableDirectory = currentExecutableDirectory.c_str();
	CurrentExecutableDirectory += "/";

	DataRootDirectory = dataRootDirectory;
	DataRootDirectory += "/";

	// The shader compiler should always live alongside the main executable
	ShaderCompilerPath = (currentExecutableDirectory / GlobalPaths::ShaderCompilerExecutableName).c_str();

	ShaderSourceDirectory = GlobalPaths::ShaderSourceDirectory;

	CrFixedPath overrideShaderSources = currentExecutableDirectory / "Shader Source";

	// If the directory does not exist, try to find a local one instead
	if (!crstl::exists(GlobalPaths::ShaderSourceDirectory))
	{
		ShaderSourceDirectory = overrideShaderSources.c_str();
	}

	if (crstl::exists(overrideShaderSources.c_str()))
	{
		ShaderSourceDirectory = overrideShaderSources.c_str();
	}
}

const crstl::string& CrGlobalPaths::GetTempDirectory()
{
	return TempDirectory;
}

const crstl::string& CrGlobalPaths::GetCurrentExecutableDirectory()
{
	return CurrentExecutableDirectory;
}

const crstl::string& CrGlobalPaths::GetCurrentWorkingDirectory()
{
	return CurrentWorkingDirectory;
}

const crstl::string& CrGlobalPaths::GetAppDataDirectory()
{
	return AppDataDirectory;
}

const crstl::string& CrGlobalPaths::GetShaderCompilerPath()
{
	return ShaderCompilerPath;
}

const crstl::string& CrGlobalPaths::GetShaderSourceDirectory()
{
	return ShaderSourceDirectory;
}

const crstl::string& CrGlobalPaths::GetTempEngineDirectory()
{
	return TempEngineDirectory;
}

const crstl::string& CrGlobalPaths::GetDataRootDirectory()
{
	return DataRootDirectory;
}
