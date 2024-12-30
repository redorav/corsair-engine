#pragma once

#include "Core/SmartPointers/CrIntrusivePtr.h"

class CrOSWindow;

struct ImGuiViewportsData
{
	CrOSWindow* osWindow;
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

	static void Initialize(const CrIntrusivePtr<CrOSWindow>& mainWindow);
};