#include "CrRendering_pch.h"

#include "Rendering/CrRenderWorld.h"
#include "Rendering/CrRenderModel.h"

#include "Core/SmartPointers/CrSharedPtr.h"

CrRenderWorld::CrRenderWorld()
{
	m_renderModelInstances.reserve(1000);
}
