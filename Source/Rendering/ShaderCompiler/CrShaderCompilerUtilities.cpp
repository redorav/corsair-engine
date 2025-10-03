#include "Rendering/ShaderCompiler/CrShaderCompiler_pch.h"

#include "CrShaderCompilerUtilities.h"

#include "crstl/filesystem.h"
#include "crstl/string.h"

#include <stdio.h>
#include <stdlib.h>

// TODO Use file
//#include <fstream>
//#include <sstream>

void CrShaderCompilerUtilities::WriteToFile(const crstl::string& filename, const crstl::string& text)
{
	crstl::file file(filename.c_str(), crstl::file_flags::write | crstl::file_flags::force_create);
	file.write(text.c_str(), text.size());
	file.close();
	printf("Wrote contents of file to %s\n", filename.c_str());
}

void CrShaderCompilerUtilities::WriteToFileIfChanged(const crstl::string& filename, const crstl::string& text)
{
	crstl::file file(filename.c_str(), crstl::file_flags::read);

	crstl::string originalContents;
	originalContents.resize_uninitialized(file.get_size());
	file.read((void*)originalContents.c_str(), file.get_size());
	file.close();

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