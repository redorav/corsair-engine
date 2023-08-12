#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrCamera.h"

#include "Rendering/CrRendering.h"

#include "Math/CrMath.h"
#include "Math/CrHlslppVectorFloat.h"
#include "Math/CrHlslppQuaternion.h"
#include "Math/CrHlslppMatrixFloat.h"

CrCamera::CrCamera() : CrEntity()
	, m_projection(cr3d::CameraProjection::Perspective)
	, m_nearPlane(0.1f)
	, m_farPlane(100.0f)
	, m_aspectRatio(1.0f)
	, m_reverseDepth(true)
	, m_resolutionWidth(1)
	, m_resolutionHeight(1)
	, m_view2WorldMatrix(float4x4::identity())
	, m_world2ViewMatrix(float4x4::identity())
	, m_view2ProjectionMatrix(float4x4::identity())
{
	m_forwardWorldSpace = float3(0, 0, 1);
	m_upWorldSpace     = float3(0, 1, 0);
	m_rightWorldSpace  = float3(1, 0, 0);
}

CrCamera::CrCamera(uint32_t resolutionWidth, uint32_t resolutionHeight, float nearPlane, float farPlane) : CrEntity()
{
	SetupPerspective(resolutionWidth, resolutionHeight, nearPlane, farPlane);
}

void CrCamera::SetupPerspective(uint32_t resolutionWidth, uint32_t resolutionHeight, float nearPlane, float /*farPlane*/)
{
	// Create a new perspective projection matrix. The height will stay the same while the width will vary as per aspect ratio.
	m_aspectRatio = (float)resolutionWidth / (float)resolutionHeight;
	//float left = -aspectRatio;
	//float right = aspectRatio;
	//float bottom = -1.0f;
	//float top = 1.0f;

	static float fovY = 60.0f;

	m_projection = cr3d::CameraProjection::Perspective;

	m_resolutionWidth = resolutionWidth;

	m_resolutionHeight = resolutionHeight;

	m_view2ProjectionMatrix = float4x4::perspective(projection(frustum::field_of_view_y(fovY * CrMath::Deg2Rad, m_aspectRatio, 100000.0f, nearPlane), zclip::zero));

	m_projection2ViewMatrix = inverse(m_view2ProjectionMatrix);
}

void CrCamera::Update()
{
	m_view2WorldMatrix = float4x4::identity();
	m_view2WorldMatrix._m00_m01_m02 = m_rightWorldSpace;
	m_view2WorldMatrix._m10_m11_m12 = m_upWorldSpace;
	m_view2WorldMatrix._m20_m21_m22 = m_forwardWorldSpace;
	m_view2WorldMatrix._m30_m31_m32 = m_position;

	m_world2ViewMatrix = inverse(m_view2WorldMatrix);

	m_world2ProjectionMatrix = mul(m_world2ViewMatrix, m_view2ProjectionMatrix);
}

void CrCamera::LookAtPosition(const float3& target, const float3& up)
{
	float3 vz = normalize(target - m_position);
	float3 vx = normalize(cross(up, vz));
	float3 vy = cross(vz, vx);

	m_forwardWorldSpace = vz;
	m_rightWorldSpace = vx;
	m_upWorldSpace = vy;
}

void CrCamera::RotateAround(const float3& pivot, const float3& axis, float angle)
{
	quaternion rotation = quaternion::rotation_axis(axis, angle * CrMath::Deg2Rad);
	float3 currentDirection = m_position - pivot;       // Get current direction
	float3 direction = mul(currentDirection, rotation); // Rotate with quaternion
	m_position = pivot + direction;                     // Get position
}

float4 CrCamera::ComputeProjectionParams() const
{
	float projectionParamsX = 1.0f;
	float projectionParamsY = 1.0f;

	if (m_reverseDepth)
	{
		projectionParamsX = m_farPlane;
		projectionParamsY = (m_nearPlane - m_farPlane) / m_nearPlane;
	}
	else
	{
		projectionParamsX = m_nearPlane;
		projectionParamsY = (m_farPlane - m_nearPlane) / m_farPlane;
	}

	float projectionParamsZ = (m_nearPlane - m_farPlane) * (m_projection == cr3d::CameraProjection::Orthographic ? 1.0f : 0.0f); // Orthographic camera
	float projectionParamsW = 0.0f; // Stereo rendering offset

	return float4(projectionParamsX, projectionParamsY, projectionParamsZ, projectionParamsW);
}

void CrCamera::Translate(const float3& t)
{
	m_position += t;
}

void CrCamera::Rotate(const float3& r)
{
	// Rotate around the Y axis
	quaternion rotationY = quaternion::rotation_y(r.y);

	// Rotate around the camera's previous right axis
	// TODO It may be better to keep hold of the current full rotation
	quaternion rotationX = quaternion::rotation_axis(m_rightWorldSpace, r.x);

	// Combine rotations
	quaternion rotationXY = mul(rotationX, rotationY);

	m_forwardWorldSpace = mul(m_forwardWorldSpace, rotationXY);
	m_upWorldSpace     = mul(m_upWorldSpace,     rotationXY);
	m_rightWorldSpace  = mul(m_rightWorldSpace,  rotationXY);
}

void CrCamera::SetPosition(const float3& p)
{
	m_position = p;
}

void CrCamera::SetFilmWidth(float filmWidth)
{
	m_filmWidthMm = filmWidth;
}

void CrCamera::SetFilmHeight(float filmHeight)
{
	m_filmHeightMm = filmHeight;
}

void CrCamera::SetVerticalFieldOfView(float fovY)
{
	m_fovY = fovY;
}

float3 CrCamera::ProjectWorldSpacePosition(float3 worldSpacePosition)
{
	float4 viewSpacePosition = mul(float4(worldSpacePosition, 1.0f), m_world2ViewMatrix);
	float4 homogeneousSpacePosition = mul(viewSpacePosition, m_view2ProjectionMatrix);
	float3 ndcSpacePosition = homogeneousSpacePosition.xyz / homogeneousSpacePosition.w;
	return ndcSpacePosition;
}

float3 CrCamera::ProjectViewSpacePosition(float3 viewSpacePosition)
{
	float4 homogeneousSpacePosition = mul(float4(viewSpacePosition, 1.0f), m_view2ProjectionMatrix);
	float3 ndcSpacePosition = homogeneousSpacePosition.xyz / homogeneousSpacePosition.w;
	return ndcSpacePosition;
}

float3 CrCamera::GetViewRay(float2 ndcSpacePosition)
{
	// Create a ray whose depth is the near plane
	float4 ndcPositionExtended(ndcSpacePosition, m_nearPlane, 1.0f);

	// Unproject using the full projection matrix
	float4 viewSpacePosition = mul(ndcPositionExtended, m_projection2ViewMatrix);

	// Normalize to get a ray
	return normalize(viewSpacePosition.xyz);
}
