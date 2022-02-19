#include "CrRendering_pch.h"

#include "CrGPUBuffer_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

CrHardwareGPUBufferVulkan::CrHardwareGPUBufferVulkan(CrRenderDeviceVulkan* vulkanRenderDevice, const CrHardwareGPUBufferDescriptor& descriptor)
	: ICrHardwareGPUBuffer(vulkanRenderDevice, descriptor)
{
	if (descriptor.usage & cr3d::BufferUsage::Index)
	{
		m_vkIndexType = descriptor.stride == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
	}

	VkResult vkResult = VK_SUCCESS;

	VkDevice vkDevice = vulkanRenderDevice->GetVkDevice();

	VkBufferCreateInfo bufferCreateInfo = crvk::CreateVkBufferCreateInfo
	(
		0,
		descriptor.numElements * descriptor.stride,
		GetVkBufferUsageFlagBits(descriptor.usage, descriptor.access),
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr
	);

	VmaAllocationCreateInfo vmaAllocationCreateInfo = {};

	switch (descriptor.access)
	{
		case cr3d::MemoryAccess::GPUOnly:
			vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			break;
		case cr3d::MemoryAccess::GPUWriteCPURead:
			vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			vmaAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case cr3d::MemoryAccess::Staging:
			vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			vmaAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case cr3d::MemoryAccess::CPUStreamToGPU:
		{
			vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			vmaAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

			// From the VMA documentation:
			// Also, Windows drivers from all 3 * *PC * *GPU vendors(AMD, Intel, NVIDIA)
			// currently provide `HOST_COHERENT` flag on all memory types that are
			// `HOST_VISIBLE`, so on this platform you may not need to bother.

			// If we want different behavior and manual flushing on other platforms we'll have to change this
			vmaAllocationCreateInfo.requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			break;
		}
		default: break;
	}

	VmaAllocationInfo vmaAllocationInfo;
	vkResult = vmaCreateBuffer(vulkanRenderDevice->GetVmaAllocator(), &bufferCreateInfo, &vmaAllocationCreateInfo, &m_vkBuffer, &m_vmaAllocation, &vmaAllocationInfo);
	CrAssert(vkResult == VK_SUCCESS);

	vulkanRenderDevice->SetVkObjectName((uint64_t)m_vkBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, descriptor.name);

	if (descriptor.usage & cr3d::BufferUsage::Data)
	{
		CrAssert(descriptor.dataFormat != cr3d::DataFormat::Count);

		VkBufferViewCreateInfo vkBufferViewCreateInfo;
		vkBufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		vkBufferViewCreateInfo.pNext = nullptr;
		vkBufferViewCreateInfo.flags = 0;
		vkBufferViewCreateInfo.buffer = m_vkBuffer;
		vkBufferViewCreateInfo.format = crvk::GetVkFormat(descriptor.dataFormat);
		vkBufferViewCreateInfo.offset = 0;
		vkBufferViewCreateInfo.range = vmaAllocationInfo.size;

		vkResult = vkCreateBufferView(vkDevice, &vkBufferViewCreateInfo, nullptr, &m_vkBufferView);
		CrAssert(vkResult == VK_SUCCESS);
	}
}

CrHardwareGPUBufferVulkan::~CrHardwareGPUBufferVulkan()
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);

	if (m_vkBufferView)
	{
		vkDestroyBufferView(vulkanRenderDevice->GetVkDevice(), m_vkBufferView, nullptr);
	}

	vmaDestroyBuffer(vulkanRenderDevice->GetVmaAllocator(), m_vkBuffer, m_vmaAllocation);
}

VkBufferUsageFlags CrHardwareGPUBufferVulkan::GetVkBufferUsageFlagBits(cr3d::BufferUsage::T usage, cr3d::MemoryAccess::T access)
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
		if (access & cr3d::MemoryAccess::GPUOnly)
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

	if (usage & cr3d::BufferUsage::TransferDst)
	{
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	if (usage & cr3d::BufferUsage::TransferSrc)
	{
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	}

	return usageFlags;
}

void* CrHardwareGPUBufferVulkan::LockPS()
{
	void* data = nullptr;

	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);
	VkResult vkResult = vmaMapMemory(vulkanRenderDevice->GetVmaAllocator(), m_vmaAllocation, &data);
	CrAssert(vkResult == VK_SUCCESS);
	return data;
}

void CrHardwareGPUBufferVulkan::UnlockPS()
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);
	vmaUnmapMemory(vulkanRenderDevice->GetVmaAllocator(), m_vmaAllocation);
}

CrArray<CrVkBufferStateInfo, cr3d::BufferState::Count> CrVkBufferResourceStateTable;

static bool PopulateVkBufferResourceTable()
{
	CrVkBufferResourceStateTable[cr3d::BufferState::Undefined]       = { VK_ACCESS_NONE_KHR };
	CrVkBufferResourceStateTable[cr3d::BufferState::ShaderInput]     = { VK_ACCESS_SHADER_READ_BIT };
	CrVkBufferResourceStateTable[cr3d::BufferState::ReadWrite]       = { VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT };
	CrVkBufferResourceStateTable[cr3d::BufferState::CopySource]      = { VK_ACCESS_TRANSFER_READ_BIT };
	CrVkBufferResourceStateTable[cr3d::BufferState::CopyDestination] = { VK_ACCESS_TRANSFER_WRITE_BIT };

	for (const CrVkBufferStateInfo& resourceInfo : CrVkBufferResourceStateTable)
	{
		CrAssertMsg(resourceInfo.accessMask != VK_ACCESS_FLAG_BITS_MAX_ENUM, "Resource info entry is invalid");
	}

	return true;
};

static bool DummyPopulateVkBufferResourceTable = PopulateVkBufferResourceTable();

const CrVkBufferStateInfo& CrHardwareGPUBufferVulkan::GetVkBufferStateInfo(cr3d::BufferState::T bufferState)
{
	return CrVkBufferResourceStateTable[bufferState];
}