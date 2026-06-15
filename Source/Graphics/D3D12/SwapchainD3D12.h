#pragma once

#include "Graphics/ISwapchain.h"

#include "d3d12.h"

namespace crgfx
{
	class SwapchainD3D12 final : public ISwapchain
	{
	public:

		SwapchainD3D12(crgfx::IDevice* renderDevice, const crgfx::CrSwapchainDescriptor& swapchainDescriptor);

		~SwapchainD3D12();

		virtual crgfx::CrSwapchainResult AcquireNextImagePS(uint64_t timeoutNanoseconds = UINT64_MAX) override;

		virtual void PresentPS() override;

		virtual void ResizePS(uint32_t width, uint32_t height) override;

	private:

		void CreateSwapchainTextures();

		ID3D12Fence* m_d3d12Fence;

		HANDLE m_fenceEvent;

		crstl::vector<UINT64> m_fenceValues;

		DXGI_SWAP_CHAIN_FLAG m_d3d12SwapchainFlags;

		IDXGISwapChain3* m_d3d12Swapchain;
	};
};