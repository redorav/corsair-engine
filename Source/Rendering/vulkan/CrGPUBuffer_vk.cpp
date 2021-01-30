#include "CrRendering_pch.h"

#include "CrGPUBuffer_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

CrHardwareGPUBufferVulkan::CrHardwareGPUBufferVulkan(CrRenderDeviceVulkan* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor) 
	: ICrHardwareGPUBuffer(descriptor)
{
	if (descriptor.usage & cr3d::BufferUsage::Index)
	{
		m_vkIndexType = descriptor.stride == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
	}

	VkResult vkResult = VK_SUCCESS;

	m_vkDevice = renderDevice->GetVkDevice();

	VkBufferCreateInfo bufferCreateInfo = crvk::CreateVkBufferCreateInfo
	(
		0, descriptor.numElements * descriptor.stride, GetVkBufferUsageFlagBits(descriptor.usage, descriptor.access), 
		VK_SHARING_MODE_EXCLUSIVE, 0, nullptr
	);

	vkResult = vkCreateBuffer(m_vkDevice, &bufferCreateInfo, nullptr, &m_vkBuffer);
	CrAssert(vkResult == VK_SUCCESS);

	// Warn about memory padding here. It seems as a general rule on PC there is a padding of 256 bytes
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(m_vkDevice, m_vkBuffer, &memReqs);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext = nullptr;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = renderDevice->GetVkMemoryType(memReqs.memoryTypeBits, GetVkMemoryPropertyFlags(descriptor.access));
	vkResult = vkAllocateMemory(m_vkDevice, &memAlloc, nullptr, &m_vkMemory);
	CrAssert(vkResult == VK_SUCCESS);

	vkResult = vkBindBufferMemory(m_vkDevice, m_vkBuffer, m_vkMemory, 0);
	CrAssert(vkResult == VK_SUCCESS);
}

CrHardwareGPUBufferVulkan::~CrHardwareGPUBufferVulkan()
{
	vkDestroyBuffer(m_vkDevice, m_vkBuffer, nullptr);

	vkFreeMemory(m_vkDevice, m_vkMemory, nullptr);
}

VkMemoryPropertyFlags CrHardwareGPUBufferVulkan::GetVkMemoryPropertyFlags(cr3d::BufferAccess::T access)
{
	VkMemoryPropertyFlags memoryFlags = 0;

	if (access & cr3d::BufferAccess::GPUWrite)
	{
		memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}

	if (access & (cr3d::BufferAccess::CPUWrite))
	{
		memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	}

	if (access & (cr3d::BufferAccess::CPURead))
	{
		memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}

	return memoryFlags;
}

VkBufferUsageFlags CrHardwareGPUBufferVulkan::GetVkBufferUsageFlagBits(cr3d::BufferUsage::T usage, cr3d::BufferAccess::T access)
{
	VkBufferUsageFlags usageFlags = 0;

	if (usage & cr3d::BufferUsage::Constant)
	{
		usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}

	if (usage & cr3d::BufferUsage::Vertex)
	{
		usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	if (usage & cr3d::BufferUsage::Index)
	{
		usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}

	if (usage & cr3d::BufferUsage::Storage)
	{
		usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}

	if (usage & cr3d::BufferUsage::Data)
	{
		if (access & cr3d::BufferAccess::GPUWrite)
		{
			usageFlags |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		}
		else
		{
			usageFlags |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		}
	}

	if (usage & cr3d::BufferUsage::Indirect)
	{
		usageFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}

	return usageFlags;
}

void* CrHardwareGPUBufferVulkan::LockPS()
{
	void* data = nullptr;
	VkResult result = VK_SUCCESS;

	if (access & (cr3d::BufferAccess::CPURead | cr3d::BufferAccess::CPUWrite))
	{
		VkMemoryMapFlags mapFlags = 0;
		result = vkMapMemory(m_vkDevice, m_vkMemory, 0, VK_WHOLE_SIZE, mapFlags, &data);
	}

	CrAssert(result == VK_SUCCESS);

	return data;
}

void CrHardwareGPUBufferVulkan::UnlockPS()
{
	if (access & (cr3d::BufferAccess::CPURead | cr3d::BufferAccess::CPUWrite))
	{
		vkUnmapMemory(m_vkDevice, m_vkMemory);
	}
}