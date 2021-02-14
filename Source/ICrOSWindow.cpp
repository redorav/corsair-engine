#include "ICrOSWindow.h"

#include <SDL.h>
#include <SDL_syswm.h>

ICrOSWindow::ICrOSWindow(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	int windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

	m_window = SDL_CreateWindow("Corsair Engine 0.01", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, windowFlags);
}

ICrOSWindow::~ICrOSWindow()
{
	//Destroy window
	SDL_DestroyWindow(m_window);
}

bool ICrOSWindow::GetIsMinimized() const
{
	return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
}

void* ICrOSWindow::GetNativeWindowHandle() const
{
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(m_window, &info);
	HWND hwnd = info.info.win.window;
	return (void*)hwnd;
}
