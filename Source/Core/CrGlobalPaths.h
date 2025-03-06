#pragma once

#include "crstl/string.h"

class CrGlobalPaths
{
public:
	
	CrGlobalPaths();

	static void SetupGlobalPaths
	(
		const char* currentExecutableDirectory,
		const char* dataRootDirectory
	);
	
	static const crstl::string& GetAppDataDirectory();
	
	static const crstl::string& GetCurrentExecutableDirectory();

	static const crstl::string& GetCurrentWorkingDirectory();

	static const crstl::string& GetDataRootDirectory();

	static const crstl::string& GetShaderCompilerPath();

	static const crstl::string& GetShaderSourceDirectory();

	static const crstl::string& GetTempEngineDirectory();

	static const crstl::string& GetTempDirectory();
	
private:
	
	// App Data directory. Use for savegames, preferences, etc
	static crstl::string AppDataDirectory;

	// Directory where executable is running
	static crstl::string CurrentExecutableDirectory;
	
	// Current working directory (not necessarily the same as the executable)
	static crstl::string CurrentWorkingDirectory;

	// Path to root of data
	static crstl::string DataRootDirectory;

	// Path to shader compiler
	static crstl::string ShaderCompilerPath;

	// Directory where shader source lives
	static crstl::string ShaderSourceDirectory;

	// Temporary directory for engine (i.e. subfolder called Corsair Engine)
	// The data here needs to be deletable, i.e. if the OS or someone nukes
	// the folder the engine must be able to ignore or recreate it
	static crstl::string TempEngineDirectory;

	// Generic temporary directory
	static crstl::string TempDirectory;
};