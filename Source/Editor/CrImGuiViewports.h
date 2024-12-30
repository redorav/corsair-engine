#pragma once

#include "Core/SmartPointers/CrIntrusivePtr.h"

class ICrOSWindow;

struct ImGuiViewportsData
{
	ICrOSWindow* osWindow;
	bool         windowOwned;

	ImGuiViewportsData()
	{
		osWindow = nullptr;
		windowOwned = false;
	}
};

// Imgui Viewports integration
class CrImGuiViewports
{
public:

	static void Initialize(const CrIntrusivePtr<ICrOSWindow>& mainWindow);
};