#pragma once

#include "Rendering/ICrPipeline.h"

class CrGraphicsPipelineD3D12 final : public ICrGraphicsPipeline
{
public:

	ID3D12PipelineState* m_d3d12PipelineState;
};

class CrComputePipelineD3D12 final : public ICrComputePipeline
{
public:

	ID3D12PipelineState* m_d3d12PipelineState;
};