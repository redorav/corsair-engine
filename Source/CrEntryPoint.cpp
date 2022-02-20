#include "Input/CrInputManager.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrFrame.h"

#include "Core/CrCommandLine.h"
#include "Core/Logging/ICrDebug.h"

#include "Core/CrGlobalPaths.h"

#include <windows.h> // TODO Remove

#include "ICrOSWindow.h"

// TODO SDL-specific
#include "Input/CrInputHandlerSDL.h"
#include <SDL.h>
#include <SDL_syswm.h>

uint32_t screenWidth = 1280;
uint32_t screenHeight = 720;

int main(int argc, char* argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
	{
		return 1;
	}

	crcore::CommandLine = CrCommandLineParser(argc, argv);

	const CrString& dataPath          = crcore::CommandLine("-root");
	const CrString& graphicsApiString = crcore::CommandLine("-graphicsapi");
	bool enableGraphicsValidation     = crcore::CommandLine["-debugGraphics"];
	bool enableRenderdoc              = crcore::CommandLine["-renderdoc"];

	CrString resolution = crcore::CommandLine("-resolution").c_str();
	if (!resolution.empty())
	{
		CrString::size_type pos = resolution.find('x');
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

	ICrOSWindow* mainWindow = new ICrOSWindow(screenWidth, screenHeight);

	void* hWnd = mainWindow->GetNativeWindowHandle();

	// HDC ourWindowHandleToDeviceContext = GetDC(hWnd);
	// Valid for the current executable (not valid for a dll)
	// http://stackoverflow.com/questions/21718027/getmodulehandlenull-vs-hinstance
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	CrPrintProcessMemory("Before Render Device");

	CrRenderSystemDescriptor renderSystemDescriptor;

	if (graphicsApiString == "vulkan")
	{
		renderSystemDescriptor.graphicsApi = cr3d::GraphicsApi::Vulkan;
	}
	else if (graphicsApiString == "d3d12")
	{
		renderSystemDescriptor.graphicsApi = cr3d::GraphicsApi::D3D12;
	}

	renderSystemDescriptor.enableValidation = enableGraphicsValidation;
	renderSystemDescriptor.enableDebuggingTool = enableRenderdoc;

	ICrRenderSystem::Initialize(renderSystemDescriptor);
	ICrRenderSystem::CreateRenderDevice();

	CrPrintProcessMemory("After Render Device");

	CrFrame frame;
	frame.Initialize(hInstance, hWnd, mainWindow->GetWidth(), mainWindow->GetHeight());

	CrInputHandlerSDL inputHandler;

	bool applicationRunning = true;

	while(applicationRunning)
	{
		CrInput.Update();

		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
				{
					applicationRunning = false;
					break;
				}
				case SDL_CONTROLLERDEVICEADDED:
				case SDL_CONTROLLERDEVICEREMOVED:
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEMOTION:
				case SDL_MOUSEWHEEL:
				case SDL_CONTROLLERAXISMOTION:
				case SDL_CONTROLLERBUTTONDOWN:
				case SDL_CONTROLLERBUTTONUP:
				{
					inputHandler.HandleEvent(event);
					break;
				}
				default:
					CrLog("Unhandled SDL message %i", event.type);
					break;
			}
		}

		if (!mainWindow->GetIsMinimized())
		{
			frame.Process(); // Process the main loop
		}
	}

	frame.Deinitialize();

	return 0;
}
