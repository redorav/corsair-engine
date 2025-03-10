#pragma once

#include "Core/CrTypedId.h"

class CrLight;
using CrLightId = CrTypedId<CrLight, uint32_t>;

namespace LightType
{
	enum T
	{
		Point,
		Spot,
		Capsule,
		Quad,
		Directional
	};
};

class CrLight
{
	CrLight()
		: m_colorIntensity(1.0f, 1.0f, 1.0f, 1.0f)
		, m_falloffRadius(1.0f)
		, m_type(LightType::Point)
	{
	}

private:

	float4 m_transform;

	float4 m_colorIntensity;

	float m_falloffRadius;

	LightType::T m_type;
};