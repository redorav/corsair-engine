#pragma once

#include "ICrShaderManager.h"

class CrShaderManagerD3D12 final : public ICrShaderManager
{
public:
	
	virtual void InitPS() override;
	
private:

	virtual CrNativeShaderStage CreateGraphicsShaderStagePS(const unsigned char* byteCode, size_t codeSize, cr3d::ShaderStage::T stage) override;

	virtual void CreateShaderResourceSetPS(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionD3D12& reflection, CrShaderResourceSet& resourceSet) override;

	virtual void CompileStagePS(CrGraphicsShaderStageCreate& shaderStageCreateInfo) override;
};
