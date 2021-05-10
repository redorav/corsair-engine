#pragma once

#include "Core/SmartPointers/CrSharedPtr.h"

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class CrRenderModelInstance
{
public:

	void SetRenderModel(const CrRenderModelSharedHandle& renderModel);

	const CrRenderModelSharedHandle& GetRenderModel() const;

private:

	CrRenderModelSharedHandle m_renderModel;
};