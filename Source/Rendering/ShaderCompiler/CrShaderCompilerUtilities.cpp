#include "Rendering/ShaderCompiler/CrShaderCompiler_pch.h"

#include "CrShaderCompilerUtilities.h"

#include "crstl/string.h"

// TODO Use file
#include <fstream>
#include <sstream>

void CrShaderCompilerUtilities::WriteToFile(const crstl::string& filename, const crstl::string& text)
{
	std::ofstream fileStream;
	fileStream.open(filename.c_str(), std::ios::out);
	fileStream.write(text.c_str(), text.size());
	fileStream.close();
	printf("Wrote contents of file to %s\n", filename.c_str());
}

void CrShaderCompilerUtilities::WriteToFileIfChanged(const crstl::string& filename, const crstl::string& text)
{
	std::ifstream originalFile(filename.c_str());
	std::stringstream originalFileStream;
	originalFileStream << originalFile.rdbuf();
	originalFile.close();

	const crstl::string& originalContents = originalFileStream.str().c_str();

	if (originalContents != text)
	{
		WriteToFile(filename, text);
	}
}

void CrShaderCompilerUtilities::QuitWithMessage(const crstl::string& errorMessage)
{
	// We need a message pump that flushes every message for the shader compiler, otherwise a crash won't allow us to see them
	printf("%s", errorMessage.c_str());
	fflush(stdout);
	exit(-1);
}