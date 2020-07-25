#include "CrRendering_pch.h"

#include "CrGPUBuffer.h"

#include "CrGPUStackAllocator_vk.h"
#include "CrRenderDevice_vk.h"

#include "Core/Logging/ICrDebug.h"

#include <vulkan/vulkan.h>

CrGPUStackAllocatorVulkan::CrGPUStackAllocatorVulkan(ICrRenderDevice* renderDevice) 
	: ICrGPUStackAllocator(renderDevice)
{
	m_vkDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();
}

CrGPUStackAllocatorVulkan::~CrGPUStackAllocatorVulkan()
{
	vkDestroyBuffer(m_vkDevice, m_vkBuffer, nullptr);

	vkFreeMemory(m_vkDevice, m_vkDeviceMemory, nullptr);
}

void CrGPUStackAllocatorVulkan::InitPS(size_t size)
{
	VkResult result;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	result = vkCreateBuffer(m_vkDevice, &bufferInfo, nullptr, &m_vkBuffer); // Create a new buffer
	CrAssert(result == VK_SUCCESS);

	// Get memory requirements including size, alignment and memory type
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(m_vkDevice, m_vkBuffer, &memReqs);

	// Allocate memory for the uniform buffers
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = memReqs.size;

	// Gets the appropriate memory type for this type of buffer allocation
	// Only memory types that are visible to the host
	// Host visible needs manual flush, which we do 
	allocInfo.memoryTypeIndex = static_cast<CrRenderDeviceVulkan*>(m_renderDevice)->GetVkMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	result = vkAllocateMemory(m_vkDevice, &allocInfo, nullptr, &m_vkDeviceMemory);
	CrAssertMsg(result != VK_ERROR_OUT_OF_HOST_MEMORY, "Could not allocate memory!");
	
	result = vkBindBufferMemory(m_vkDevice, m_vkBuffer, m_vkDeviceMemory, 0); // Bind memory to buffer
	CrAssert(result == VK_SUCCESS);
}

void* CrGPUStackAllocatorVulkan::BeginPS()
{
	return m_hardwareBuffer->Lock();
}

void CrGPUStackAllocatorVulkan::EndPS()
{
	m_hardwareBuffer->Unlock();
}
