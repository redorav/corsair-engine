#pragma once

#include <vulkan/vulkan.h>

#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/DataFormats.h"

extern PFN_vkSetDebugUtilsObjectNameEXT    vkSetDebugUtilsObjectName;
extern PFN_vkSetDebugUtilsObjectTagEXT     vkSetDebugUtilsObjectTag;
extern PFN_vkQueueBeginDebugUtilsLabelEXT  vkQueueBeginDebugUtilsLabel;
extern PFN_vkQueueEndDebugUtilsLabelEXT    vkQueueEndDebugUtilsLabel;
extern PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabel;
extern PFN_vkCmdBeginDebugUtilsLabelEXT    vkCmdBeginDebugUtilsLabel;
extern PFN_vkCmdEndDebugUtilsLabelEXT      vkCmdEndDebugUtilsLabel;
extern PFN_vkCmdInsertDebugUtilsLabelEXT   vkCmdInsertDebugUtilsLabel;
extern PFN_vkCreateDebugUtilsMessengerEXT  vkCreateDebugUtilsMessenger;
extern PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger;
extern PFN_vkSubmitDebugUtilsMessageEXT    vkSubmitDebugUtilsMessage;

namespace crvk
{
	// Sourced from https://github.com/Tobski/simple_vulkan_synchronization/blob/master/thsvs_simpler_vulkan_synchronization.h
	// This shows how to make the combinations but it tries to tie them with the point in the pipeline at which they
	// are used, whereas we decouple that and get fewer combinations
	struct VkImageTransitionInfo
	{
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_MAX_ENUM;
		VkAccessFlags accessMask = VK_ACCESS_FLAG_BITS_MAX_ENUM;
	};

	VkFormat GetVkFormat(crgfx::DataFormat::T format);

	inline VkIndexType GetVkIndexType(crgfx::DataFormat::T format)
	{
		switch (format)
		{
			case crgfx::DataFormat::R32_Uint: return VK_INDEX_TYPE_UINT32;
			case crgfx::DataFormat::R16_Uint: return VK_INDEX_TYPE_UINT16;
			case crgfx::DataFormat::R8_Uint: return VK_INDEX_TYPE_UINT8_EXT;
			default: return VK_INDEX_TYPE_UINT32;
		}
	}

	crgfx::DataFormat::T GetDataFormat(VkFormat vkFormat);

	VkSamplerAddressMode GetVkAddressMode(crgfx::AddressMode addressMode);

	VkFilter GetVkFilter(crgfx::Filter filter);

	VkSamplerMipmapMode GetVkMipmapMode(crgfx::Filter filter);

	VkBorderColor GetVkBorderColor(crgfx::BorderColor borderColor);

	VkBlendOp GetVkBlendOp(crgfx::BlendOp blendOp);

	VkBlendFactor GetVkBlendFactor(crgfx::BlendFactor blendFactor);

	VkCompareOp GetVkCompareOp(crgfx::CompareOp compareOp);

	VkStencilOp GetVkStencilOp(crgfx::StencilOp stencilOp);

	VkSampleCountFlagBits GetVkSampleCount(crgfx::SampleCount sampleCount);

	VkShaderStageFlagBits GetVkShaderStage(crgfx::ShaderStage::T shaderStage);
	
	VkPrimitiveTopology GetVkPrimitiveTopology(crgfx::PrimitiveTopology primitiveTopology);

	VkVertexInputRate GetVkVertexInputRate(crgfx::VertexInputRate vertexInputRate);

	VkPolygonMode GetVkPolygonFillMode(crgfx::PolygonFillMode fillMode);

	VkCullModeFlags GetVkPolygonCullMode(crgfx::PolygonCullMode cullMode);

	VkFrontFace GetVkFrontFace(crgfx::FrontFace frontFace);

	VkDescriptorType GetVkDescriptorType(crgfx::ShaderResourceType::T resourceType);

	VkAttachmentLoadOp GetVkAttachmentLoadOp(crgfx::RenderTargetLoadOp loadOp);

	VkAttachmentStoreOp GetVkAttachmentStoreOp(crgfx::RenderTargetStoreOp storeOp);

	VkPipelineStageFlags GetVkPipelineStageFlagsFromShaderStages(crgfx::ShaderStageFlags::T shaderStages);

	VkImageAspectFlags GetVkImageAspectFlags(crgfx::DataFormat::T textureFormat);

	VkImageTransitionInfo GetVkImageStateInfo(crgfx::DataFormat::T textureFormat, crgfx::TextureLayout::T textureLayout);

	VkPipelineStageFlags GetVkPipelineStageFlags(const crgfx::TextureState& textureState);

	VkBufferCreateInfo CreateVkBufferCreateInfo(VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, uint32_t* pQueueFamilyIndices);

	VkMemoryAllocateInfo CreateVkMemoryAllocateInfo(VkDeviceSize allocationSize, uint32_t memoryTypeIndex, void* extension = nullptr);

	VkWriteDescriptorSet CreateVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding, uint32_t arrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, const VkDescriptorImageInfo* imageInfo, const VkDescriptorBufferInfo* bufferInfo, const VkBufferView* texelBufferView);
}
