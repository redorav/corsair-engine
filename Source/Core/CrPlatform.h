#pragma once

namespace cr
{
	// Relevant platforms here. The platform and the graphics API are kept separate
	namespace Platform
	{
		enum T : uint8_t
		{
			Windows,
			Count
		};

		constexpr const char* ToString(Platform::T platform)
		{
			switch (platform)
			{
				case Windows: return "windows";
				default: return "invalid";
			}
		}
	};
}