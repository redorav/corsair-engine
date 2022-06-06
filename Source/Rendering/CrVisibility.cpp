#include "CrRendering_pch.h"

#include "CrVisibility.h"

#include "Core/Containers/CrArray.h"

#include "Math/CrHlslppMatrixFloat.h"

// These projected corners are before the division by w
void CrVisibility::ComputeObbProjection
(
	const CrBoundingBox& obb,
	const float4x4& worldTransform,
	const float4x4& viewProjectionMatrix,
	CrArray<float4, 8>& projectedCorners)
{
	float4x4 worldViewProjectionMatrix = mul(worldTransform, viewProjectionMatrix);

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

	for (uint32_t i = 0; i < boxVertices.size(); ++i)
	{
		projectedCorners[i] = mul(boxVertices[i], worldViewProjectionMatrix);
	}
}

bool CrVisibility::AreProjectedPointsOnScreen(const CrBoxVertices& projectedCorners)
{
	float3 outsideLeft = float3(1.0f, 1.0f, 1.0f);
	float3 outsideRight = float3(1.0f, 1.0f, 1.0f);

	for (uint32_t i = 0; i < projectedCorners.size(); ++i)
	{
		const float4& projectedPosition = projectedCorners[i];
		outsideLeft = outsideLeft * (projectedPosition.xyz < float3(-projectedPosition.ww, 0.0f));
		outsideRight = outsideRight * (projectedPosition.xyz > float3(projectedPosition.www));
	}

	return !(any(outsideLeft) || any(outsideRight));
}

// Calculates the Obb in projection space and effectively does the same calculations as a vertex shader would do
// to determine whether any part of the bounding box is inside the camera.
// TODO This function can be optimized further
bool CrVisibility::IsObbInFrustum
(
	const CrBoundingBox& obb, 
	const float4x4& worldTransform, 
	const float4x4& viewProjectionMatrix
)
{
	CrArray<float4, 8> projectedCorners;
	ComputeObbProjection(obb, worldTransform, viewProjectionMatrix, projectedCorners);

	return AreProjectedPointsOnScreen(projectedCorners);
}