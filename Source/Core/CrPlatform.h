#pragma once

namespace cr
{
	// Relevant platforms here. The platform and the graphics API are kept separate
	namespace Platform
	{
		enum T : uint32_t
		{
			Windows,
			Count
		};

		constexpr const char* ToString(Platform::T platform, bool lowercase = false)
		{
			switch (platform)
			{
				case Windows: return lowercase ? "windows" : "Windows";
				default: return lowercase ? "invalid" : "Invalid";
			}
		}
	};
}