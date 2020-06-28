#include "CrGUI.h"
#include "CrMain.h"

#include "Rendering/ICrRenderDevice.h"

#include "Core/CrCommandLine.h"
#include "Core/CrTime.h"
#include "Core/Logging/ICrDebug.h"

#include <windows.h> // TODO Remove

bool g_appWasClosed = false; // TODO This global needs to go

// TODO Put in a better place. Here for the sake of EASTL for now
void* operator new[](size_t size, const char* /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
	return malloc(size);
}

void* operator new[](size_t size, size_t /*alignment*/, size_t /*alignmentOffset*/, const char* /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
	return malloc(size);
}

int main(int argc, char* argv[])
{
	crcore::CommandLine.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

	CrGUI* crGUI = new CrGUI(argc, argv);

	CrString dataPath = crcore::CommandLine("-root").str().c_str();

	if (dataPath.empty())
	{
		CrAssertMsg(false, "No root on the command line!");
	}

	HWND hWnd = crGUI->GetMainWindowHandle();
	//HDC ourWindowHandleToDeviceContext = GetDC(hWnd);
	HINSTANCE hInstance = GetModuleHandle(nullptr); // Valid for the current executable (not valid for a dll) http://stackoverflow.com/questions/21718027/getmodulehandlenull-vs-hinstance

	ICrRenderDevice::Create(hInstance, hWnd, cr3d::GraphicsApi::Vulkan); // Need to make a window class here that abstracts these Windows-specific things
	ICrRenderDevice* renderDevice = (ICrRenderDevice*)ICrRenderDevice::GetRenderDevice();

	while(!g_appWasClosed)
	{
		//float startTime;
		CrInput.Update();

		QApplication::processEvents();

		// TODO Create proper frame

		renderDevice->Present();

		CrTime::IncrementFrameCount();

		//ProcessContinuousInput();

		// TODO: Only process input if the main application is focused!

		//Render(ourWindowHandleToDeviceContext);
	}

	return 0;
}
