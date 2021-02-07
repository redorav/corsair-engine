#include "CrInputManager.h"

#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrFrame.h"

#include "Core/CrCommandLine.h"
#include "Core/CrFrameTime.h"
#include "Core/Logging/ICrDebug.h"

#include <windows.h> // TODO Remove

#include "ICrOSWindow.h"

#include <SDL.h>
#include <SDL_syswm.h>

bool g_appWasClosed = false; // TODO This global needs to go

uint32_t screenWidth = 1280;
uint32_t screenHeight = 720;

int main(int argc, char* argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		return 1;
	}
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

	crcore::CommandLine.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

	CrString dataPath             = crcore::CommandLine("-root").str().c_str();
	CrString graphicsApiString    = crcore::CommandLine("-graphicsapi").str().c_str();
	bool enableGraphicsValidation = crcore::CommandLine["-debugGraphics"];
	bool enableRenderdoc          = crcore::CommandLine["-renderdoc"];

	if (dataPath.empty())
	{
		CrAssertMsg(false, "No root on the command line");
	}

	ICrOSWindow* mainWindow = new ICrOSWindow(1280, 720);

	void* hWnd = mainWindow->GetNativeWindowHandle();

	//HDC ourWindowHandleToDeviceContext = GetDC(hWnd);
	HINSTANCE hInstance = GetModuleHandle(nullptr); // Valid for the current executable (not valid for a dll) http://stackoverflow.com/questions/21718027/getmodulehandlenull-vs-hinstance

	CrPrintProcessMemory("Before Render Device");

	CrRenderDeviceDescriptor renderDeviceDescriptor;

	if (graphicsApiString == "vulkan")
	{
		renderDeviceDescriptor.graphicsApi = cr3d::GraphicsApi::Vulkan;
	}
	else if (graphicsApiString == "d3d12")
	{
		renderDeviceDescriptor.graphicsApi = cr3d::GraphicsApi::D3D12;
	}

	renderDeviceDescriptor.enableValidation = enableGraphicsValidation;
	renderDeviceDescriptor.enableDebuggingTool = enableRenderdoc;

	ICrRenderDevice::Create(renderDeviceDescriptor);

	CrPrintProcessMemory("After Render Device");

	CrFrame frame;
	frame.Init(hInstance, hWnd, screenWidth, screenHeight);

	while(!g_appWasClosed)
	{
		CrInput.Update();

		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
				{
					exit(0);
					break;
				}
				case SDL_SYSWMEVENT:
				{
					SDL_SysWMmsg wMsg = *event.syswm.msg;
					MSG msg = {};
					msg.hwnd = wMsg.msg.win.hwnd;
					msg.message = wMsg.msg.win.msg;
					msg.wParam = wMsg.msg.win.wParam;
					msg.lParam = wMsg.msg.win.lParam;
					CrInput.HandleMessage((void*)&msg);
					break;
				}
				default:
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
