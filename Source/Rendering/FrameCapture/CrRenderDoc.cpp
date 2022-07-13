#include "CrRendering_pch.h"

#include "CrRenderDoc.h"
#include "renderdoc_app.h"

#include "Rendering/ICrRenderSystem.h"

#include "Core/Logging/ICrDebug.h"
#include "Core/FileSystem/CrPath.h"

#if defined(WINDOWS_PLATFORM)
#include <shellapi.h>
#endif

void CrRenderDoc::Initialize(const CrRenderSystemDescriptor& renderSystemDescriptor)
{
#if defined(WINDOWS_PLATFORM)

	// Try to load it directly if it's been loaded already. This is the safest option since it means the program
	// has been launched from the renderdoc interface, and that is the dll we want to use
	m_renderDocModule = GetModuleHandleA("renderdoc.dll");

	if (!m_renderDocModule)
	{
		CrPath renderdocDllPath;

		// On Vulkan, try to locate the dll that has been registered by the layers. If we don't manually load this
		// dll and load a different one, when Vulkan initializes it will load both and capture replay will fail.
		// Always try to load this one first
		if (renderSystemDescriptor.graphicsApi == cr3d::GraphicsApi::Vulkan)
		{
			HKEY renderdocLayerKey = 0;

			if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Khronos\\Vulkan\\ImplicitLayers\\", 0, KEY_READ, &renderdocLayerKey) == ERROR_SUCCESS)
			{
				int valueCount = 0;
				const DWORD StringSize = 512;
				DWORD stringSize = StringSize;
				char stringData[StringSize] = {};
				while (RegEnumValueA(renderdocLayerKey, valueCount, stringData, &stringSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
				{
					if (strstr(stringData, "renderdoc") != nullptr)
					{
						renderdocDllPath = stringData;
						renderdocDllPath = renderdocDllPath.parent_path();
						renderdocDllPath /= "renderdoc.dll";
						break;
					}

					stringSize = StringSize;
					valueCount++;
				}
			}
		}

		// For all other APIs or if the Vulkan layer isn't registered, try to load the one from the installation folder
		// This path in Vulkan won't conflict if it doesn't try to load any layer
		if (renderdocDllPath.empty())
		{
			HKEY renderdocIconKey = 0;
			if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Classes\\RenderDoc.RDCCapture.1\\DefaultIcon\\", 0, KEY_READ, &renderdocIconKey) == ERROR_SUCCESS)
			{
				const DWORD StringSize = 512;
				DWORD stringSize = StringSize;
				char stringData[StringSize] = {};
				LSTATUS valueStatus = RegGetValueA(renderdocIconKey, nullptr, nullptr, RRF_RT_ANY, nullptr, &stringData, &stringSize);

				if (valueStatus == ERROR_SUCCESS)
				{
					renderdocDllPath = stringData;
					renderdocDllPath = renderdocDllPath.parent_path();
					renderdocDllPath /= "renderdoc.dll";
				}

				RegCloseKey(renderdocIconKey);
			}
		}

		// If we managed to find a path to the renderdoc dll, load it
		if (!renderdocDllPath.empty())
		{
			m_renderDocModule = LoadLibraryA(renderdocDllPath.c_str());
		}
	}

#endif

	if (m_renderDocModule)
	{
		pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress((HMODULE)m_renderDocModule, "RENDERDOC_GetAPI");
		RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_5_0, (void**)&m_renderDocApi);

		RENDERDOC_InputButton captureButtons[] = { eRENDERDOC_Key_F10, eRENDERDOC_Key_F12 };

		m_renderDocApi->SetCaptureKeys(captureButtons, sizeof_array(captureButtons));

		if (renderSystemDescriptor.enableValidation)
		{
			m_renderDocApi->SetCaptureOptionU32(eRENDERDOC_Option_APIValidation, 1);
		}
	}
}

void CrRenderDoc::TriggerCapture()
{
	if (m_renderDocApi)
	{
		m_renderDocApi->TriggerCapture();
	}
}

void CrRenderDoc::StartCapture()
{
	if (m_renderDocApi)
	{
		m_renderDocApi->StartFrameCapture(m_currentDevice, m_currentWindow);
	}
}

void CrRenderDoc::EndCapture()
{
	if (m_renderDocApi)
	{
		m_renderDocApi->EndFrameCapture(m_currentDevice, m_currentWindow);
	}
}

bool CrRenderDoc::IsFrameCapturing()
{
	if (m_renderDocApi)
	{
		return m_renderDocApi->IsFrameCapturing();
	}

	return false;
}

void CrRenderDoc::SetDeviceAndWindow(void* device, void* window)
{
	m_currentDevice = device;
	m_currentWindow = window;
}
