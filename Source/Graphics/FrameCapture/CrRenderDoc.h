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

	void* m_renderDocModule = nullptr;

	void* m_currentDevice = nullptr;

	void* m_currentWindow = nullptr;

	RENDERDOC_API_1_5_0* m_renderDocApi = nullptr;
};