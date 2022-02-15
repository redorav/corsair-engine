#pragma once

#include <vulkan/vulkan.h>
#include "CrVMA.h"

extern PFN_vkDebugMarkerSetObjectTagEXT		vkDebugMarkerSetObjectTag;
extern PFN_vkDebugMarkerSetObjectNameEXT	vkDebugMarkerSetObjectName;
extern PFN_vkCmdDebugMarkerBeginEXT			vkCmdDebugMarkerBegin;
extern PFN_vkCmdDebugMarkerEndEXT			vkCmdDebugMarkerEnd;
extern PFN_vkCmdDebugMarkerInsertEXT		vkCmdDebugMarkerInsert;

namespace crvk
{
	VkFormat GetVkFormat(cr3d::DataFormat::T format);

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

	VkBufferCreateInfo CreateVkBufferCreateInfo(VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, uint32_t* pQueueFamilyIndices);

	VkMemoryAllocateInfo CreateVkMemoryAllocateInfo(VkDeviceSize allocationSize, uint32_t memoryTypeIndex, void* extension = nullptr);

	VkWriteDescriptorSet CreateVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding, uint32_t arrayElement, uint32_t descriptorCount, VkDescriptorType descriptorType, const VkDescriptorImageInfo* imageInfo, const VkDescriptorBufferInfo* bufferInfo, const VkBufferView* texelBufferView);
}
