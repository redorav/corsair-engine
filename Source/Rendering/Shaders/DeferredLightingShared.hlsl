#ifndef DEFERRED_LIGHTING_SHARED_HLSL
#define DEFERRED_LIGHTING_SHARED_HLSL

namespace GBufferDebugMode
{
	enum T : uint32_t
	{
		None,
		Albedo,
		WorldNormals,
		Roughness,
		F0,
		Depth,
		DepthLinear,
		Count
	};
};

#endif