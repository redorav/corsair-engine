#include "CrImGuiViewports.h"

#include "ICrOSWindow.h"

#include "Core/Logging/ICrDebug.h"

#include "Rendering/CrRendererConfig.h"

#include <imgui.h>

#include <windows.h> // TODO Move into platform-specific file

struct ImGuiViewportsBackendData
{
	char* ClipboardTextData;
	bool WantUpdateMonitors;

	ImGuiViewportsBackendData() { memset((void*)this, 0, sizeof(*this)); }
};

static ImGuiViewportsBackendData* ImGuiViewportsGetBackendData()
{
	return ImGui::GetCurrentContext() ? (ImGuiViewportsBackendData*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

static void ImGuiViewportsSetClipboardText(ImGuiContext*, const char* text)
{
	unused_parameter(text);
}

static const char* ImGuiViewportsGetClipboardText(ImGuiContext*)
{
	return nullptr;
}

static void ImGuiViewportsPlatformSetImeData(ImGuiContext*, ImGuiViewport* viewport, ImGuiPlatformImeData* data)
{
	unused_parameter(viewport);
	unused_parameter(data);
}

// TODO Windows only
static BOOL CALLBACK ImGuiViewportsUpdateMonitors_EnumFunc(HMONITOR monitor, HDC, LPRECT, LPARAM)
{
	MONITORINFO info = {};
	info.cbSize = sizeof(MONITORINFO);

	if (!::GetMonitorInfo(monitor, &info))
	{
		return TRUE;
	}

	ImGuiPlatformMonitor imgui_monitor;
	imgui_monitor.MainPos  = ImVec2((float)info.rcMonitor.left, (float)info.rcMonitor.top);
	imgui_monitor.MainSize = ImVec2((float)(info.rcMonitor.right - info.rcMonitor.left), (float)(info.rcMonitor.bottom - info.rcMonitor.top));
	imgui_monitor.WorkPos  = ImVec2((float)info.rcWork.left, (float)info.rcWork.top);
	imgui_monitor.WorkSize = ImVec2((float)(info.rcWork.right - info.rcWork.left), (float)(info.rcWork.bottom - info.rcWork.top));
	imgui_monitor.DpiScale = 1.0f;// ImGui_ImplWin32_GetDpiScaleForMonitor(monitor);
	imgui_monitor.PlatformHandle = (void*)monitor;
	
	if (imgui_monitor.DpiScale <= 0.0f)
	{
		return TRUE; // Some accessibility applications are declaring virtual monitors with a DPI of 0, see #7902.
	}

	ImGuiPlatformIO& io = ImGui::GetPlatformIO();
	if (info.dwFlags & MONITORINFOF_PRIMARY)
	{
		io.Monitors.push_front(imgui_monitor);
	}
	else
	{
		io.Monitors.push_back(imgui_monitor);
	}

	return TRUE;
}

static void ImGuiViewportsUpdateMonitors()
{
	ImGuiViewportsBackendData* backendData = ImGuiViewportsGetBackendData();
	ImGui::GetPlatformIO().Monitors.resize(0);
	::EnumDisplayMonitors(nullptr, nullptr, ImGuiViewportsUpdateMonitors_EnumFunc, 0);
	backendData->WantUpdateMonitors = false;
}

static void ImGuiViewportsCreateWindow(ImGuiViewport* viewport)
{
	ImGuiViewportsData* viewportData = IM_NEW(ImGuiViewportsData)();
	viewport->PlatformUserData = viewportData;

	// (viewport->Flags & ImGuiViewportFlags_NoDecoration)
	// (viewport->Flags & ImGuiViewportFlags_NoDecoration)
	// (viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
	// (viewport->Flags & ImGuiViewportFlags_TopMost)

	CrOSWindowDescriptor windowDescriptor;
	windowDescriptor.topMost         = (viewport->Flags & ImGuiViewportFlags_TopMost) != 0;
	windowDescriptor.decoration      = (viewport->Flags & ImGuiViewportFlags_NoDecoration) == 0;
	windowDescriptor.name            = "No Title Yet";
	windowDescriptor.swapchainFormat = CrRendererConfig::SwapchainFormat;

	ICrOSWindow* osWindow = new ICrOSWindow(windowDescriptor);
	viewportData->osWindow = osWindow;
	viewportData->windowOwned = true;
}

static void ImGuiViewportsDestroyWindow(ImGuiViewport* viewport)
{
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;

	if (viewportData)
	{
		delete viewportData->osWindow;
		viewportData->osWindow = nullptr;
		IM_DELETE(viewportData);
	}

	viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void ImGuiViewportsShowWindow(ImGuiViewport* viewport)
{
	unused_parameter(viewport);
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;
	viewportData->osWindow->Show();
}

static void ImGuiViewportsUpdateWindow(ImGuiViewport* viewport)
{
	unused_parameter(viewport);
}

static ImVec2 ImGuiViewportsGetWindowPos(ImGuiViewport* viewport)
{
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;

	uint32_t positionX = 0, positionY = 0;
	viewportData->osWindow->GetPosition(positionX, positionY);

	return ImVec2((float)positionX, (float)positionY);
}

static void ImGuiViewportsSetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
{
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;
	viewportData->osWindow->SetPosition((uint32_t)pos.x, (uint32_t)pos.y);
}

static ImVec2 ImGuiViewportsGetWindowSize(ImGuiViewport* viewport)
{
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;

	uint32_t width, height;
	viewportData->osWindow->GetSizePixels(width, height);

	return ImVec2((float)width, (float)height);
}

static void ImGuiViewportsSetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;
	viewportData->osWindow->SetSizePixels((uint32_t)size.x, (uint32_t)size.y);
}

static void ImGuiViewportsSetWindowTitle(ImGuiViewport* viewport, const char* title)
{
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;
	viewportData->osWindow->SetTitle(title);
}

static void ImGuiViewportsSetWindowAlpha(ImGuiViewport* viewport, float alpha)
{
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;
	viewportData->osWindow->SetTransparencyAlpha(alpha); 
}

static void ImGuiViewportsSetWindowFocus(ImGuiViewport* viewport)
{
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;
	viewportData->osWindow->SetFocus();
}

static bool ImGuiViewportsGetWindowFocus(ImGuiViewport* viewport)
{
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;
	return viewportData->osWindow->GetIsFocused();
}

static bool ImGuiViewportsGetWindowMinimized(ImGuiViewport* viewport)
{
	ImGuiViewportsData* viewportData = (ImGuiViewportsData*)viewport->PlatformUserData;
	return viewportData->osWindow->GetIsMinimized();
}

void CrImGuiViewports::Initialize(const CrIntrusivePtr<ICrOSWindow>& mainWindow)
{
	unused_parameter(mainWindow);
	CrAssertMsg(ImGui::GetCurrentContext() != nullptr, "Imgui must have been created");

	ImGuiIO& io = ImGui::GetIO();

	ImGuiViewportsBackendData* bd = IM_NEW(ImGuiViewportsBackendData)();
	io.BackendPlatformUserData = (void*)bd;
	io.BackendPlatformName = "Corsair Engine Window Imgui";
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

	ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
	platformIO.Platform_SetClipboardTextFn = ImGuiViewportsSetClipboardText;
	platformIO.Platform_GetClipboardTextFn = ImGuiViewportsGetClipboardText;
	platformIO.Platform_SetImeDataFn = ImGuiViewportsPlatformSetImeData;

	ImGuiViewportsUpdateMonitors();

	if (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports)
	{
		// Register platform interface (will be coupled with a renderer interface)
		platformIO.Platform_CreateWindow       = ImGuiViewportsCreateWindow;
		platformIO.Platform_DestroyWindow      = ImGuiViewportsDestroyWindow;
		platformIO.Platform_ShowWindow         = ImGuiViewportsShowWindow;
		platformIO.Platform_UpdateWindow       = ImGuiViewportsUpdateWindow;
		platformIO.Platform_SetWindowPos       = ImGuiViewportsSetWindowPos;
		platformIO.Platform_GetWindowPos       = ImGuiViewportsGetWindowPos;
		platformIO.Platform_SetWindowSize      = ImGuiViewportsSetWindowSize;
		platformIO.Platform_GetWindowSize      = ImGuiViewportsGetWindowSize;
		platformIO.Platform_SetWindowFocus     = ImGuiViewportsSetWindowFocus;
		platformIO.Platform_GetWindowFocus     = ImGuiViewportsGetWindowFocus;
		platformIO.Platform_GetWindowMinimized = ImGuiViewportsGetWindowMinimized;
		platformIO.Platform_SetWindowTitle     = ImGuiViewportsSetWindowTitle;
		//platform_io.Platform_RenderWindow    = nullptr;
		//platform_io.Platform_SwapBuffers     = nullptr;
		platformIO.Platform_SetWindowAlpha     = ImGuiViewportsSetWindowAlpha;

		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImGuiViewportsData* mainViewportData = IM_NEW(ImGuiViewportsData)();
		mainViewportData->osWindow = mainWindow.get();
		mainViewportData->windowOwned = false;
		mainViewport->PlatformUserData = mainViewportData;
	}
}