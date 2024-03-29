#include "Rendering/CrRendering_pch.h"

#include "CrPIX.h"
#include "Core/CrMacros.h"

warnings_off
#define USE_PIX
#include "WinPixEventRuntime/pix3.h"
warnings_on

void CrPIX::Initialize(const CrRenderSystemDescriptor& /*renderSystemDescriptor*/)
{
	m_pixHandle = PIXLoadLatestWinPixGpuCapturerLibrary();
}

void CrPIX::StartCapture()
{
	PIXCaptureParameters captureParameters = {};
	PIXBeginCapture(PIX_CAPTURE_GPU, &captureParameters);
}

void CrPIX::EndCapture()
{
	PIXEndCapture(false);
}

bool CrPIX::IsFrameCapturing()
{
	return PIXGetCaptureState() != 0;
}

void CrPIX::SetDeviceAndWindow(void* /*device*/, void* window)
{
	PIXSetTargetWindow((HWND)window);
}
