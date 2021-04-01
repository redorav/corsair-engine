#include "Input/CrInputManager.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrFrame.h"

#include "Core/CrCommandLine.h"
#include "Core/CrFrameTime.h"
#include "Core/Logging/ICrDebug.h"

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

	crcore::CommandLine.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

	CrString dataPath             = crcore::CommandLine("-root").str().c_str();
	CrString graphicsApiString    = crcore::CommandLine("-graphicsapi").str().c_str();
	bool enableGraphicsValidation = crcore::CommandLine["-debugGraphics"];
	bool enableRenderdoc          = crcore::CommandLine["-renderdoc"];

	if (dataPath.empty())
	{
		CrAssertMsg(false, "No root on the command line");
	}

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
	frame.Init(hInstance, hWnd, mainWindow->GetWidth(), mainWindow->GetHeight());

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

	return 0;
}
