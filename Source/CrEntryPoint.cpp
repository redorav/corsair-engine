#include "CrInputManager.h"

#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrFrame.h"

#include "Core/CrCommandLine.h"
#include "Core/CrFrameTime.h"
#include "Core/Logging/ICrDebug.h"

#include <windows.h> // TODO Remove

#include "ICrOSWindow.h"

#include <SDL.h>

bool g_appWasClosed = false; // TODO This global needs to go

uint32_t screenWidth = 1280;
uint32_t screenHeight = 720;

int main(int argc, char* argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		return 1;
	}

	crcore::CommandLine.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

	CrString dataPath = crcore::CommandLine("-root").str().c_str();

	if (dataPath.empty())
	{
		CrAssertMsg(false, "No root on the command line");
	}

	ICrOSWindow* mainWindow = new ICrOSWindow(1280, 720);

	void* hWnd = mainWindow->GetNativeWindowHandle();

	//HDC ourWindowHandleToDeviceContext = GetDC(hWnd);
	HINSTANCE hInstance = GetModuleHandle(nullptr); // Valid for the current executable (not valid for a dll) http://stackoverflow.com/questions/21718027/getmodulehandlenull-vs-hinstance

	CrPrintProcessMemory("Before Render Device");

	ICrRenderDevice::Create(cr3d::GraphicsApi::Vulkan); // Need to make a window class here that abstracts these Windows-specific things

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
					exit(0);
					break;

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
