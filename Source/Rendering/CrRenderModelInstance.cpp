#include "CrRendering_pch.h"

#include "Rendering/CrRenderModelInstance.h"

void CrRenderModelInstance::SetRenderModel(const CrRenderModelSharedHandle& renderModel)
{
	m_renderModel = renderModel;
}

const CrRenderModelSharedHandle& CrRenderModelInstance::GetRenderModel() const
{
	return m_renderModel;
}
