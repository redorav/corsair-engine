#pragma once

#include "Rendering/ICrSwapchain.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrDataFormats.h"

#include "Math/CrMath.h"

#include "crstl/intrusive_ptr.h"

struct CrOSWindowDescriptor
{
	CrOSWindowDescriptor()
		: positionX(0)
		, positionY(0)
		, width(1)
		, height(1)
		, parentWindow(nullptr)
		, swapchainFormat(cr3d::DataFormat::Invalid)
		, decoration(true)
		, topMost(false)
	{

	}

	uint32_t positionX;
	uint32_t positionY;
	uint32_t width;
	uint32_t height;
	CrOSWindow* parentWindow;
	crstl::fixed_string128 name;
	cr3d::DataFormat::T swapchainFormat;
	uint32_t decoration : 1;
	uint32_t topMost : 1;
};

namespace CursorType
{
	enum T
	{
		Arrow,
		TextInput,
		ResizeAll,
		ResizeEW,
		ResizeNS,
		ResizeNESW,
		ResizeNWSE,
		Hand,
		NotAllowed,
		None,
		Count
	};
}

class CrOSWindow : public crstl::intrusive_ptr_interface_delete
{
public:

	CrOSWindow(const CrOSWindowDescriptor& windowDescriptor);

	~CrOSWindow();

	// Initialize the windowing system
	static void Initialize();

	static void SetCursor(CursorType::T cursorType);
	
	// Destroy the window. A window can be destroyed externally while the object or pointer is still active. For that reason we
	// can query whether a window has been destroyed to avoid using its resources in an invalid state. A destroyed window cannot
	// be recreated, a new one must be built
	void Destroy();

	bool GetIsMinimized() const;

	bool GetIsFocused() const;

	bool GetIsDestroyed() const;

	void* GetNativeWindowHandle() const;

	void* GetParentWindowHandle() const;

	void* GetNativeInstanceHandle() const;

	void GetPosition(uint32_t& positionX, uint32_t& positionY) const;

	void GetSizePixels(uint32_t& width, uint32_t& height) const;

	void SetPosition(uint32_t positionX, uint32_t positionY);

	void SetSizePixels(uint32_t width, uint32_t height);

	void SetFocus();

	void SetTitle(const char* title);

	void SetTransparencyAlpha(float alpha);

	void Show();

	const CrSwapchainHandle& GetSwapchain() { return m_swapchain; }

private:

	void* m_hwnd = nullptr;

	void* m_parentHwnd = nullptr;

	void* m_hInstance = nullptr;

	uint64_t m_style = 0;

	uint64_t m_exStyle = 0;

	bool m_resized = false;

	CrSwapchainHandle m_swapchain;
};