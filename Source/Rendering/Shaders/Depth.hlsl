#ifndef DEPTH_DOWNSAMPLE_HLSL
#define DEPTH_DOWNSAMPLE_HLSL

#include "ComputeCommon.hlsl"
#include "Common.hlsl"

Texture2D<float> RawDepthTexture;

RWTexture2D<float2> RWLinearDepthMinMaxMip1;
RWTexture2D<float2> RWLinearDepthMinMaxMip2;
RWTexture2D<float2> RWLinearDepthMinMaxMip3;
RWTexture2D<float2> RWLinearDepthMinMaxMip4;

// Can linearize both perspective and orthographic projections
float LinearizeDepth(float rawDepth, float4 projectionParams)
{
	return rawDepth * projectionParams.z + projectionParams.x / (1.0 - rawDepth * projectionParams.y);
}

float4 LinearizeDepth(float4 rawDepth, float4 projectionParams)
{
	return rawDepth * projectionParams.z + projectionParams.x / (1.0 - rawDepth * projectionParams.y);
}

static const int DEPTH_DOWNSAMPLE_GROUP_SIZE = 8;

groupshared float2 gs_minMaxDepth[DEPTH_DOWNSAMPLE_GROUP_SIZE][DEPTH_DOWNSAMPLE_GROUP_SIZE];

// This function can only produce 4 mipmaps (starting from mip 1) because it's all it has space in LDS for
// It's all good in any case, if we need to process further we can run another really cheap compute pass
// to do the rest
[numthreads(DEPTH_DOWNSAMPLE_GROUP_SIZE, DEPTH_DOWNSAMPLE_GROUP_SIZE, 1)]
void DepthDownsampleLinearizeMinMaxCS(CS_IN In)
{
	uint2 depthTextureResolution;
	RawDepthTexture.GetDimensions(depthTextureResolution.x, depthTextureResolution.y);

	uint2 pixelCoordMip1 = In.dispatchThreadId.xy;
	uint2 groupThreadId = In.groupThreadId.xy;

	// We are outputting to mipmap 1 with respect to the full resolution depth buffer
	float2 centerUV = (2 * pixelCoordMip1.xy + 0.5) / (float2)depthTextureResolution;

	float4 rawDepths = RawDepthTexture.GatherRed(AllPointClampSampler, centerUV);

	float4 linearDepths = LinearizeDepth(rawDepths, cb_Camera.projectionParams);

	float minDepth = min4(linearDepths.x, linearDepths.y, linearDepths.z, linearDepths.w);
	float maxDepth = max4(linearDepths.x, linearDepths.y, linearDepths.z, linearDepths.w);

	RWLinearDepthMinMaxMip1[pixelCoordMip1] = float2(minDepth, maxDepth);
	gs_minMaxDepth[groupThreadId.x][groupThreadId.y] = float2(minDepth, maxDepth);

	GroupMemoryBarrierWithGroupSync();

	// Process Mip 2
	if ((pixelCoordMip1.x % 2) == 0 && (pixelCoordMip1.y % 2) == 0)
	{
		float2 minMaxTL = gs_minMaxDepth[groupThreadId.x + 0][groupThreadId.y + 0];
		float2 minMaxTR = gs_minMaxDepth[groupThreadId.x + 1][groupThreadId.y + 0];
		float2 minMaxBL = gs_minMaxDepth[groupThreadId.x + 0][groupThreadId.y + 1];
		float2 minMaxBR = gs_minMaxDepth[groupThreadId.x + 1][groupThreadId.y + 1];

		float minDepthMip2 = min4(minMaxTL.x, minMaxTR.x, minMaxBL.x, minMaxBR.x);
		float maxDepthMip2 = max4(minMaxTL.y, minMaxTR.y, minMaxBL.y, minMaxBR.y);

		RWLinearDepthMinMaxMip2[pixelCoordMip1 / 2] = float2(minDepthMip2, maxDepthMip2);
		gs_minMaxDepth[groupThreadId.x][groupThreadId.y] = float2(minDepthMip2, maxDepthMip2);
	}

	GroupMemoryBarrierWithGroupSync();

	// Process Mip 3
	if ((pixelCoordMip1.x % 4) == 0 && (pixelCoordMip1.y % 4) == 0)
	{
		float2 minMaxTL = gs_minMaxDepth[groupThreadId.x + 0][groupThreadId.y + 0];
		float2 minMaxTR = gs_minMaxDepth[groupThreadId.x + 3][groupThreadId.y + 0];
		float2 minMaxBL = gs_minMaxDepth[groupThreadId.x + 0][groupThreadId.y + 3];
		float2 minMaxBR = gs_minMaxDepth[groupThreadId.x + 3][groupThreadId.y + 3];

		float minDepthMip3 = min4(minMaxTL.x, minMaxTR.x, minMaxBL.x, minMaxBR.x);
		float maxDepthMip3 = max4(minMaxTL.y, minMaxTR.y, minMaxBL.y, minMaxBR.y);

		RWLinearDepthMinMaxMip3[pixelCoordMip1 / 4] = float2(minDepthMip3, maxDepthMip3);
		gs_minMaxDepth[groupThreadId.x][groupThreadId.y] = float2(minDepthMip3, maxDepthMip3);
	}

	GroupMemoryBarrierWithGroupSync();

	// Process Mip 4
	if ((pixelCoordMip1.x % 8) == 0 && (pixelCoordMip1.y % 8) == 0)
	{
		float2 minMaxTL = gs_minMaxDepth[groupThreadId.x + 0][groupThreadId.y + 0];
		float2 minMaxTR = gs_minMaxDepth[groupThreadId.x + 7][groupThreadId.y + 0];
		float2 minMaxBL = gs_minMaxDepth[groupThreadId.x + 0][groupThreadId.y + 7];
		float2 minMaxBR = gs_minMaxDepth[groupThreadId.x + 7][groupThreadId.y + 7];

		float minDepthMip4 = min4(minMaxTL.x, minMaxTR.x, minMaxBL.x, minMaxBR.x);
		float maxDepthMip4 = max4(minMaxTL.y, minMaxTR.y, minMaxBL.y, minMaxBR.y);

		RWLinearDepthMinMaxMip4[pixelCoordMip1 / 8] = float2(minDepthMip4, maxDepthMip4);
	}
}

#endif