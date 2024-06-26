#pragma once

#include "Core/Containers/CrHashMap.h"
#include "Core/String/CrString.h"

class CrCommandLineParser
{
public:

	CrCommandLineParser() {}

	CrCommandLineParser(int argc, char* argv[]);

	// Get first value using key
	const CrString& operator()(const CrString& key)
	{
		auto it = commandLineArgs.find(key);
		if (it != commandLineArgs.end())
		{
			return it->second;
		}
		else
		{
			return invalid;
		}
	}

	// Get whether key is present
	bool operator[](const CrString& key)
	{
		return commandLineArgs.find(key) != commandLineArgs.end();
	}

	// Iterate through the different values of options
	template<typename FunctionT>
	void for_each(const CrString& key, const FunctionT& function)
	{
		commandLineArgs.for_each(key, function);
	}

private:

	CrString invalid;

	// It is unordered with respect to keys, but ordered values for each key
	CrHashMultiMap<CrString, CrString> commandLineArgs;
};

namespace crcore
{
	extern CrCommandLineParser CommandLine;
}