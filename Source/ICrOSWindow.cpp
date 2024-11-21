#include "ICrOSWindow.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_properties.h>

ICrOSWindow::ICrOSWindow(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Corsair Engine 0.01");
	//SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, x);
	//SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, y);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
	m_window = SDL_CreateWindowWithProperties(props);
	SDL_DestroyProperties(props);
}

ICrOSWindow::~ICrOSWindow()
{
	SDL_DestroyWindow(m_window);
}

bool ICrOSWindow::GetIsMinimized() const
{
	return (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED);
}

void* ICrOSWindow::GetNativeWindowHandle() const
{
	return SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
}
