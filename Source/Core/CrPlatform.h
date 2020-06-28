#pragma once

namespace cr
{
	struct Platform
	{
		enum T
		{
			VulkanPC,
			DX12_PC,
			VulkanOSX,
			Count
		};
	};
}