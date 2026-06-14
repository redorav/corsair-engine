#pragma once

#include "Graphics/ICrPipeline.h"

namespace crgfx
{
	class DeviceVulkan;
};

class CrGraphicsPipelineVulkan final : public ICrGraphicsPipeline
{
public:

	CrGraphicsPipelineVulkan
	(
		crgfx::DeviceVulkan* vulkanRenderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor, 
		const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor
	);

	~CrGraphicsPipelineVulkan();

	VkPipeline GetVkPipeline() const { return m_vkPipeline; }

	VkPipelineLayout GetVkPipelineLayout() const { return m_vkPipelineLayout; }

#if !defined(CR_CONFIG_FINAL)

	virtual void RecompilePS(crgfx::IDevice* renderDevice, const CrGraphicsShaderHandle& graphicsShader) override;

#endif

private:

	void Initialize(crgfx::DeviceVulkan* vulkanRenderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor);

	void Deinitialize();

	// Describes the resource binding layout for this pipeline
	// This is later used to bind the descriptor sets
	VkPipelineLayout m_vkPipelineLayout;

	VkPipeline m_vkPipeline;
};

class CrComputePipelineVulkan final : public ICrComputePipeline
{
public:

	CrComputePipelineVulkan(crgfx::DeviceVulkan* vulkanRenderDevice, const CrComputeShaderHandle& computeShader);

	~CrComputePipelineVulkan();

	VkPipeline GetVkPipeline() const { return m_vkPipeline; }

	VkPipelineLayout GetVkPipelineLayout() const { return m_vkPipelineLayout; }

#if !defined(CR_CONFIG_FINAL)

	virtual void RecompilePS(crgfx::IDevice* renderDevice, const CrComputeShaderHandle& computeShader) override;

#endif

private:

	void Initialize(crgfx::DeviceVulkan* vulkanRenderDevice, const CrComputeShaderHandle& computeShader);

	void Deinitialize();

	VkPipelineLayout m_vkPipelineLayout;

	VkPipeline m_vkPipeline;
};