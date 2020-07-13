#pragma once

#include "ICrShaderManager.h"

class CrShaderManagerVulkan final : public ICrShaderManager
{
public:
	
	virtual void InitPS() final override;
	
private:

	virtual VkShaderModule CreateGraphicsShaderStagePS(const unsigned char* byteCode, size_t codeSize, cr3d::ShaderStage::T stage) final override;

	virtual void CreateShaderResourceSetPS(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet) final override;

	virtual void CompileStagePS(CrGraphicsShaderStageCreate& shaderStageCreateInfo) final override;
};
