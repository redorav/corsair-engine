#include "Rendering/CrRendering_pch.h"

#include "CrGPUBuffer_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

CrHardwareGPUBufferVulkan::CrHardwareGPUBufferVulkan(CrRenderDeviceVulkan* vulkanRenderDevice, const CrHardwareGPUBufferDescriptor& descriptor)
	: ICrHardwareGPUBuffer(vulkanRenderDevice, descriptor)
{
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
		case cr3d::MemoryAccess::GPUOnlyWrite:
		case cr3d::MemoryAccess::GPUOnlyRead:
			vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			break;
		case cr3d::MemoryAccess::GPUWriteCPURead:
			vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			vmaAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			break;
		case cr3d::MemoryAccess::StagingUpload:
		case cr3d::MemoryAccess::StagingDownload:
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

	vulkanRenderDevice->SetVkObjectName((uint64_t)m_vkBuffer, VK_OBJECT_TYPE_BUFFER, descriptor.name);

	if (descriptor.usage & cr3d::BufferUsage::Typed)
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

	if (descriptor.initialData)
	{
		CrAssertMsg(descriptor.initialDataSize <= vmaAllocationInfo.size, "Not enough memory in buffer");

		if (descriptor.access == cr3d::MemoryAccess::GPUOnlyWrite)
		{
			uint8_t* bufferData = m_renderDevice->BeginBufferUpload(this);
			{
				memcpy(bufferData, descriptor.initialData, descriptor.initialDataSize);
			}
			m_renderDevice->EndBufferUpload(this);
		}
		else
		{
			void* data;
			vkResult = vmaMapMemory(vulkanRenderDevice->GetVmaAllocator(), m_vmaAllocation, &data);
			CrAssert(vkResult == VK_SUCCESS);
			memcpy(data, descriptor.initialData, descriptor.initialDataSize);
			vmaUnmapMemory(vulkanRenderDevice->GetVmaAllocator(), m_vmaAllocation);
		}
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

	if (usage & cr3d::BufferUsage::Typed)
	{
		if (access & cr3d::MemoryAccess::GPUOnlyWrite)
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

VkPipelineStageFlags CrHardwareGPUBufferVulkan::GetVkPipelineStageFlags(cr3d::BufferState::T bufferState, cr3d::ShaderStageFlags::T shaderStages)
{
	VkPipelineStageFlags pipelineFlags = 0;

	pipelineFlags |= crvk::GetVkPipelineStageFlagsFromShaderStages(shaderStages);

	if (bufferState == cr3d::BufferState::IndirectArgument)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	}

	return pipelineFlags;
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

crstl::array<CrVkBufferStateInfo, cr3d::BufferState::Count> CrVkBufferResourceStateTable;

static bool PopulateVkBufferResourceTable()
{
	CrVkBufferResourceStateTable[cr3d::BufferState::Undefined]        = { VK_ACCESS_NONE_KHR };
	CrVkBufferResourceStateTable[cr3d::BufferState::ShaderInput]      = { VK_ACCESS_SHADER_READ_BIT };
	CrVkBufferResourceStateTable[cr3d::BufferState::ReadWrite]        = { VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT };
	CrVkBufferResourceStateTable[cr3d::BufferState::CopySource]       = { VK_ACCESS_TRANSFER_READ_BIT };
	CrVkBufferResourceStateTable[cr3d::BufferState::CopyDestination]  = { VK_ACCESS_TRANSFER_WRITE_BIT };
	CrVkBufferResourceStateTable[cr3d::BufferState::IndirectArgument] = { VK_ACCESS_INDIRECT_COMMAND_READ_BIT } ;

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