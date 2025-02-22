#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrRenderingResources.h"
#include "Rendering/CrShaderSources.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/CrMaterialCompiler.h"
#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/CrRendererConfig.h"
#include "CrFrame.h"

#include "Core/Input/CrInputManager.h"
#include "Core/Input/CrPlatformInput.h"
#include "Core/CrCommandLine.h"
#include "Core/Logging/ICrDebug.h"

#include "Core/CrGlobalPaths.h"

#include <windows.h> // TODO Remove

#include "CrOSWindow.h"

uint32_t screenWidth = 1280;
uint32_t screenHeight = 720;

int main(int argc, char* argv[])
{
	crcore::CommandLine = CrCommandLineParser(argc, argv);

	const CrString& dataPath             = crcore::CommandLine("-root");
	const CrString& graphicsApiString    = crcore::CommandLine("-graphicsApi");
	const CrString& graphicsVendorString = crcore::CommandLine("-graphicsVendor");
	bool enableGraphicsValidation        = crcore::CommandLine["-debugGraphics"];
	bool enableRenderDoc                 = crcore::CommandLine["-renderdoc"];
	bool enablePIX                       = crcore::CommandLine["-pix"];
	bool waitForDebugger                 = crcore::CommandLine["-waitForDebugger"];

	if (waitForDebugger)
	{
		GetDebug()->WaitForDebugger();
	}

	CrString resolution = crcore::CommandLine("-resolution").c_str();
	if (!resolution.empty())
	{
		size_t pos = resolution.find('x');
		if(pos != CrString::npos)
		{
			screenWidth = atoi(resolution.substr(0, pos).c_str());
			screenHeight = atoi(resolution.substr(pos + 1).c_str());
			CrLog("Resolution set to %dx%d", screenWidth, screenHeight);
		} 
		else
		{
			CrLog("Resolution argument specified with an incorrect format! Expected format is <screenWidth>x<screenHeight>! Got \"%s\"", resolution.c_str());
		}
	}

	if (dataPath.empty())
	{
		CrAssertMsg(false, "No root on the command line");
	}

	CrGlobalPaths::SetupGlobalPaths(argv[0], dataPath.c_str());

	CrPrintProcessMemory("Before Render Device");

	CrRenderSystemDescriptor renderSystemDescriptor;
	renderSystemDescriptor.graphicsApi      = cr3d::GraphicsApi::FromString(graphicsApiString.c_str());

	// Default API is Vulkan
	if (renderSystemDescriptor.graphicsApi == cr3d::GraphicsApi::Count)
	{
		renderSystemDescriptor.graphicsApi = cr3d::GraphicsApi::Vulkan;
	}

	renderSystemDescriptor.enableValidation = enableGraphicsValidation;

	ICrRenderSystem::Initialize(renderSystemDescriptor);

	CrRenderDeviceDescriptor renderDeviceDescriptor;
	renderDeviceDescriptor.preferredVendor = cr3d::GraphicsVendor::FromString(graphicsVendorString.c_str());
	renderDeviceDescriptor.enableRenderDoc = enableRenderDoc;
	renderDeviceDescriptor.enablePIX       = enablePIX;
	RenderSystem->CreateRenderDevice(renderDeviceDescriptor);

	const CrRenderDeviceHandle& renderDevice = RenderSystem->GetRenderDevice();

	CrPrintProcessMemory("After Render Device");

	CrInputManager::Initialize();
	CrShaderSources::Initialize();
	CrShaderManager::Initialize(renderDevice.get());
	CrMaterialCompiler::Initialize();
	CrBuiltinPipelines::Initialize();
	CrRenderingResources::Initialize(renderDevice.get());
	CrOSWindow::Initialize();

	CrOSWindowDescriptor osWindowDescriptor;
	osWindowDescriptor.swapchainFormat = CrRendererConfig::SwapchainFormat;
	osWindowDescriptor.width = screenWidth;
	osWindowDescriptor.height = screenHeight;
	osWindowDescriptor.name = "Main Window";
	CrIntrusivePtr<CrOSWindow> mainWindow = CrIntrusivePtr<CrOSWindow>(new CrOSWindow(osWindowDescriptor));

	CrFrame frame;
	frame.Initialize(mainWindow);

	bool applicationRunning = true;

	while(applicationRunning)
	{
		CrInput.Update();

		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		
			switch (msg.message)
			{
				case WM_QUIT:
					applicationRunning = false;
					break;
			}
		}

		if (mainWindow->GetIsDestroyed())
		{
			applicationRunning = false;
		}

		if (applicationRunning)
		{
			if (!mainWindow->GetIsMinimized())
			{
				frame.Process();
			}
		}
	}

	frame.Deinitialize();

	CrRenderingResources::Deinitialize();

	return 0;
}
