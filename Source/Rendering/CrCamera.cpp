#include "CrRendering_pch.h"

#include "CrCamera.h"
#include "Math/CrMath.h"
#include "Math/CrHlslppVectorFloat.h"
#include "Math/CrHlslppQuaternion.h"
#include "Math/CrHlslppMatrixFloat.h"

CrCamera::CrCamera() : CrEntity()
, m_view2WorldMatrix(float4x4::identity())
, m_world2ViewMatrix(float4x4::identity())
, m_view2ProjectionMatrix(float4x4::identity())
{
	m_lookAtWorldSpace = float3(0, 0, 1);
	m_upWorldSpace     = float3(0, 1, 0);
	m_rightWorldSpace  = float3(1, 0, 0);
}

CrCamera::CrCamera(float width, float height, float nearPlane, float farPlane) : CrEntity()
{
	SetupPerspective(width, height, nearPlane, farPlane);
}

void CrCamera::SetupPerspective(float filmWidth, float filmHeight, float nearPlane, float /*farPlane*/)
{
	// Create a new perspective projection matrix. The height will stay the same while the width will vary as per aspect ratio.
	float aspectRatio = filmWidth / filmHeight;
	//float left = -aspectRatio;
	//float right = aspectRatio;
	//float bottom = -1.0f;
	//float top = 1.0f;

	static float fovY = 60.0f;

	m_view2ProjectionMatrix = float4x4::perspective(projection(frustum::field_of_view_y(fovY * CrMath::Deg2Rad, aspectRatio, 100000.0f, nearPlane), zclip::zero));
}

void CrCamera::LookAt(const float3& target, const float3& up)
{
	float3 vz = normalize(target - m_position);
	float3 vx = normalize(cross(vz, up));
	float3 vy = cross(vx, vz);

	m_lookAtWorldSpace = vz;
	m_rightWorldSpace = vx;
	m_upWorldSpace = vy;
}

void CrCamera::RotateAround(const float3& pivot, const float3& axis, float angle)
{
	quaternion rotation = quaternion::rotation_axis(axis, angle * CrMath::Deg2Rad);
	float3 currentDirection = m_position - pivot; // Get current direction
	float3 direction = mul(currentDirection, rotation); // Rotate with quaternion
	m_position = pivot + direction; // Get position
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

	m_lookAtWorldSpace = mul(m_lookAtWorldSpace, rotationXY);
	m_upWorldSpace     = mul(m_upWorldSpace,     rotationXY);
	m_rightWorldSpace  = mul(m_rightWorldSpace,  rotationXY);
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

void CrCamera::Update()
{
	m_view2WorldMatrix = float4x4::identity();
	m_view2WorldMatrix._m00_m01_m02 = m_rightWorldSpace;
	m_view2WorldMatrix._m10_m11_m12 = m_upWorldSpace;
	m_view2WorldMatrix._m20_m21_m22 = m_lookAtWorldSpace;
	m_view2WorldMatrix._m30_m31_m32 = m_position;

	m_world2ViewMatrix = inverse(m_view2WorldMatrix);

	m_world2ProjectionMatrix = mul(m_world2ViewMatrix, m_view2ProjectionMatrix);
}