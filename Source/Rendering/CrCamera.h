#pragma once

#include "CrEntity.h"

#include "Math/CrHlslppMatrixFloat.h"

#include "Core/SmartPointers/CrIntrusivePtr.h"

// TODO Remove the CrEntity hierarchy. This is the engine object
class CrCamera final : public CrEntity, public CrIntrusivePtrInterface
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

	CrCamera(uint32_t resolutionWidth, uint32_t resolutionHeight, float nearPlane, float farPlane);

	void SetupPerspective(uint32_t resolutionWidth, uint32_t resolutionHeight, float nearPlane, float farPlane);

	void UpdateMatrices();

	void Translate(const float3& t);

	void SetPosition(const float3& p);
	
	void SetCameraRotationVectors(float3 forwardVector, float3 rightVector, float3 upVector);

	void SetNearPlaneWidth(float filmWidth);

	float GetNearPlaneWidth() const;

	void SetNearPlaneHeight(float filmHeight);

	float GetNearPlaneHeight() const;

	void SetVerticalFieldOfView(float fovY);

	float4 ComputeProjectionParams() const;

	float GetNearPlane() const { return m_nearPlane; }

	float GetFarPlane() const { return m_farPlane; }

	float GetAspectRatio() const { return m_aspectRatio; }

	uint32_t GetResolutionWidth() const { return m_resolutionWidth; }

	uint32_t GetResolutionHeight() const { return m_resolutionHeight; }

	const float3& GetForwardVector() const { return m_forwardWorldSpace; }

	const float3& GetRightVector() const { return m_rightWorldSpace; }

	const float3& GetUpVector() const { return m_upWorldSpace; }

	const float4x4& GetWorld2ViewMatrix() const { return m_world2ViewMatrix; }

	const float3x3& GetWorld2ViewRotation() const { return reinterpret_cast<const float3x3&>(m_world2ViewMatrix); }

	const float4x4& GetView2WorldMatrix() const { return m_view2WorldMatrix; }

	const float3x3& GetView2WorldRotation() const { return reinterpret_cast<const float3x3&>(m_view2WorldMatrix); }

	const float4x4& GetView2ProjectionMatrix() const { return m_view2ProjectionMatrix; }

	const float4x4& GetWorld2ProjectionMatrix() const { return m_world2ProjectionMatrix; }

	const float4x4& GetProjection2ViewMatrix() const { return m_projection2ViewMatrix; }

	const float4x4& GetProjection2WorldMatrix() const { return m_projection2WorldMatrix; }

	float3 ProjectWorldSpacePosition(float3 worldSpacePosition);

	float3 ProjectViewSpacePosition(float3 viewSpacePosition);

	float3 GetViewRay(float2 ndcPosition);

private:

	cr3d::CameraProjection m_projection;

	float m_nearPlaneWidth;

	float m_nearPlaneHeight;

	float m_fovY;

	float m_nearPlane;

	float m_farPlane;

	float m_aspectRatio;

	bool m_reverseDepth;

	uint32_t m_resolutionWidth;

	uint32_t m_resolutionHeight;

	// Camera vectors in world space
	float3 m_forwardWorldSpace;

	float3 m_upWorldSpace;

	float3 m_rightWorldSpace;

	// Matrices used in forward projection (from world or view space into NDC space)
	float4x4 m_world2ViewMatrix;

	float4x4 m_view2ProjectionMatrix;

	float4x4 m_world2ProjectionMatrix;

	// Matrices used in back projection (taking from NDC space back to view or world space)
	float4x4 m_view2WorldMatrix;

	float4x4 m_projection2ViewMatrix;

	float4x4 m_projection2WorldMatrix;
};