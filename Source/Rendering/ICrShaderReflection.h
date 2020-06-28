#pragma once

#include "Core/Containers/CrVector.h"

using bindpoint_t = uint8_t;

class CrShaderResource
{
public:
	const char* name;
	bindpoint_t bindPoint;

	static CrShaderResource Invalid;
};

/** 
 * Provides shader reflection functionality. After a shader has been compiled or loaded the shader reflection structure can
 * be queried for information regarding resource usage.
 */

class ICrShaderReflection
{
public:

	void AddShaderStage(cr3d::ShaderStage::T stage, const CrVector<unsigned char>& bytecode)
	{
		AddShaderStagePS(stage, bytecode);
	}
	
	CrShaderResource GetResource(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType, uint32_t index) const
	{
		return GetResourcePS(stage, resourceType, index);
	}

	uint32_t GetResourceCount(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType) const
	{
		return GetResourceCountPS(stage, resourceType);
	}

protected:

	virtual void AddShaderStagePS(cr3d::ShaderStage::T stage, const CrVector<unsigned char>& bytecode) = 0;

	virtual CrShaderResource GetResourcePS(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType, uint32_t index) const = 0;

	virtual uint32_t GetResourceCountPS(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType) const = 0;
};
