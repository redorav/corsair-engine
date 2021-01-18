#pragma once

#include "Rendering/ICrShaderManager.h"

class CrShaderManagerVulkan final : public ICrShaderManager
{
public:
	
	virtual void InitPS() override;
	
private:

	virtual void CreateShaderResourceTablePS(const CrShaderReflectionVulkan& reflection, CrShaderResourceTable& resourceTable) const override;
};
