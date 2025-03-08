#pragma once

#include "crstl/intrusive_ptr.h"

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

	static void Initialize(const crstl::intrusive_ptr<CrOSWindow>& mainWindow);
};