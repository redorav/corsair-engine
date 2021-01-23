#include "CrRendering_pch.h"

#include "CrPipelineStateManager_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrRenderPass_vk.h"
#include "CrShader_vk.h"
#include "Rendering/ICrShaderManager.h" // TODO remove
#include "CrVulkan.h"
#include "Rendering/CrGPUBuffer.h"

#include "Core/Containers/CrArray.h"

#include "Core/Logging/ICrDebug.h"

ICrGraphicsPipeline* CrPipelineStateManagerVulkan::CreateGraphicsPipelinePS
(
	const CrGraphicsPipelineDescriptor& psoDescriptor, 
	const ICrGraphicsShader* graphicsShader, 
	const CrVertexDescriptor& vertexDescriptor,
	const CrRenderPassDescriptor& renderPassDescriptor
)
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);
	CrGraphicsPipelineVulkan* vulkanGraphicsPipeline = new CrGraphicsPipelineVulkan();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.pNext = nullptr;
	inputAssemblyState.flags = 0;
	inputAssemblyState.topology = crvk::GetVkPrimitiveTopology(psoDescriptor.primitiveTopology);
	inputAssemblyState.primitiveRestartEnable = false;
	
	VkPipelineRasterizationStateCreateInfo rasterizerState = {};
	rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerState.pNext = nullptr;
	rasterizerState.flags = 0;
	rasterizerState.depthClampEnable = false;
	rasterizerState.polygonMode = crvk::GetVkPolygonFillMode(psoDescriptor.rasterizerState.fillMode);
	rasterizerState.cullMode = crvk::GetVkPolygonCullMode(psoDescriptor.rasterizerState.cullMode);
	rasterizerState.frontFace = crvk::GetVkFrontFace(psoDescriptor.rasterizerState.frontFace);
	
	// TODO Complete this section
	rasterizerState.depthClampEnable = psoDescriptor.rasterizerState.depthClipEnable;
	rasterizerState.lineWidth = 1.0f;
	//rasterizerState.rasterizerDiscardEnable = false;
	//rasterizerState.depthBiasEnable = false;
	
	VkPipelineColorBlendStateCreateInfo colorBlendState;
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.pNext = nullptr;
	colorBlendState.flags = 0;
	colorBlendState.logicOpEnable = false;
	colorBlendState.logicOp = VK_LOGIC_OP_NO_OP;
	
	uint32_t numRenderTargets = psoDescriptor.blendState.numRenderTargets;
	
	CrVector<VkPipelineColorBlendAttachmentState> blendAttachments(numRenderTargets);
	for (uint32_t i = 0; i < numRenderTargets; ++i)
	{
		const CrRenderTargetBlendDescriptor& renderTargetBlend = psoDescriptor.blendState.renderTargetBlends[i];
		blendAttachments[i].colorWriteMask = renderTargetBlend.colorWriteMask; // TODO Careful with this, needs a platform-specific translation
		blendAttachments[i].blendEnable = renderTargetBlend.enable;
		if (renderTargetBlend.enable)
		{
			blendAttachments[i].colorBlendOp		= crvk::GetVkBlendOp(renderTargetBlend.colorBlendOp);
			blendAttachments[i].dstColorBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.dstColorBlendFactor);
			blendAttachments[i].srcColorBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.srcColorBlendFactor);
	
			blendAttachments[i].alphaBlendOp		= crvk::GetVkBlendOp(renderTargetBlend.alphaBlendOp);
			blendAttachments[i].dstAlphaBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.dstAlphaBlendFactor);
			blendAttachments[i].srcAlphaBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.srcAlphaBlendFactor);
		}
	}
	
	colorBlendState.attachmentCount = numRenderTargets;
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
	multisampleState.rasterizationSamples = crvk::GetVkSampleCount(psoDescriptor.multisampleState.sampleCount);
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
	depthStencilState.depthTestEnable		= psoDescriptor.depthStencilState.depthTestEnable;
	depthStencilState.depthWriteEnable		= psoDescriptor.depthStencilState.depthWriteEnable;
	depthStencilState.depthCompareOp		= crvk::GetVkCompareOp(psoDescriptor.depthStencilState.depthCompareOp);
	depthStencilState.depthBoundsTestEnable	= psoDescriptor.depthStencilState.depthBoundsTestEnable;
	
	// Stencil
	depthStencilState.stencilTestEnable = psoDescriptor.depthStencilState.stencilTestEnable;
	
	depthStencilState.front.depthFailOp	= crvk::GetVkStencilOp(psoDescriptor.depthStencilState.front.depthFailOp);
	depthStencilState.front.failOp		= crvk::GetVkStencilOp(psoDescriptor.depthStencilState.front.stencilFailOp);
	depthStencilState.front.passOp		= crvk::GetVkStencilOp(psoDescriptor.depthStencilState.front.stencilPassOp);
	depthStencilState.front.compareOp	= crvk::GetVkCompareOp(psoDescriptor.depthStencilState.front.stencilCompareOp);
	depthStencilState.front.compareMask	= psoDescriptor.depthStencilState.front.stencilReadMask;
	depthStencilState.front.writeMask	= psoDescriptor.depthStencilState.front.stencilWriteMask;
	depthStencilState.front.reference	= psoDescriptor.depthStencilState.front.reference;
	
	depthStencilState.back.depthFailOp	= crvk::GetVkStencilOp(psoDescriptor.depthStencilState.back.depthFailOp);
	depthStencilState.back.failOp		= crvk::GetVkStencilOp(psoDescriptor.depthStencilState.back.stencilFailOp);
	depthStencilState.back.passOp		= crvk::GetVkStencilOp(psoDescriptor.depthStencilState.back.stencilPassOp);
	depthStencilState.back.compareOp	= crvk::GetVkCompareOp(psoDescriptor.depthStencilState.back.stencilCompareOp);
	depthStencilState.back.compareMask	= psoDescriptor.depthStencilState.back.stencilReadMask;
	depthStencilState.back.writeMask	= psoDescriptor.depthStencilState.back.stencilWriteMask;
	depthStencilState.back.reference	= psoDescriptor.depthStencilState.back.reference;
	
	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;
	
	// Viewport state
	VkPipelineViewportStateCreateInfo viewportState;
	viewportState.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext			= nullptr;
	viewportState.flags			= 0;
	viewportState.viewportCount	= 1; // One viewport
	viewportState.pViewports	= nullptr;
	viewportState.scissorCount	= 1; // One scissor rectangle
	viewportState.pScissors		= nullptr;
	
	// Dynamic states can be set even after the pipeline has been created, so there is no need to create new pipelines
	// just for changing a viewport's dimensions or a scissor box
	// The dynamic state properties themselves are stored in the command buffer
	CrArray<VkDynamicState, 2> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	
	VkPipelineDynamicStateCreateInfo dynamicState;
	dynamicState.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext				= nullptr;
	dynamicState.flags				= 0;
	dynamicState.pDynamicStates		= dynamicStateEnables.data();
	dynamicState.dynamicStateCount	= (uint32_t)dynamicStateEnables.size();
	
	VkResult vkResult;
	
	// Shader information
	VkPipelineShaderStageCreateInfo shaderStages[cr3d::ShaderStage::GraphicsStageCount] = {};
	VkPipelineShaderStageCreateInfo shaderStageInfo = {};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

	const CrGraphicsShaderVulkan* vulkanGraphicsShader = static_cast<const CrGraphicsShaderVulkan*>(graphicsShader);

	const CrVector<VkShaderModule>& vkShaderModules = vulkanGraphicsShader->GetVkShaderModules();

	const CrVector<CrShaderStageInfo>& stageInfos = vulkanGraphicsShader->GetStages();

	uint32_t usedShaderStages = 0;

	for (uint32_t i = 0; i < stageInfos.size(); ++i)
	{
		const CrShaderStageInfo& stageInfo = stageInfos[i];
		const VkShaderModule vkShaderModule = vkShaderModules[i];

		shaderStageInfo.module = vkShaderModule;
		shaderStageInfo.stage = crvk::GetVkShaderStage(stageInfo.stage);
		shaderStageInfo.pName = stageInfo.entryPoint.c_str();
		shaderStages[usedShaderStages++] = shaderStageInfo;
	}
	
	const CrShaderResourceTableVulkan& resourceTable = static_cast<const CrShaderResourceTableVulkan&>(graphicsShader->GetResourceTable());

	// Pipeline Resource Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1; // TODO Don't know what cases would warrant making this > 1
	pipelineLayoutCreateInfo.pSetLayouts = &resourceTable.m_vkDescriptorSetLayout; // From shader
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	// TODO Push constants? Need to be part of the psoDescriptor?
	
	vkResult = vkCreatePipelineLayout(vulkanRenderDevice->GetVkDevice(), &pipelineLayoutCreateInfo, nullptr, &vulkanGraphicsPipeline->m_vkPipelineLayout);

	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create pipeline layout");

	// Binding description
	CrVector<VkVertexInputBindingDescription> bindingDescriptions;
	bindingDescriptions.resize(1);
	bindingDescriptions[0].binding = 0; // TODO Shader binding location
	bindingDescriptions[0].stride = vertexDescriptor.GetDataSize();
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	
	uint32_t offset = 0;
	
	// Create vertex input state
	CrVector<VkVertexInputAttributeDescription> attributeDescriptions;
	
	attributeDescriptions.resize(vertexDescriptor.GetNumAttributes());
	
	for (uint32_t i = 0; i < vertexDescriptor.GetNumAttributes(); ++i)
	{
		const cr3d::DataFormatInfo& vertexFormatInfo = vertexDescriptor.GetVertexInfo(i);
		attributeDescriptions[i].binding = 0; // TODO Shader binding location
		attributeDescriptions[i].location = i;
		attributeDescriptions[i].format = crvk::GetVkFormat(vertexFormatInfo.format);
		attributeDescriptions[i].offset = offset;
		offset += vertexFormatInfo.dataOrBlockSize;
	}
	
	// Assign to vertex buffer
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.pNext = nullptr;
	vertexInputState.flags = 0;
	vertexInputState.vertexBindingDescriptionCount		= (uint32_t)bindingDescriptions.size();
	vertexInputState.pVertexBindingDescriptions			= bindingDescriptions.data();
	vertexInputState.vertexAttributeDescriptionCount	= (uint32_t)attributeDescriptions.size();
	vertexInputState.pVertexAttributeDescriptions		= attributeDescriptions.data();
	
	// We don't need the real renderpass here to create pipeline state objects, a compatible one is enough
	// to make it work. I suspect the only thing it really needs is the description of how we're going to
	// be using the render pass. In D3D12 this is actually part of the psoDescriptor (RTVFormats, DSVFormat)
	// TODO This should probably be optimized either by having another function that passes an actual render pass
	// or by having a cache of render pass descriptors. The reason for using a dummy render pass is so that
	// pipelines can be precreated without needing access to the actual draw commands.
	CrRenderPassVulkan dummyRenderPass = CrRenderPassVulkan(m_renderDevice, renderPassDescriptor);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount				= (uint32_t) graphicsShader->GetStages().size();
	pipelineInfo.pStages				= shaderStages;
	
	pipelineInfo.layout					= vulkanGraphicsPipeline->m_vkPipelineLayout; // TODO Hack
	pipelineInfo.pVertexInputState		= &vertexInputState;				// TODO Create this pipeline layout first from the shader/vertex descriptor
	pipelineInfo.renderPass				= dummyRenderPass.GetVkRenderPass();
	
	pipelineInfo.pInputAssemblyState	= &inputAssemblyState;
	pipelineInfo.pRasterizationState	= &rasterizerState;
	pipelineInfo.pColorBlendState		= &colorBlendState;
	pipelineInfo.pMultisampleState		= &multisampleState;
	pipelineInfo.pViewportState			= &viewportState;
	pipelineInfo.pDepthStencilState		= &depthStencilState;
	pipelineInfo.pDynamicState			= &dynamicState;

	vkResult = vkCreateGraphicsPipelines
	(vulkanRenderDevice->GetVkDevice(), vulkanRenderDevice->GetVkPipelineCache(), 1, &pipelineInfo, nullptr, &vulkanGraphicsPipeline->m_vkPipeline);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create graphics pipeline");

	return vulkanGraphicsPipeline;
}
