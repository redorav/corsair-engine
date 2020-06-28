#include "CrRendering_pch.h"

#include "CrRenderPass_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrCommandBuffer_vk.h"

#include "Core/Logging/ICrDebug.h"

VkAttachmentDescription CrRenderPassVulkan::GetVkAttachmentDescription(const CrAttachmentDescriptor& attachmentDescriptor)
{
	VkAttachmentDescription attachmentDescription;

	attachmentDescription.flags = 0;
	attachmentDescription.format = crvk::GetVkFormat(attachmentDescriptor.format);
	attachmentDescription.samples = crvk::GetVkSampleCount(attachmentDescriptor.samples);
	attachmentDescription.loadOp = GetVkAttachmentLoadOp(attachmentDescriptor.loadOp);
	attachmentDescription.storeOp = GetVkAttachmentStoreOp(attachmentDescriptor.storeOp);
	attachmentDescription.stencilLoadOp = GetVkAttachmentLoadOp(attachmentDescriptor.stencilLoadOp);
	attachmentDescription.stencilStoreOp = GetVkAttachmentStoreOp(attachmentDescriptor.stencilStoreOp);

	VkAccessFlags dummy;
	CrCommandBufferVulkan::GetVkImageLayoutAndAccessFlags(false, attachmentDescriptor.initialState, attachmentDescription.initialLayout, dummy);
	CrCommandBufferVulkan::GetVkImageLayoutAndAccessFlags(false, attachmentDescriptor.finalState, attachmentDescription.finalLayout, dummy);

	return attachmentDescription;
}

CrRenderPassVulkan::CrRenderPassVulkan(ICrRenderDevice* renderDevice, const CrRenderPassDescriptor& renderPassDescriptor) : m_vkRenderPass(nullptr)
{
	m_vkDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();

	// Attachment are up to number of render targets, plus depth
	CrArray<VkAttachmentDescription, cr3d::MaxRenderTargets + 1> attachments;

	// Attachment references are set in subpasses
	CrArray<VkAttachmentReference, cr3d::MaxRenderTargets> colorReferences;
	VkAttachmentReference depthReference;

	uint32_t numColorAttachments = 0;
	uint32_t numDepthAttachments = 0;

	for (uint32_t i = 0; i < renderPassDescriptor.m_colorAttachments.size(); ++i)
	{
		const CrAttachmentDescriptor& attachmentDescriptor = renderPassDescriptor.m_colorAttachments[i];

		if (attachmentDescriptor.format != cr3d::DataFormat::Invalid)
		{
			colorReferences[numColorAttachments].attachment = i;
			colorReferences[numColorAttachments].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachments[i] = GetVkAttachmentDescription(attachmentDescriptor);
			numColorAttachments++;
		}
		else
		{
			break;
		}
	}

	if (renderPassDescriptor.m_depthAttachment.format != cr3d::DataFormat::Invalid)
	{
		depthReference.attachment = numColorAttachments;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[numColorAttachments] = GetVkAttachmentDescription(renderPassDescriptor.m_depthAttachment);
		numDepthAttachments++;
	}

	// All render passes need at least one subpass to work. By defining a subpass dependency as external on both sides, we get the simplest render pass.
	VkSubpassDescription subpassDescription;
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = numColorAttachments;
	subpassDescription.pColorAttachments = colorReferences.data();
	subpassDescription.pResolveAttachments = nullptr;
	subpassDescription.pDepthStencilAttachment = numDepthAttachments > 0 ? &depthReference : nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	CrArray<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Create the renderpass
	VkRenderPassCreateInfo renderPassInfo;
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.flags = 0;
	renderPassInfo.attachmentCount = numColorAttachments + numDepthAttachments;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = (uint32_t)dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	VkResult result = vkCreateRenderPass(m_vkDevice, &renderPassInfo, nullptr, &m_vkRenderPass);
	CrAssert(result == VK_SUCCESS);
}

CrRenderPassVulkan::~CrRenderPassVulkan()
{
	vkDestroyRenderPass(m_vkDevice, m_vkRenderPass, nullptr);
}

VkAttachmentLoadOp CrRenderPassVulkan::GetVkAttachmentLoadOp(CrAttachmentLoadOp loadOp)
{
	switch (loadOp)
	{
		case CrAttachmentLoadOp::Clear:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case CrAttachmentLoadOp::DontCare:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		case CrAttachmentLoadOp::Load:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		default:
			return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
	}
}

VkAttachmentStoreOp CrRenderPassVulkan::GetVkAttachmentStoreOp(CrAttachmentStoreOp storeOp)
{
	switch (storeOp)
	{
		case CrAttachmentStoreOp::DontCare:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		case CrAttachmentStoreOp::Store:
			return VK_ATTACHMENT_STORE_OP_STORE;
		default:
			return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
	}
}
