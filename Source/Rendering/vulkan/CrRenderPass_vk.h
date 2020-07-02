#pragma once

#include "ICrRenderPass.h"
#include <vulkan/vulkan.h>

class ICrRenderDevice;

class CrRenderPassVulkan final : public ICrRenderPass
{
public:

	CrRenderPassVulkan(ICrRenderDevice* renderDevice, const CrRenderPassDescriptor& renderPassDescriptor);

	~CrRenderPassVulkan();

	VkRenderPass GetVkRenderPass() const;

private:

	static VkAttachmentDescription GetVkAttachmentDescription(const CrAttachmentDescriptor& attachmentDescriptor);

	static VkAttachmentLoadOp GetVkAttachmentLoadOp(CrAttachmentLoadOp loadOp);

	static VkAttachmentStoreOp GetVkAttachmentStoreOp(CrAttachmentStoreOp storeOp);

	VkDevice m_vkDevice;

	VkRenderPass m_vkRenderPass;
};

inline VkRenderPass CrRenderPassVulkan::GetVkRenderPass() const
{
	return m_vkRenderPass;
}