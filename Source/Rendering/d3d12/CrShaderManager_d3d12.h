#pragma once

#include "ICrShaderManager.h"

class CrShaderManagerD3D12 final : public ICrShaderManager
{
public:
	
	virtual void InitPS() final override;
	
private:

	virtual CrNativeShaderStage CreateGraphicsShaderStagePS(const unsigned char* byteCode, size_t codeSize, cr3d::ShaderStage::T stage) final override;

	virtual void CreateShaderResourceSetPS(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionD3D12& reflection, CrShaderResourceSet& resourceSet) final override;

	virtual void CompileStagePS(CrGraphicsShaderStageCreate& shaderStageCreateInfo) final override;
};
