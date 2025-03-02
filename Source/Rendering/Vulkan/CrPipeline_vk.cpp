#include "Rendering/CrRendering_pch.h"
#include "CrPipeline_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrShader_vk.h"

CrGraphicsPipelineVulkan::CrGraphicsPipelineVulkan
(
	CrRenderDeviceVulkan* vulkanRenderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor,
	const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor
)
	: ICrGraphicsPipeline(vulkanRenderDevice, pipelineDescriptor, graphicsShader, vertexDescriptor)
{
	Initialize(vulkanRenderDevice, pipelineDescriptor, graphicsShader, vertexDescriptor);
}

void CrGraphicsPipelineVulkan::Initialize(CrRenderDeviceVulkan* vulkanRenderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor)
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.pNext = nullptr;
	inputAssemblyState.flags = 0;
	inputAssemblyState.topology = crvk::GetVkPrimitiveTopology(pipelineDescriptor.primitiveTopology);
	inputAssemblyState.primitiveRestartEnable = false;

	VkPipelineRasterizationStateCreateInfo rasterizerState {};
	rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerState.pNext = nullptr;
	rasterizerState.flags = 0;
	rasterizerState.polygonMode = crvk::GetVkPolygonFillMode(pipelineDescriptor.rasterizerState.fillMode);
	rasterizerState.cullMode = crvk::GetVkPolygonCullMode(pipelineDescriptor.rasterizerState.cullMode);
	rasterizerState.frontFace = crvk::GetVkFrontFace(pipelineDescriptor.rasterizerState.frontFace);

	// Depth clamp is the opposite of depth clip. See discussion here https://github.com/gpuweb/gpuweb/issues/2100
	rasterizerState.depthClampEnable = !pipelineDescriptor.rasterizerState.depthClipEnable;

	// TODO Complete this section
	rasterizerState.lineWidth = 1.0f;
	//rasterizerState.rasterizerDiscardEnable = false;
	//rasterizerState.depthBiasEnable = false;

	if (pipelineDescriptor.rasterizerState.conservativeRasterization)
	{
		VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterizationState {};
		conservativeRasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
		conservativeRasterizationState.conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
		conservativeRasterizationState.extraPrimitiveOverestimationSize = 0.0f;
		rasterizerState.pNext = &conservativeRasterizationState;
	}

	VkPipelineColorBlendStateCreateInfo colorBlendState;
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.pNext = nullptr;
	colorBlendState.flags = 0;
	colorBlendState.logicOpEnable = false;
	colorBlendState.logicOp = VK_LOGIC_OP_NO_OP;

	CrFixedVector<VkPipelineColorBlendAttachmentState, cr3d::MaxRenderTargets> blendAttachments;
	for (uint32_t i = 0, end = cr3d::MaxRenderTargets; i < end; ++i)
	{
		const CrRenderTargetBlendDescriptor& renderTargetBlend = pipelineDescriptor.blendState.renderTargetBlends[i];
		const CrRenderTargetFormatDescriptor& renderTarget = pipelineDescriptor.renderTargets;

		if (renderTarget.colorFormats[i] != cr3d::DataFormat::Invalid)
		{
			VkPipelineColorBlendAttachmentState& blendAttachment = blendAttachments.push_back();
			blendAttachment.colorWriteMask = renderTargetBlend.colorWriteMask;
			blendAttachment.blendEnable = renderTargetBlend.enable;
			blendAttachment.colorBlendOp = crvk::GetVkBlendOp(renderTargetBlend.colorBlendOp);
			blendAttachment.dstColorBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.dstColorBlendFactor);
			blendAttachment.srcColorBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.srcColorBlendFactor);

			blendAttachment.alphaBlendOp = crvk::GetVkBlendOp(renderTargetBlend.alphaBlendOp);
			blendAttachment.dstAlphaBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.dstAlphaBlendFactor);
			blendAttachment.srcAlphaBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.srcAlphaBlendFactor);
		}
		else
		{
			break;
		}
	}

	colorBlendState.attachmentCount = (uint32_t)blendAttachments.size();
	colorBlendState.pAttachments = blendAttachments.data();
	colorBlendState.blendConstants[0] = 1.0f;
	colorBlendState.blendConstants[1] = 1.0f;
	colorBlendState.blendConstants[2] = 1.0f;
	colorBlendState.blendConstants[3] = 1.0f;

	// Multi sampling state
	VkPipelineMultisampleStateCreateInfo multisampleState;
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.pNext = nullptr;
	multisampleState.flags = 0;
	multisampleState.rasterizationSamples = crvk::GetVkSampleCount(pipelineDescriptor.sampleCount);
	multisampleState.sampleShadingEnable = false;
	multisampleState.minSampleShading = 0.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = false;
	multisampleState.alphaToOneEnable = false;

	// Depth stencil state
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.pNext = nullptr;
	depthStencilState.flags = 0;

	// Depth
	depthStencilState.depthTestEnable = pipelineDescriptor.depthStencilState.depthTestEnable;
	depthStencilState.depthWriteEnable = pipelineDescriptor.depthStencilState.depthWriteEnable;
	depthStencilState.depthCompareOp = crvk::GetVkCompareOp(pipelineDescriptor.depthStencilState.depthCompareOp);
	depthStencilState.depthBoundsTestEnable = pipelineDescriptor.depthStencilState.depthBoundsTestEnable;

	// Stencil
	depthStencilState.stencilTestEnable = pipelineDescriptor.depthStencilState.stencilTestEnable;

	depthStencilState.front.depthFailOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.frontDepthFailOp);
	depthStencilState.front.failOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.frontStencilFailOp);
	depthStencilState.front.passOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.frontStencilPassOp);
	depthStencilState.front.compareOp = crvk::GetVkCompareOp(pipelineDescriptor.depthStencilState.frontStencilCompareOp);
	depthStencilState.front.compareMask = pipelineDescriptor.depthStencilState.stencilReadMask;
	depthStencilState.front.writeMask = pipelineDescriptor.depthStencilState.stencilWriteMask;
	depthStencilState.front.reference = pipelineDescriptor.depthStencilState.reference;

	depthStencilState.back.depthFailOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.backDepthFailOp);
	depthStencilState.back.failOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.backStencilFailOp);
	depthStencilState.back.passOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.backStencilPassOp);
	depthStencilState.back.compareOp = crvk::GetVkCompareOp(pipelineDescriptor.depthStencilState.backStencilCompareOp);
	depthStencilState.back.compareMask = pipelineDescriptor.depthStencilState.stencilReadMask;
	depthStencilState.back.writeMask = pipelineDescriptor.depthStencilState.stencilWriteMask;
	depthStencilState.back.reference = pipelineDescriptor.depthStencilState.reference;

	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;

	// Viewport state
	VkPipelineViewportStateCreateInfo viewportState;
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	viewportState.flags = 0;
	viewportState.viewportCount = 1; // One viewport
	viewportState.pViewports = nullptr;
	viewportState.scissorCount = 1; // One scissor rectangle
	viewportState.pScissors = nullptr;

	// Dynamic states can be set even after the pipeline has been created, so there is no need to create new pipelines
	// just for changing a viewport's dimensions or a scissor box
	// The dynamic state properties themselves are stored in the command buffer
	crstl::array<VkDynamicState, 3> dynamicStateEnables =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_STENCIL_REFERENCE
	};

	VkPipelineDynamicStateCreateInfo dynamicState;
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = nullptr;
	dynamicState.flags = 0;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();

	VkResult vkResult;

	// Shader information
	VkPipelineShaderStageCreateInfo shaderStages[cr3d::ShaderStage::GraphicsStageCount] = {};
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
	shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

	const CrGraphicsShaderVulkan* vulkanGraphicsShader = static_cast<const CrGraphicsShaderVulkan*>(graphicsShader.get());

	const CrVector<VkShaderModule>& vkShaderModules = vulkanGraphicsShader->GetVkShaderModules();

	const CrVector<CrShaderBytecodeHandle>& bytecodes = vulkanGraphicsShader->GetBytecodes();

	uint32_t usedShaderStages = 0;

	for (uint32_t i = 0; i < bytecodes.size(); ++i)
	{
		const CrShaderBytecodeHandle& bytecode = bytecodes[i];
		const VkShaderModule vkShaderModule = vkShaderModules[i];

		shaderStageCreateInfo.module = vkShaderModule;
		shaderStageCreateInfo.stage = crvk::GetVkShaderStage(bytecode->GetShaderStage());
		shaderStageCreateInfo.pName = bytecode->GetEntryPoint().c_str();
		shaderStages[usedShaderStages++] = shaderStageCreateInfo;
	}

	VkDescriptorSetLayout descriptorSetLayouts[] = { vulkanGraphicsShader->GetVkDescriptorSetLayout() };

	// Pipeline Resource Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1; // TODO Handle when we have more than one layout
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	// TODO Push constants? Need to be part of the psoDescriptor?

	vkResult = vkCreatePipelineLayout(vulkanRenderDevice->GetVkDevice(), &pipelineLayoutCreateInfo, nullptr, &m_vkPipelineLayout);

	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create pipeline layout");

	uint32_t vertexStreamCount = vertexDescriptor.GetStreamCount();
	crstl::array<VkVertexInputBindingDescription, cr3d::MaxVertexStreams> bindingDescriptions;

	for (uint32_t streamId = 0; streamId < vertexStreamCount; ++streamId)
	{
		bindingDescriptions[streamId].binding = streamId;
		bindingDescriptions[streamId].stride = vertexDescriptor.GetStreamStride(streamId);
		bindingDescriptions[streamId].inputRate = crvk::GetVkVertexInputRate(vertexDescriptor.GetInputRate(streamId));
	}

	uint32_t attributeCount = vertexDescriptor.GetAttributeCount();
	crstl::array<VkVertexInputAttributeDescription, cr3d::MaxVertexAttributes> attributeDescriptions;

	uint32_t offset = 0;
	uint32_t streamId = 0;
	for (uint32_t i = 0; i < attributeCount; ++i)
	{
		const CrVertexAttribute& vertexAttribute = vertexDescriptor.GetAttribute(i);
		const cr3d::DataFormatInfo& vertexFormatInfo = cr3d::DataFormats[vertexAttribute.format];
		attributeDescriptions[i].binding = vertexAttribute.streamId;
		attributeDescriptions[i].location = i; // We assume attributes come in the order the shader expects them
		attributeDescriptions[i].format = crvk::GetVkFormat((cr3d::DataFormat::T)vertexAttribute.format);

		if (streamId != vertexAttribute.streamId)
		{
			streamId = vertexAttribute.streamId;
			offset = 0;
		}

		attributeDescriptions[i].offset = offset;
		offset += vertexFormatInfo.dataOrBlockSize;
	}

	// Assign to vertex buffer
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.pNext = nullptr;
	vertexInputState.flags = 0;
	vertexInputState.vertexBindingDescriptionCount = vertexStreamCount;
	vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputState.vertexAttributeDescriptionCount = attributeCount;
	vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

	// We don't need the real renderpass here to create pipeline state objects, a compatible one is enough
	// to make it work. I suspect the only thing it really needs is the description of how we're going to
	// be using the render pass. In D3D12 this is actually part of the pipeline descriptor (RTVFormats, DSVFormat)
	// TODO This should probably be optimized either by having another function that passes an actual render pass
	// or by having a cache of render pass descriptors. The reason for using a dummy render pass is so that
	// pipelines can be precreated without needing access to the actual draw commands. Another alternative is
	// to create the render pass on the stack by using a custom allocator.
	VkRenderPass vkCompatibleRenderPass;
	{
		CrFixedVector<VkAttachmentDescription, cr3d::MaxRenderTargets + 1> attachments;
		CrFixedVector<VkAttachmentReference, cr3d::MaxRenderTargets> colorReferences;
		VkAttachmentReference depthReference;

		uint32_t numColorAttachments = colorBlendState.attachmentCount;
		uint32_t numDepthAttachments = 0;

		for (uint32_t i = 0; i < numColorAttachments; ++i)
		{
			colorReferences.push_back({ i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			VkAttachmentDescription attachmentDescription = {};
			attachmentDescription.flags = 0;
			attachmentDescription.format = crvk::GetVkFormat(pipelineDescriptor.renderTargets.colorFormats[i]);
			attachmentDescription.samples = crvk::GetVkSampleCount(pipelineDescriptor.renderTargets.sampleCount);
			attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
			attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments.push_back(attachmentDescription);
		}

		if (pipelineDescriptor.renderTargets.depthFormat != cr3d::DataFormat::Invalid)
		{
			depthReference = { numColorAttachments, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			VkAttachmentDescription attachmentDescription = {};
			attachmentDescription.flags = 0;
			attachmentDescription.format = crvk::GetVkFormat(pipelineDescriptor.renderTargets.depthFormat);
			attachmentDescription.samples = crvk::GetVkSampleCount(pipelineDescriptor.renderTargets.sampleCount);
			attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
			attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments.push_back(attachmentDescription);
			numDepthAttachments = 1;
		}

		// All render passes need at least one subpass to work.
		// By defining a subpass dependency as external on both sides, we get the simplest render pass
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

		// Create the renderpass
		VkRenderPassCreateInfo renderPassInfo;
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.flags = 0;
		renderPassInfo.attachmentCount = (uint32_t)attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 0;
		renderPassInfo.pDependencies = nullptr;

		vkResult = vkCreateRenderPass(vulkanRenderDevice->GetVkDevice(), &renderPassInfo, nullptr, &vkCompatibleRenderPass);
		CrAssert(vkResult == VK_SUCCESS);
	}

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = (uint32_t)graphicsShader->GetBytecodes().size();
	graphicsPipelineCreateInfo.pStages = shaderStages;

	graphicsPipelineCreateInfo.layout = m_vkPipelineLayout;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
	graphicsPipelineCreateInfo.renderPass = vkCompatibleRenderPass;

	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizerState;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendState;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleState;
	graphicsPipelineCreateInfo.pViewportState = &viewportState;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilState;
	graphicsPipelineCreateInfo.pDynamicState = &dynamicState;

	vkResult = vkCreateGraphicsPipelines(vulkanRenderDevice->GetVkDevice(), vulkanRenderDevice->GetVkPipelineCache(), 1, &graphicsPipelineCreateInfo, nullptr, &m_vkPipeline);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create graphics pipeline");

	vulkanRenderDevice->SetVkObjectName((uint64_t)m_vkPipeline, VK_OBJECT_TYPE_PIPELINE, graphicsShader->GetDebugName());

	vkDestroyRenderPass(vulkanRenderDevice->GetVkDevice(), vkCompatibleRenderPass, nullptr);
}

CrGraphicsPipelineVulkan::~CrGraphicsPipelineVulkan()
{
	Deinitialize();
}

void CrGraphicsPipelineVulkan::Deinitialize()
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);

	vkDestroyPipeline(vulkanRenderDevice->GetVkDevice(), m_vkPipeline, nullptr);

	vkDestroyPipelineLayout(vulkanRenderDevice->GetVkDevice(), m_vkPipelineLayout, nullptr);
}

CrComputePipelineVulkan::CrComputePipelineVulkan(CrRenderDeviceVulkan* vulkanRenderDevice, const CrComputeShaderHandle& computeShader)
	: ICrComputePipeline(vulkanRenderDevice, computeShader)
{
	Initialize(vulkanRenderDevice, computeShader);
}

void CrComputePipelineVulkan::Initialize(CrRenderDeviceVulkan* vulkanRenderDevice, const CrComputeShaderHandle& computeShader)
{
	const CrComputeShaderVulkan* vulkanComputeShader = static_cast<const CrComputeShaderVulkan*>(computeShader.get());

	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStage.module = static_cast<const CrComputeShaderVulkan*>(computeShader.get())->GetVkShaderModule();
	shaderStage.pName = computeShader->GetBytecode()->GetEntryPoint().c_str();

	VkResult vkResult;

	VkDescriptorSetLayout descriptorSetLayouts[] = { vulkanComputeShader->GetVkDescriptorSetLayout() };

	// Pipeline Resource Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1; // TODO Handle when we have more than one layout
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	// TODO Push constants? Need to be part of the psoDescriptor?

	vkResult = vkCreatePipelineLayout(vulkanRenderDevice->GetVkDevice(), &pipelineLayoutCreateInfo, nullptr, &m_vkPipelineLayout);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create compute pipeline layout");

	VkComputePipelineCreateInfo computePipelineCreateInfo = {};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.stage = shaderStage;
	computePipelineCreateInfo.layout = m_vkPipelineLayout;

	vkResult = vkCreateComputePipelines(vulkanRenderDevice->GetVkDevice(), vulkanRenderDevice->GetVkPipelineCache(), 1, &computePipelineCreateInfo, nullptr, &m_vkPipeline);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create compute pipeline");

	vulkanRenderDevice->SetVkObjectName((uint64_t)m_vkPipeline, VK_OBJECT_TYPE_PIPELINE, computeShader->GetDebugName());
}

CrComputePipelineVulkan::~CrComputePipelineVulkan()
{
	Deinitialize();
}

void CrComputePipelineVulkan::Deinitialize()
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);

	vkDestroyPipeline(vulkanRenderDevice->GetVkDevice(), m_vkPipeline, nullptr);

	vkDestroyPipelineLayout(vulkanRenderDevice->GetVkDevice(), m_vkPipelineLayout, nullptr);
}

#if !defined(CR_CONFIG_FINAL)

void CrGraphicsPipelineVulkan::RecompilePS(ICrRenderDevice* renderDevice, const CrGraphicsShaderHandle& graphicsShader)
{
	Deinitialize();
	Initialize(static_cast<CrRenderDeviceVulkan*>(renderDevice), m_pipelineDescriptor, graphicsShader, m_vertexDescriptor);
}

void CrComputePipelineVulkan::RecompilePS(ICrRenderDevice* renderDevice, const CrComputeShaderHandle& computeShader)
{
	Deinitialize();
	Initialize(static_cast<CrRenderDeviceVulkan*>(renderDevice), computeShader);
}

#endif