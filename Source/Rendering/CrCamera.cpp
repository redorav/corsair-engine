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

void CrCamera::SetupPerspective(float filmWidth, float filmHeight, float nearPlane, float farPlane)
{
	// Create a new perspective projection matrix. The height will stay the same while the width will vary as per aspect ratio.
	float aspectRatio = filmWidth / filmHeight;
	//float left = -aspectRatio;
	//float right = aspectRatio;
	//float bottom = -1.0f;
	//float top = 1.0f;

	static float fovY = 60.0f;

	//m_view2ProjectionMatrix = CreateProjectionMatrix(left, right, top, bottom, nearPlane, farPlane);
	m_view2ProjectionMatrix = CreateProjectionMatrix(fovY * CrMath::Deg2Rad, aspectRatio, nearPlane, farPlane);
}

float4x4 CrCamera::CreateProjectionMatrix
(
	float fovY, float aspectRatio, float nearPlane, float farPlane
)
{
	float4x4 projectionMatrix;

	// Vulkan projection matrix
	// https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
	// http://dev.theomader.com/depth-precision/
	// NDC is	-1 <= x <=  1
	//			 1 <= y <= -1
	//			 0 <= z <=  1

	// |                 |                  |                    |                  |
	// | 2 * n / (r - l) |        0         |  (r + l) / (r - l) |        0         |
	// |                 |                  |                    |                  |
	// |        0        | -2 * n / (t - b) | -(t + b) / (t - b) |        0         |
	// |                 |                  |                    |                  |
	// |        0        |        0         |     f / (f - n)    | -n * f / (f - n) |
	// |                 |                  |                    |                  |
	// |        0        |        0         |         1          |        0         |
	// |                 |                  |                    |                  |

	bool reverseDepth  = true;
	bool infiniteDepth = true;
	bool columnMajor   = true;

	float f = farPlane;
	float n = nearPlane;

	float xBias = 0.0f;
	float yBias = 0.0f;
	float zBias = 0.0f;

	// The values are named according to a column-major matrix (where vectors are columns and matrices are multiplied as M * v)
	// If the parameter column major is false, we simply construct a transposed matrix
	
	// Parameters depending on width and height, field of view and aspect ratio
	float m00 = 1.0f / (aspectRatio * tan(fovY * 0.5f));
	float m11 = 1.0f / tan(fovY * 0.5f);

	// Offset parameters
	float m02 = xBias;
	float m12 = yBias;

	// Parameters depending on far and near plane, plus the z bias
	float m22 = 0.0f;
	float m23 = 0.0f;

	if (reverseDepth)
	{
		if (infiniteDepth)
		{
			m22 = 0.0f;
			m23 = n;
		}
		else
		{
			m22 = 1.0f - f / (f - n);
			m23 = n * f / (f - n);
		}
	}
	else
	{
		if (infiniteDepth)
		{
			m22 = 1.0f;
			m23 = -n;
		}
		else
		{
			m22 = f / (f - n);
			m23 = -n * f / (f - n);
		}
	}

	m22 += zBias;

	if (columnMajor)
	{
		projectionMatrix = float4x4
		(
			 m00, 0.0f,  m02, 0.0f,
			0.0f,  m11,  m12, 0.0f,
			0.0f, 0.0f,  m22,  m23,
			0.0f, 0.0f, 1.0f, 0.0f
		);
	}
	else
	{
		projectionMatrix = float4x4
		(
			 m00, 0.0f, 0.0f, 0.0f,
			0.0f,  m11, 0.0f, 0.0f,
			 m02,  m12,  m22, 1.0f,
			0.0f, 0.0f,  m23, 0.0f
		);
	}

	// TODO based on the API, we need to have different projection matrices
	// because they project into different NDC. This should be the only change we need to make

	return projectionMatrix;
}

float4x4 CrCamera::CreateProjectionMatrix(float left, float right, float top, float bottom, float nearPlane, float farPlane)
{
	float4x4 projectionMatrix(0.0f);

	// Vulkan projection matrix
	// https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
	// http://dev.theomader.com/depth-precision/
	// NDC is	-1 <= x <=  1
	//			 1 <= y <= -1
	//			 0 <= z <=  1

	// |                 |                  |                    |                  |
	// | 2 * n / (r - l) |        0         |  (r + l) / (r - l) |        0         |
	// |                 |                  |                    |                  |
	// |        0        | -2 * n / (t - b) | -(t + b) / (t - b) |        0         |
	// |                 |                  |                    |                  |
	// |        0        |        0         |     f / (f - n)    | -n * f / (f - n) |
	// |                 |                  |                    |                  |
	// |        0        |        0         |         1          |        0         |
	// |                 |                  |                    |                  |

	bool reverseDepth = true;
	//bool infiniteDepth = false;

	float f = farPlane;
	float n = nearPlane;
	float r = right;
	float l = left;
	float t = top;
	float b = bottom;

	projectionMatrix._m00 = 2.0f * n / (r - l);
	projectionMatrix._m02 = (r + l) / (r - l);

	projectionMatrix._m11 = 2.0f * n / (t - b);
	projectionMatrix._m12 = -(t + b) / (t - b);

	if (reverseDepth)
	{
		projectionMatrix._m22 = -f / (f - n) + 1.0f;
		projectionMatrix._m23 = n * f / (f - n);
	}
	else
	{
		projectionMatrix._m22 = f / (f - n);
		projectionMatrix._m23 = -n * f / (f - n);
	}

	projectionMatrix._m32 = 1.0f;

	// TODO based on the API, we need to have different projection matrices
	// because they project into different NDC. This should be the only change we need to make

	return projectionMatrix;
}

void CrCamera::LookAt(const float3& target, const float3& up)
{
	float3 vz = normalize(target - m_position);
	float3 vx = normalize(cross(vz, up));
	float3 vy = normalize(cross(vx, vz));

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
