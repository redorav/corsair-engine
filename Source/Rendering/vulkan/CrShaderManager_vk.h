#pragma once

#include "Rendering/ICrShaderManager.h"

class CrShaderManagerVulkan final : public ICrShaderManager
{
public:
	
	virtual void InitPS() override;
	
private:

	virtual void CreateShaderResourceSetPS(const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet) const override;
};
