#pragma once

#include "CrEntity.h"

#include "Math/CrHlslppMatrixFloat.h"

// TODO Remove the CrEntity hierarchy. This is the engine object
class CrCamera : public CrEntity
{
public:

	struct ProjectionParams
	{
		cr3d::CameraProjection projection;
		float nearPlane;
		float farPlane;

		bool reverseDepth;
		bool infiniteDepth;
	};

	CrCamera();

	CrCamera(float width, float height, float nearPlane, float farPlane);

	void SetupPerspective(float filmWidth, float filmHeight, float nearPlane, float farPlane);

	void Update();

	void Translate(const float3& t);

	void Rotate(const float3& r);

	void SetPosition(const float3& p);

	void SetFilmWidth(float filmWidth);

	void SetFilmHeight(float filmHeight);

	void SetVerticalFieldOfView(float fovY);

	void LookAt(const float3& target, const float3& up);

	void RotateAround(const float3& point, const float3& axis, float angle);

	const float3& GetLookatVector() const { return m_lookAtWorldSpace; }

	const float3& GetRightVector() const { return m_rightWorldSpace; }

	const float3& GetUpVector() const { return m_upWorldSpace; }

	const float4x4& GetWorld2ViewMatrix() const { return m_world2ViewMatrix; }

	const float4x4& GetView2ProjectionMatrix() const { return m_view2ProjectionMatrix; }

	const float4x4& GetWorld2ProjectionMatrix() const { return m_world2ProjectionMatrix; }

private:

	float m_filmWidthMm;
	float m_filmHeightMm;

	float m_fovY;
	//float m_nearPlane;
	//float m_farPlane;

	// Camera vectors in world space
	float3 m_lookAtWorldSpace;
	float3 m_upWorldSpace;
	float3 m_rightWorldSpace;

	float4x4 m_view2WorldMatrix;
	float4x4 m_world2ViewMatrix;
	float4x4 m_view2ProjectionMatrix;
	float4x4 m_world2ProjectionMatrix;
};