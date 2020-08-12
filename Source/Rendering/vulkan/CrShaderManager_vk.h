#pragma once

#include "ICrShaderManager.h"

class CrShaderManagerVulkan final : public ICrShaderManager
{
public:
	
	virtual void InitPS() override;
	
private:

	virtual void CreateShaderResourceSetPS(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet) const override;
};
