#pragma once

struct CrRenderSystemDescriptor;

class CrPIX
{
public:

	void Initialize(const CrRenderSystemDescriptor& renderSystemDescriptor);

	void TriggerCapture();

	void StartCapture();

	void EndCapture();

	bool IsFrameCapturing();

	void SetDeviceAndWindow(void* device, void* window);

private:

	void* m_pixHandle;
};