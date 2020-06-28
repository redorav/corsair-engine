#pragma once

#include "CrEntity.h"

// TODO Remove the CrEntity hierarchy. This is the engine object
class CrCamera : public CrEntity
{
public:

	float3 m_up;
	float3 m_right;
	float3 m_lookAt;

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

	// TODO Remove all these in favor of putting things in the transform
	// Basically Translate, Rotate, etc all go in the transform
	void Translate(const float3& t);
	void Rotate(const float3& r);

	void SetFilmWidth(float filmWidth);

	void SetFilmHeight(float filmHeight);

	void SetVerticalFieldOfView(float fovY);

	void LookAt(const float3& target, const float3& up);

	void RotateAround(const float3& point, const float3& axis, float angle);

	const float4x4& GetWorld2ViewMatrix() const;

	const float4x4& GetView2ProjectionMatrix() const;

private:

	float m_filmWidthMm;
	float m_filmHeightMm;

	float m_fovY;
	//float m_nearPlane;
	//float m_farPlane;

	float4x4 m_view2WorldMatrix;
	float4x4 m_world2ViewMatrix;
	float4x4 m_view2ProjectionMatrix;

	// Sets up the actual hardware perspective matrix by specifying the base parameters. Not intended for outside use.
	float4x4 CreateProjectionMatrix(float left, float right, float top, float bottom, float nearPlane, float farPlane);

	float4x4 CreateProjectionMatrix(float verticalFovY, float aspectRatio, float nearPlane, float farPlane);
};