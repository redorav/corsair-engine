#pragma once

#include "Rendering/ICrFramebuffer.h"
#include <vulkan/vulkan.h>

class ICrRenderDevice;

class CrFramebufferVulkan final : public ICrFramebuffer
{
public:

	~CrFramebufferVulkan();

	CrFramebufferVulkan(ICrRenderDevice* renderDevice, const CrFramebufferCreateParams& params);

	VkFramebuffer GetVkFramebuffer() const;

private:

	VkDevice m_vkDevice;

	VkFramebuffer m_vkFramebuffer;
};

inline VkFramebuffer CrFramebufferVulkan::GetVkFramebuffer() const
{
	return m_vkFramebuffer;
}