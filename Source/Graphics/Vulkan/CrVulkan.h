#pragma once

#include <vulkan/vulkan.h>

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrDataFormats.h"

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

	VkFormat GetVkFormat(cr3d::DataFormat::T format);

	inline VkIndexType GetVkIndexType(cr3d::DataFormat::T format)
	{
		switch (format)
		{
			case cr3d::DataFormat::R32_Uint: return VK_INDEX_TYPE_UINT32;
			case cr3d::DataFormat::R16_Uint: return VK_INDEX_TYPE_UINT16;
			case cr3d::DataFormat::R8_Uint: return VK_INDEX_TYPE_UINT8_EXT;
			default: return VK_INDEX_TYPE_UINT32;
		}
	}

	cr3d::DataFormat::T GetDataFormat(VkFormat vkFormat);

	VkSamplerAddressMode GetVkAddressMode(cr3d::AddressMode addressMode);

	VkFilter GetVkFilter(cr3d::Filter filter);

	VkSamplerMipmapMode GetVkMipmapMode(cr3d::Filter filter);

	VkBorderColor GetVkBorderColor(cr3d::BorderColor borderColor);

	VkBlendOp GetVkBlendOp(cr3d::BlendOp blendOp);

	VkBlendFactor GetVkBlendFactor(cr3d::BlendFactor blendFactor);

	VkCompareOp GetVkCompareOp(cr3d::CompareOp compareOp);

	VkStencilOp GetVkStencilOp(cr3d::StencilOp stencilOp);

	VkSampleCountFlagBits GetVkSampleCount(cr3d::SampleCount sampleCount);

	VkShaderStageFlagBits GetVkShaderStage(cr3d::ShaderStage::T shaderStage);
	
	VkPrimitiveTopology GetVkPrimitiveTopology(cr3d::PrimitiveTopology primitiveTopology);

	VkVertexInputRate GetVkVertexInputRate(cr3d::VertexInputRate vertexInputRate);

	VkPolygonMode GetVkPolygonFillMode(cr3d::PolygonFillMode fillMode);

	VkCullModeFlags GetVkPolygonCullMode(cr3d::PolygonCullMode cullMode);

	VkFrontFace GetVkFrontFace(cr3d::FrontFace frontFace);

	VkDescriptorType GetVkDescriptorType(cr3d::ShaderResourceType::T resourceType);

	VkAttachmentLoadOp GetVkAttachmentLoadOp(CrRenderTargetLoadOp loadOp);

	VkAttachmentStoreOp GetVkAttachmentStoreOp(CrRenderTargetStoreOp storeOp);

	VkPipelineStageFlags GetVkPipelineStageFlagsFromShaderStages(cr3d::ShaderStageFlags::T shaderStages);

	VkImageAspectFlags GetVkImageAspectFlags(cr3d::DataFormat::T textureFormat);

	VkImageTransitionInfo GetVkImageStateInfo(cr3d::DataFormat::T textureFormat, cr3d::TextureLayout::T textureLayout);

	VkPipelineStageFlags GetVkPipelineStageFlags(const cr3d::TextureState& textureState);

	VkBufferCreateInfo CreateVkBufferCreateInfo(VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, uint32_t* pQueueFamilyIndices);

	VkMemoryAllocateInfo CreateVkMemoryAllocateInfo(VkDeviceSize allocationSize, uint32_t memoryTypeIndex, void* extension = nullptr);

	VkWriteDescriptorSet CreateVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding, uint32_t arrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, const VkDescriptorImageInfo* imageInfo, const VkDescriptorBufferInfo* bufferInfo, const VkBufferView* texelBufferView);
}
