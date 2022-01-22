#include "CrRendering_pch.h"

#include "ICrShader.h"

#include "Core/Logging/ICrDebug.h"

CrShaderCompilerDefines CrShaderCompilerDefines::Dummy;

ICrShaderBindingLayout::ICrShaderBindingLayout(const CrShaderBindingLayoutResources& resources)
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

		m_rwTextureOffset = (uint8_t)m_bindings.size();
		m_rwTextureCount = (uint8_t)resources.rwTextures.size();
		m_bindings.insert(m_bindings.end(), resources.rwTextures.begin(), resources.rwTextures.end());

		m_storageBufferOffset = (uint8_t)m_bindings.size();
		m_storageBufferCount = (uint8_t)resources.storageBuffers.size();
		m_bindings.insert(m_bindings.end(), resources.storageBuffers.begin(), resources.storageBuffers.end());

		m_rwStorageBufferOffset = (uint8_t)m_bindings.size();
		m_rwStorageBufferCount = (uint8_t)resources.rwStorageBuffers.size();
		m_bindings.insert(m_bindings.end(), resources.rwStorageBuffers.begin(), resources.rwStorageBuffers.end());

		m_rwDataBufferOffset = (uint8_t)m_bindings.size();
		m_rwDataBufferCount = (uint8_t)resources.rwDataBuffers.size();
		m_bindings.insert(m_bindings.end(), resources.rwDataBuffers.begin(), resources.rwDataBuffers.end());
	}
}