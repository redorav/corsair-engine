#include "Rendering/CrRendering_pch.h"

#include "CrPIX.h"
#include "Core/CrMacros.h"

#include "Rendering/Extensions/CrPIXHeader.h"

bool CrPIX::Initialize()
{
	m_pixHandle = PIXLoadLatestWinPixGpuCapturerLibrary();
	return m_pixHandle != nullptr;
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
