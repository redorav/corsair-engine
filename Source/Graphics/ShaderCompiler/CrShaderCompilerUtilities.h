#pragma once

#include "Core/CrCoreForwardDeclarations.h"

class CrShaderCompilerUtilities
{
public:

	static void WriteToFile(const crstl::string& filename, const crstl::string& text);

	static void WriteToFileIfChanged(const crstl::string& filename, const crstl::string& text);

	static void QuitWithMessage(const crstl::string& errorMessage);
};