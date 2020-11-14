#pragma once

#include "Core/FileSystem/CrFileSystem.h"
#include "Core/String/CrString.h"
#include "Core/String/CrFixedString.h"

#include <cstdint>

struct CrProcessDescriptor
{
	CrProcessDescriptor() {}

	CrProcessDescriptor(const CrPath& executablePath, const CrFixedString512& commandLine, bool waitForCompletion = true, uint32_t waitTimeout = 0xffffffff)
		: executablePath(executablePath), commandLine(commandLine), waitForCompletion(waitForCompletion), waitTimeout(waitTimeout) {}

	CrPath executablePath;
	CrFixedString512 commandLine;
	bool waitForCompletion = true;
	uint32_t waitTimeout = 0xffffffff;
};

enum class CrProcessResult
{
	Success,
	Error
};

class CrProcess
{
public:
	
	static CrProcessResult RunExecutable(const CrProcessDescriptor& processDescriptor);

private:

	CrProcess() {}
};