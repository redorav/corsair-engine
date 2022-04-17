#pragma once

struct CrRenderSystemDescriptor;
struct RENDERDOC_API_1_5_0;

class CrRenderDoc
{
public:

	void Initialize(const CrRenderSystemDescriptor& renderSystemDescriptor);

	void TriggerCapture();

	void StartCapture();

	void EndCapture();

	bool IsFrameCapturing();

	void SetDeviceAndWindow(void* device, void* window);

private:

	void* m_renderDocModule;

	void* m_currentDevice;

	void* m_currentWindow;

	RENDERDOC_API_1_5_0* m_renderDocApi;
};