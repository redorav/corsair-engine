#include "CrRendering_pch.h"

#include "ICrShader.h"
#include "ShaderResources.h"

#include "Core/Logging/ICrDebug.h"

ICrShaderBindingTable::ICrShaderBindingTable(const CrShaderBindingTableResources& resources)
{
	size_t totalResourceCount =
		resources.constantBuffers.size() + 
		resources.samplers.size() + 
		resources.textures.size() + 
		resources.rwTextures.size() + 
		resources.storageBuffers.size() + 
		resources.rwStorageBuffers.size() + 
		resources.dataBuffers.size() + 
		resources.rwDataBuffers.size();

	CrAssert(totalResourceCount < m_bindings.capacity());

	{
		m_constantBufferOffset = (uint8_t)m_bindings.size();
		m_constantBufferCount = (uint8_t)resources.constantBuffers.size();
		m_bindings.insert(m_bindings.end(), resources.constantBuffers.begin(), resources.constantBuffers.end());

		m_samplerOffset = (uint8_t)m_bindings.size();
		m_samplerCount = (uint8_t)resources.samplers.size();
		m_bindings.insert(m_bindings.end(), resources.samplers.begin(), resources.samplers.end());

		m_textureOffset = (uint8_t)m_bindings.size();
		m_textureCount = (uint8_t)resources.textures.size();
		m_bindings.insert(m_bindings.end(), resources.textures.begin(), resources.textures.end());

		m_rwDataBufferOffset = (uint8_t)m_bindings.size();
		m_rwDataBufferCount = (uint8_t)resources.rwDataBuffers.size();
		m_bindings.insert(m_bindings.end(), resources.rwDataBuffers.begin(), resources.rwDataBuffers.end());
	}
}