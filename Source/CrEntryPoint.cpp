#include "Graphics/IGraphicsSystem.h"
#include "Graphics/IDevice.h"
#include "Graphics/CommonResources.h"
#include "Graphics/CrShaderSources.h"
#include "Graphics/CrShaderManager.h"
#include "Graphics/CrMaterialCompiler.h"
#include "Graphics/CrBuiltinPipelines.h"
#include "Graphics/CrRendererConfig.h"
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

	const crstl::string& dataPath             = crcore::CommandLine("-root");
	const crstl::string& graphicsApiString    = crcore::CommandLine("-graphicsApi");
	const crstl::string& graphicsVendorString = crcore::CommandLine("-graphicsVendor");
	bool enableGraphicsValidation        = crcore::CommandLine["-debugGraphics"];
	bool enableRenderDoc                 = crcore::CommandLine["-renderdoc"];
	bool enablePIX                       = crcore::CommandLine["-pix"];
	bool waitForDebugger                 = crcore::CommandLine["-waitForDebugger"];

	if (waitForDebugger)
	{
		GetDebug()->WaitForDebugger();
	}

	crstl::string resolution = crcore::CommandLine("-resolution").c_str();
	if (!resolution.empty())
	{
		size_t pos = resolution.find('x');
		if(pos != crstl::string::npos)
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

	crgfx::GraphicsSystemDescriptor graphicsSystemDescriptor;
	graphicsSystemDescriptor.graphicsApi      = crgfx::GraphicsApi::FromString(graphicsApiString.c_str());

	// Default API is Vulkan
	if (graphicsSystemDescriptor.graphicsApi == crgfx::GraphicsApi::Count)
	{
		graphicsSystemDescriptor.graphicsApi = crgfx::GraphicsApi::Vulkan;
	}

	graphicsSystemDescriptor.enableValidation = enableGraphicsValidation;
	graphicsSystemDescriptor.enableRenderDoc = enableRenderDoc;
	graphicsSystemDescriptor.enablePIX = enablePIX;

	crgfx::InitializeGraphicsSystem(graphicsSystemDescriptor);

	crgfx::DeviceDescriptor graphicsDeviceDescriptor;
	graphicsDeviceDescriptor.preferredVendor = crgfx::GraphicsVendor::FromString(graphicsVendorString.c_str());
	crgfx::CreateMainDevice(graphicsDeviceDescriptor);
	crgfx::InitializeCommonResources();

	CrPrintProcessMemory("After Graphics Device");

	CrInputManager::Initialize();
	CrShaderSources::Initialize();
	CrShaderManager::Initialize();
	CrMaterialCompiler::Initialize();
	CrBuiltinPipelines::Initialize();
	CrOSWindow::Initialize();

	CrOSWindowDescriptor osWindowDescriptor;
	osWindowDescriptor.swapchainFormat = CrRendererConfig::SwapchainFormat;
	osWindowDescriptor.width = screenWidth;
	osWindowDescriptor.height = screenHeight;
	osWindowDescriptor.name = "Main Window";
	crstl::intrusive_ptr<CrOSWindow> mainWindow = crstl::intrusive_ptr<CrOSWindow>(new CrOSWindow(osWindowDescriptor));

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

	crgfx::DeinitializeCommonResources();

	return 0;
}
