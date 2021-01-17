#include "CrRendering_pch.h"

#include "CrCamera.h"

CrCamera::CrCamera() : CrEntity()
, m_view2WorldMatrix(float4x4::identity())
, m_world2ViewMatrix(float4x4::identity())
, m_view2ProjectionMatrix(float4x4::identity())
{}

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

	m_lookAt = vz;
	m_right = vx;
	m_up = vy;
}

void CrCamera::RotateAround(const float3& pivot, const float3& axis, float angle)
{
	quaternion rotation = axisangle(axis, angle * CrMath::Deg2Rad);
	float3 currentDirection = m_position - pivot; // Get current direction
	float3 direction = rotation * currentDirection; // Rotate with quaternion
	m_position = pivot + direction; // Get position
}

void CrCamera::Translate(const float3& t)
{
	m_position += t;
}

void CrCamera::Rotate(const float3& r)
{
	quaternion rot = euler(r);
	m_qrotation *= rot;
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
	m_view2WorldMatrix = float4x4(m_qrotation);
	
	m_view2WorldMatrix._m03 = m_position.x;
	m_view2WorldMatrix._m13 = m_position.y;
	m_view2WorldMatrix._m23 = m_position.z;

	m_world2ViewMatrix = inverse(m_view2WorldMatrix);

	m_lookAt	= mul(m_qrotation, float3(0, 0, 1));
	m_up		= mul(m_qrotation, float3(0, 1, 0));
	m_right		= mul(m_qrotation, float3(1, 0, 0));
}

const hlslpp::float4x4& CrCamera::GetWorld2ViewMatrix() const
{
	return m_world2ViewMatrix;
}

const hlslpp::float4x4& CrCamera::GetView2ProjectionMatrix() const
{
	return m_view2ProjectionMatrix;
}
