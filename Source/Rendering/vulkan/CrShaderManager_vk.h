#pragma once

#include "ICrShaderManager.h"

class CrShaderManagerVulkan final : public ICrShaderManager
{
public:
	
	virtual void InitPS() override;
	
private:

	virtual void CreateShaderResourceSetPS(const CrVector<CrShaderStageInfo>& shaderStageInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet) const override;
};
