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

	uint32_t GetWidth() const;

	uint32_t GetHeight() const;

private:

	uint32_t m_width = 0;

	uint32_t m_height = 0;

	SDL_Window* m_window = nullptr;
};

inline uint32_t ICrOSWindow::GetWidth() const
{
	return m_width;
}

inline uint32_t ICrOSWindow::GetHeight() const
{
	return m_height;
}