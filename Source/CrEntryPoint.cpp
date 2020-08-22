#include "CrGUI.h"
#include "CrMain.h"

#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrFrame.h"

#include "Core/CrCommandLine.h"
#include "Core/CrFrameTime.h"
#include "Core/Logging/ICrDebug.h"

#include <windows.h> // TODO Remove

bool g_appWasClosed = false; // TODO This global needs to go

int main(int argc, char* argv[])
{
	crcore::CommandLine.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

	CrGUI* crGUI = new CrGUI(argc, argv);

	CrString dataPath = crcore::CommandLine("-root").str().c_str();

	if (dataPath.empty())
	{
		CrAssertMsg(false, "No root on the command line");
	}

	HWND hWnd = crGUI->GetMainWindowHandle();
	//HDC ourWindowHandleToDeviceContext = GetDC(hWnd);
	HINSTANCE hInstance = GetModuleHandle(nullptr); // Valid for the current executable (not valid for a dll) http://stackoverflow.com/questions/21718027/getmodulehandlenull-vs-hinstance

	ICrRenderDevice::Create(cr3d::GraphicsApi::Vulkan); // Need to make a window class here that abstracts these Windows-specific things

	CrFrame frame;
	frame.Init(hInstance, hWnd, 1280, 720);

	while(!g_appWasClosed)
	{
		CrInput.Update();

		QApplication::processEvents();

		// Process the main loop
		frame.Process();

		CrFrameTime::IncrementFrameCount();
	}

	return 0;
}
