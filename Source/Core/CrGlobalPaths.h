#pragma once

#include "Core/String/CrString.h"

class CrGlobalPaths
{
public:
	
	CrGlobalPaths();

	static void SetupGlobalPaths
	(
		const char* currentExecutableDirectory,
		const char* dataRootDirectory
	);
	
	static const CrString& GetAppDataDirectory();
	
	static const CrString& GetCurrentExecutableDirectory();

	static const CrString& GetCurrentWorkingDirectory();

	static const CrString& GetDataRootDirectory();

	static const CrString& GetShaderCompilerPath();

	static const CrString& GetShaderSourceDirectory();

	static const CrString& GetTempEngineDirectory();

	static const CrString& GetTempDirectory();
	
private:
	
	// App Data directory. Use for savegames, preferences, etc
	static CrString AppDataDirectory;

	// Directory where executable is running
	static CrString CurrentExecutableDirectory;
	
	// Current working directory (not necessarily the same as the executable)
	static CrString CurrentWorkingDirectory;

	// Path to root of data
	static CrString DataRootDirectory;

	// Path to shader compiler
	static CrString ShaderCompilerPath;

	// Directory where shader source lives
	static CrString ShaderSourceDirectory;

	// Temporary directory for engine (i.e. subfolder called Corsair Engine)
	// The data here needs to be deletable, i.e. if the OS or someone nukes
	// the folder the engine must be able to ignore or recreate it
	static CrString TempEngineDirectory;

	// Generic temporary directory
	static CrString TempDirectory;
};