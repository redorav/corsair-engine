#include "CrRendering_pch.h"

#include "CrRenderSystem_d3d12.h"
#include "CrSwapchain_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrGPUSynchronization_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"
#include "Core/CrMacros.h"

CrSwapchainD3D12::CrSwapchainD3D12(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor) : ICrSwapchain(renderDevice, swapchainDescriptor)
{
	CrRenderDeviceD3D12* d3d12Device = static_cast<CrRenderDeviceD3D12*>(renderDevice);

	DXGI_SWAP_CHAIN_DESC1 d3d12SwapchainDescriptor = {};
	d3d12SwapchainDescriptor.BufferCount = swapchainDescriptor.requestedBufferCount;
	d3d12SwapchainDescriptor.Format = crd3d::GetDXGIFormat(swapchainDescriptor.format);
	d3d12SwapchainDescriptor.Width = swapchainDescriptor.requestedWidth;
	d3d12SwapchainDescriptor.Height = swapchainDescriptor.requestedHeight;
	d3d12SwapchainDescriptor.SampleDesc.Count = 1;
	d3d12SwapchainDescriptor.SampleDesc.Quality = 0;
	d3d12SwapchainDescriptor.Stereo = false; // TODO

	// You don't need to pass DXGI_USAGE_BACK_BUFFER when you create a swap chain
	// Use the surface or resource as an output render target
	d3d12SwapchainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	d3d12SwapchainDescriptor.Scaling = DXGI_SCALING_NONE;
	d3d12SwapchainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // TODO Investigate
	d3d12SwapchainDescriptor.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	d3d12SwapchainDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // Let it change modes on fullscreen to windowed

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC d3d12FullscreenDescriptor = {};

	HRESULT hResult = static_cast<const CrRenderSystemD3D12*>(ICrRenderSystem::Get())->GetDXGIFactory()->CreateSwapChainForHwnd
	(
		d3d12Device->GetD3DDirectCommandQueue(),
		(HWND)swapchainDescriptor.platformWindow,
		&d3d12SwapchainDescriptor,
		nullptr, // TODO Handle fullscreen
		nullptr,
		&m_d3d12Swapchain
	);

	CrAssertMsg(SUCCEEDED(hResult), "Swapchain creation failed");

	m_width = swapchainDescriptor.requestedWidth;
	m_height = swapchainDescriptor.requestedHeight;
	m_imageCount = swapchainDescriptor.requestedBufferCount; // In the case of D3D12, requesting too many fails
	m_format = swapchainDescriptor.format;

	CrTextureDescriptor swapchainTextureParams;
	swapchainTextureParams.width = m_width;
	swapchainTextureParams.height = m_height;
	swapchainTextureParams.format = m_format;
	swapchainTextureParams.name = "Swapchain Texture";
	swapchainTextureParams.usage = cr3d::TextureUsage::SwapChain;

	for (uint32_t i = 0; i < m_imageCount; i++)
	{
		ID3D12Resource* surfaceResource;
		m_d3d12Swapchain->GetBuffer(i, IID_PPV_ARGS(&surfaceResource));
		swapchainTextureParams.extraDataPtr = surfaceResource; // Swapchain resource
		m_textures[i] = renderDevice->CreateTexture(swapchainTextureParams);
	}
}

CrSwapchainD3D12::~CrSwapchainD3D12()
{

}

CrSwapchainResult CrSwapchainD3D12::AcquireNextImagePS(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds)
{
	unused_parameter(signalSemaphore);
	unused_parameter(timeoutNanoseconds);
	return CrSwapchainResult::Success;
}

void CrSwapchainD3D12::PresentPS(ICrRenderDevice* renderDevice, const ICrGPUSemaphore* waitSemaphore)
{
	unused_parameter(renderDevice);
	unused_parameter(waitSemaphore);
}