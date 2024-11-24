#include "Rendering/CrRendering_pch.h"

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
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(renderDevice);

	m_d3d12SwapchainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

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
	d3d12SwapchainDescriptor.Flags = m_d3d12SwapchainFlags; // Let it change modes on fullscreen to windowed

	//DXGI_SWAP_CHAIN_FULLSCREEN_DESC d3d12FullscreenDescriptor = {};

	IDXGISwapChain1* swapchain;

	HRESULT hResult = static_cast<const CrRenderSystemD3D12*>(ICrRenderSystem::Get())->GetDXGIFactory4()->CreateSwapChainForHwnd
	(
		d3d12RenderDevice->GetD3D12GraphicsCommandQueue(),
		(HWND)swapchainDescriptor.platformWindow,
		&d3d12SwapchainDescriptor,
		nullptr, // TODO Handle fullscreen
		nullptr,
		&swapchain
	);

	CrAssertMsg(SUCCEEDED(hResult) && swapchain != nullptr, "Swapchain creation failed");

	// Cast to an upgraded swapchain with more functionality
	swapchain->QueryInterface(IID_PPV_ARGS(&m_d3d12Swapchain));
	swapchain->Release();

	CrAssertMsg(SUCCEEDED(hResult), "Swapchain creation failed");

	m_width = swapchainDescriptor.requestedWidth;
	m_height = swapchainDescriptor.requestedHeight;
	m_imageCount = swapchainDescriptor.requestedBufferCount; // In the case of D3D12, requesting too many fails
	m_format = swapchainDescriptor.format;

	CreateSwapchainTextures();

	m_fenceValues.resize(m_imageCount);

	d3d12RenderDevice->GetD3D12Device()->CreateFence(m_fenceValues[m_currentBufferIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence));

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

CrSwapchainD3D12::~CrSwapchainD3D12()
{
	m_d3d12Swapchain->Release();

	m_d3d12Fence->Release();

	CloseHandle(m_fenceEvent);
}

CrSwapchainResult CrSwapchainD3D12::AcquireNextImagePS(uint64_t timeoutNanoseconds)
{
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(ICrRenderSystem::GetRenderDevice().get());
	ID3D12CommandQueue* commandQueue = d3d12RenderDevice->GetD3D12GraphicsCommandQueue();

	// Signal the fence
	const UINT64 currentFenceValue = m_fenceValues[m_currentBufferIndex];
	commandQueue->Signal(m_d3d12Fence, currentFenceValue);

	// Update the buffer index
	m_currentBufferIndex = m_d3d12Swapchain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_d3d12Fence->GetCompletedValue() < m_fenceValues[m_currentBufferIndex])
	{
		m_d3d12Fence->SetEventOnCompletion(m_fenceValues[m_currentBufferIndex], m_fenceEvent);
		WaitForSingleObjectEx(m_fenceEvent, (DWORD)timeoutNanoseconds, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_currentBufferIndex] = currentFenceValue + 1;

	return CrSwapchainResult::Success;
}

// https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-present
void CrSwapchainD3D12::PresentPS()
{
	UINT syncInternal = 1;
	UINT flags = 0;

	m_d3d12Swapchain->Present(syncInternal, flags);
}

void CrSwapchainD3D12::ResizePS(uint32_t width, uint32_t height)
{
	CrAssertMsg(m_d3d12Swapchain != nullptr, "Swapchain must have been previously created");

	m_width = width;
	m_height = height;

	// Assert that no one else has a reference to the swapchain texture. We cannot resize a swapchain with outstanding references to it
	// We also release the swapchain resource manually to avoid depending on the deletion queue to resize the swapchain
	for (uint32_t i = 0; i < m_imageCount; i++)
	{
		CrAssertMsg(m_textures[i]->get_ref() == 1, "Reference to resource being held externally");
		CrTextureD3D12* d3d12Texture = (CrTextureD3D12*)m_textures[i].get();
		d3d12Texture->GetD3D12Resource()->Release();
	}

	m_textures.clear();

	HRESULT hResult = m_d3d12Swapchain->ResizeBuffers(0, width, height, crd3d::GetDXGIFormat(m_format), m_d3d12SwapchainFlags);

	if (hResult == 0x887A0001)
	{
		CrAssertMsg(false, "Swapchain resources still in use");
	}

	CrAssertMsg(SUCCEEDED(hResult), "Failed to recreate swapchain");

	CreateSwapchainTextures();
}

void CrSwapchainD3D12::CreateSwapchainTextures()
{
	CrTextureDescriptor swapchainTextureParams;
	swapchainTextureParams.width = m_width;
	swapchainTextureParams.height = m_height;
	swapchainTextureParams.format = m_format;
	swapchainTextureParams.usage = cr3d::TextureUsage::SwapChain;

	m_textures.reserve(m_imageCount);

	for (uint32_t i = 0; i < m_imageCount; i++)
	{
		CrFixedString128 swapchainName(m_name);
		swapchainName.append_sprintf(" Texture %i", i);
		swapchainTextureParams.name = swapchainName.c_str();

		ID3D12Resource* surfaceResource;
		m_d3d12Swapchain->GetBuffer(i, IID_PPV_ARGS(&surfaceResource));
		swapchainTextureParams.extraDataPtr = surfaceResource; // Swapchain resource
		m_textures.push_back(m_renderDevice->CreateTexture(swapchainTextureParams));
	}
}
