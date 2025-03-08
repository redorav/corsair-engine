#pragma once

#include "Core/CrCoreForwardDeclarations.h"

struct CrStringUtilities
{
	// Split input string into lines
	static void SplitLines(crstl::vector<crstl::string>& lines, const crstl::string& input);
};