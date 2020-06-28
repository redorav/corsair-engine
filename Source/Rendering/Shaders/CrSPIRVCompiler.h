#pragma once

#include <string>
#include <vector>

#include "Rendering/CrRenderingForwardDeclarations.h"

class CrSPIRVCompiler
{
public:

	static bool HLSLtoSPIRV(const std::string& shaderPath, const std::string& entryPoint, const cr3d::ShaderStage::T shaderStage, std::vector<uint32_t>& spirvBytecode);
	
};