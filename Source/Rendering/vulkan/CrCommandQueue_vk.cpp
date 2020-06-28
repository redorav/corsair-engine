#include "CrRendering_pch.h"

#include "CrRenderDevice_vk.h"
#include "CrCommandQueue_vk.h"
#include "CrCommandBuffer_vk.h"
#include "CrGPUSynchronization_vk.h"

#include "Core/Logging/ICrDebug.h"

CrCommandQueueVulkan::CrCommandQueueVulkan(ICrRenderDevice* renderDevice, CrCommandQueueType::T/* type*/) 
	: ICrCommandQueue(renderDevice), m_vkQueue(nullptr), m_vkCommandBufferPool(nullptr), m_queueNodeIndex(0)
{
	CrRenderDeviceVulkan* vulkanDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice);

	// Assign this device to the queue
	m_vkDevice = vulkanDevice->GetVkDevice();

	uint32_t commandQueueIndex = vulkanDevice->ReserveVkQueueIndex();
	
	// Create the VkQueue
	vkGetDeviceQueue(m_vkDevice, vulkanDevice->GetVkQueueFamilyIndex(), commandQueueIndex, &m_vkQueue);

	// Create a command pool from which the queue can allocate command buffers
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = vulkanDevice->GetVkQueueFamilyIndex();
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // TODO Transient or Reset? Check Metal and DX12
	VkResult result = vkCreateCommandPool(m_vkDevice, &cmdPoolInfo, nullptr, &m_vkCommandBufferPool);
	CrAssert(result == VK_SUCCESS);
}

ICrCommandBuffer* CrCommandQueueVulkan::CreateCommandBufferPS()
{
	return new CrCommandBufferVulkan(this);
}

void CrCommandQueueVulkan::DestroyCommandBufferPS(const ICrCommandBuffer* commandBuffer)
{
	vkFreeCommandBuffers(m_vkDevice, m_vkCommandBufferPool, 1, &static_cast<const CrCommandBufferVulkan*>(commandBuffer)->GetVkCommandBuffer());
}

void CrCommandQueueVulkan::SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;

	if (signalSemaphore)
	{
		submitInfo.pSignalSemaphores = &static_cast<const CrGPUSemaphoreVulkan*>(signalSemaphore)->GetVkSemaphore();
		submitInfo.signalSemaphoreCount = 1;
	}

	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; // TODO Need more control over this, probably best to put inside the semaphore object

	if (waitSemaphore)
	{
		submitInfo.pWaitSemaphores = &static_cast<const CrGPUSemaphoreVulkan*>(waitSemaphore)->GetVkSemaphore();
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitDstStageMask = &waitStageMask;
	}

	submitInfo.pCommandBuffers = &static_cast<const CrCommandBufferVulkan*>(commandBuffer)->GetVkCommandBuffer();

	VkResult result = vkQueueSubmit(m_vkQueue, 1, &submitInfo, signalFence ? static_cast<const CrGPUFenceVulkan*>(signalFence)->GetVkFence() : nullptr);
	CrAssert(result == VK_SUCCESS);
}

void CrCommandQueueVulkan::WaitIdlePS()
{
	vkQueueWaitIdle(m_vkQueue);
}

VkDevice CrCommandQueueVulkan::GetVkDevice() const
{
	return m_vkDevice;
}

VkQueue CrCommandQueueVulkan::GetVkQueue() const
{
	return m_vkQueue;
}

VkCommandPool CrCommandQueueVulkan::GetVkCommandBufferPool() const
{
	return m_vkCommandBufferPool;
}