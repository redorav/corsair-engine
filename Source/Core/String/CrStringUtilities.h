#pragma once

#include "Core/CrCoreForwardDeclarations.h"

// TODO
// 1) These utilities could be templated to allow for more reuse
// 2) These could live as static functions inside an extended version 
//    CrString of eastl::string instead of as free functions inside utilities
// 3) It means changing using CrString = eastl::string to CrString : public eastl::string

struct CrStringUtilities
{
	// Erase all instances of char needle, in place
	static void EraseAll(CrString& input, char needle);

	// Erase all instances of string needle, in place
	static void EraseAll(CrString& input, const CrString& needle);

	static void ReplaceAll(CrString& input, char needle, char replace);

	// Split input string into lines
	static void SplitLines(CrVector<CrString>& lines, const CrString& input);
};