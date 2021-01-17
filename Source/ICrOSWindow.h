#pragma once

#include "Math/CrMath.h"

struct SDL_Window;

class ICrOSWindow
{
public:

	static void Initialize();

	ICrOSWindow(uint32_t width, uint32_t height);

	~ICrOSWindow();

	bool GetIsMinimized() const;

	void* GetNativeWindowHandle() const;

private:

	uint32_t m_width = 0;

	uint32_t m_height = 0;

	SDL_Window* m_window = nullptr;
};