#pragma once

#include "Core/CrCoreForwardDeclarations.h"

class CrShaderCompilerUtilities
{
public:

	static void WriteToFile(const CrString& filename, const CrString& text);

	static void WriteToFileIfChanged(const CrString& filename, const CrString& text);

	static void QuitWithMessage(const CrString& errorMessage);
};