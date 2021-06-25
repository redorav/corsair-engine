#pragma once

#include "Core/Containers/CrHashMap.h"
#include "Core/String/CrString.h"

class CrCommandLineParser
{
public:

	CrCommandLineParser() {}

	CrCommandLineParser(int argc, char* argv[])
	{
		for (int i = 0; i < argc; ++i)
		{
			// If it starts with an option character
			if (argv[i][0] == '-')
			{
				// If it has a value and the value is not an option string
				if ((i + 1) < argc && argv[i + 1][0] != '-')
				{
					commandLineArgs.insert({ CrString(argv[i]), CrString(argv[i + 1]) }); // Insert value option
					++i; // Skip next argument
				}
				else
				{
					commandLineArgs.insert({ CrString(argv[i]), CrString() });
				}
			}
		}
	}

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
	const bool operator[](const CrString& key)
	{
		return commandLineArgs.find(key) != commandLineArgs.end();
	}

	// Iterate through the different values of options
	template<typename Fn>
	const void for_each(const CrString& key, const Fn& fn)
	{
		auto range = commandLineArgs.equal_range(key);
		for (auto it = range.first; it != range.second; ++it)
		{
			fn(it->second);
		}
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