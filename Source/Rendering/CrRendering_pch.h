#pragma once

#include "CrRendering.h"

#if defined(VULKAN_API)
// Vulkan on Windows includes windows.h which defines things like min and max, aside from being very big.
// To counter it we define NOMINMAX, etc. globally
#include <vulkan/vulkan.h>
#include "vulkan/CrVulkan.h"
#endif

#if defined(D3D12_API)
#include <d3d12.h>
#include <dxgi1_4.h>
//#include <d3dx12.h>
#endif