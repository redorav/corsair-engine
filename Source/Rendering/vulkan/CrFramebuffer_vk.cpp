#include "CrRendering_pch.h"

#include "CrFramebuffer_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrTexture_vk.h"
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

CrFramebufferVulkan::~CrFramebufferVulkan()
{
	vkDestroyFramebuffer(m_vkDevice, m_vkFramebuffer, nullptr);
}

CrFramebufferVulkan::CrFramebufferVulkan(ICrRenderDevice* renderDevice, const CrFramebufferCreateParams& params) : ICrFramebuffer(params)
{
	m_vkDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();

	// Create render pass that would be compatible with this framebuffer. We use this one to create the framebuffer, even though
	// we can use any other compatible render pass to actually draw.
	CrArray<VkImageView, cr3d::MaxRenderTargets> attachmentImageViews;
	CrArray<VkAttachmentDescription, cr3d::MaxRenderTargets> attachmentDescriptions = {};
	CrArray<VkAttachmentReference, cr3d::MaxRenderTargets> colorReferences;

	// There can only be one depth-stencil attachment
	VkAttachmentReference depthReference;
	VkAttachmentReference* depthReferencePtr = nullptr;

	uint32_t attachmentCount = 0;
	uint32_t colorReferenceCount = 0;

	uint32_t width = params.m_colorTargets[0].texture->GetWidth();
	uint32_t height = params.m_colorTargets[0].texture->GetHeight();

	// TODO Do we allow to have holes?
	for (uint32_t i = 0; i < attachmentDescriptions.size(); ++i)
	{
		const CrFramebufferCreateParams::CrAttachmentProperties& properties = params.m_colorTargets[i];
		const CrTextureVulkan* texture = static_cast<const CrTextureVulkan*>(properties.texture);

		if (texture)
		{
			attachmentDescriptions[i] = texture->GetVkAttachmentDescription();
			attachmentImageViews[i] = texture->GetVkImageViewSingleMipSlice(properties.mipMap, properties.slice);
			colorReferences[colorReferenceCount] = { i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			colorReferenceCount++;
			attachmentCount = i + 1; // Record the last attachment
		}
		else
		{
			// TODO can have holes as long as it's marked as VK_ATTACHMENT_UNUSED?
			// VkAttachmentDescription unused;
		}
	}

	if (params.m_depthTarget.texture)
	{
		const CrTextureVulkan* depthTexture = static_cast<const CrTextureVulkan*>(params.m_depthTarget.texture);
		attachmentDescriptions[attachmentCount] = depthTexture->GetVkAttachmentDescription();
		attachmentImageViews[attachmentCount] = depthTexture->GetVkImageViewSingleMipSlice(params.m_depthTarget.mipMap, params.m_depthTarget.slice);
		depthReference = { attachmentCount, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
		depthReferencePtr = &depthReference;
		attachmentCount++;
	}

	CrAssert(width > 0 && height > 0);

	VkSubpassDescription subpassDescription = { 0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, colorReferenceCount, colorReferences.data(), nullptr, depthReferencePtr, 0, nullptr };

	VkRenderPass dummyVkPass = crvk::CreateVkRenderPass(m_vkDevice, attachmentCount, attachmentDescriptions.data(), 1, &subpassDescription, 0, nullptr);
	
	m_vkFramebuffer = crvk::CreateVkFramebuffer(m_vkDevice, dummyVkPass, attachmentCount, attachmentImageViews.data(), width, height, 1);

	vkDestroyRenderPass(m_vkDevice, dummyVkPass, nullptr);
}
