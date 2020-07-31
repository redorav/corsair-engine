#pragma once

#include "ICrShaderManager.h"

class CrShaderManagerVulkan final : public ICrShaderManager
{
public:
	
	virtual void InitPS() override;
	
private:

	virtual VkShaderModule CreateGraphicsShaderStagePS(const unsigned char* byteCode, size_t codeSize, cr3d::ShaderStage::T stage) override;

	virtual void CreateShaderResourceSetPS(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet) override;

	virtual void CompileStagePS(CrGraphicsShaderStageCreate& shaderStageCreateInfo) override;
};
