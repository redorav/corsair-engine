#pragma once

struct CrRenderSystemDescriptor;

class CrPIX
{
public:

	bool Initialize();

	void TriggerCapture();

	void StartCapture();

	void EndCapture();

	bool IsFrameCapturing();

	void SetDeviceAndWindow(void* device, void* window);

private:

	void* m_pixHandle;
};