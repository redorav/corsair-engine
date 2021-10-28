#include "CrRendering_pch.h"

#include "CrVisibility.h"

#include "Core/Containers/CrArray.h"

// Calculates the Obb in projection space and effectively does the same calculations as a vertex shader would do
// to determine whether any part of the bounding box is inside the camera.
// TODO This function can be optimized further
bool CrVisiblity::ObbProjection(const CrBoundingBox& obb, const float4x4& transform, const float4x4& viewProjectionMatrix)
{
	float4x4 worldProjectionMatrix = mul(transform, viewProjectionMatrix);

	float4 cornerMin = float4(obb.center, 1.0f) - float4(obb.extents, 0.0f);
	float4 cornerMax = float4(obb.center, 1.0f) + float4(obb.extents, 0.0f);

	CrArray<float4, 8> boxVertices =
	{
		cornerMax,
		float4(cornerMin.x, cornerMax.yzw),
		float4(cornerMin.x, cornerMax.y, cornerMin.zw),
		float4(cornerMax.xy, cornerMin.zw),

		float4(cornerMax.x, cornerMin.y, cornerMax.zw),
		float4(cornerMin.xy, cornerMax.zw),
		cornerMin,
		float4(cornerMax.x, cornerMin.yzw)
	};

	float4 maxProjectedPosition = float4(-FLT_MAX);
	float4 minProjectedPosition = float4(FLT_MAX);

	for (uint32_t i = 0; i < boxVertices.size(); ++i)
	{
		float4 projectedPosition = mul(boxVertices[i], worldProjectionMatrix);
		projectedPosition /= projectedPosition.wwww;

		maxProjectedPosition = max(maxProjectedPosition, projectedPosition);
		minProjectedPosition = min(minProjectedPosition, projectedPosition);
	}

	if (any(maxProjectedPosition.xy <= -1.0f) || 
		any(minProjectedPosition.xy >= 1.0f) ||
		maxProjectedPosition.z <= 0.0f ||
		minProjectedPosition.z >= 1.0f)
	{
		return false;
	}
	else
	{
		return true;
	}
}