#pragma once

#include "ICrShaderManager.h"

class CrShaderManagerD3D12 final : public ICrShaderManager
{
public:
	
	virtual void InitPS() override;
	
private:

	virtual void CreateShaderResourceSetPS(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionD3D12& reflection, CrShaderResourceSet& resourceSet) override;
};
