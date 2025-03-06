#pragma once

#include "Core/Containers/CrHashMap.h"

#include "crstl/string.h"

class CrCommandLineParser
{
public:

	CrCommandLineParser() {}

	CrCommandLineParser(int argc, char* argv[]);

	// Get first value using key
	const crstl::string& operator()(const crstl::string& key)
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
	bool operator[](const crstl::string& key)
	{
		return commandLineArgs.find(key) != commandLineArgs.end();
	}

	// Iterate through the different values of options
	template<typename FunctionT>
	void for_each(const crstl::string& key, const FunctionT& function)
	{
		commandLineArgs.for_each(key, function);
	}

private:

	crstl::string invalid;

	// It is unordered with respect to keys, but ordered values for each key
	CrHashMultiMap<crstl::string, crstl::string> commandLineArgs;
};

namespace crcore
{
	extern CrCommandLineParser CommandLine;
}