#include "CrFrameTime.h"

#include "Core/String/CrString.h"
#include "Core/Logging/ICrDebug.h"

uint64_t CrFrameTime::m_frameCount = 0;
CrTime CrFrameTime::m_frameDelta;
CrTime CrFrameTime::m_framePreviousEndTime;

uint64_t CrFrameTime::m_lastUpdatedFrameCount;
CrTime CrFrameTime::m_lastUpdatedTime;

void CrFrameTime::IncrementFrameCount()
{
	CrTime currentTime = CrTime::Current();

	if ((currentTime - m_lastUpdatedTime).AsSeconds() > 1.0f)
	{
		uint64_t frames = m_frameCount - m_lastUpdatedFrameCount;

		CrString str = eastl::to_string(frames).c_str();

		CrLog("[FPS] %d [DELTA] %f ms", frames, m_frameDelta.AsMilliseconds());
		CrPrintProcessMemory("Frame Memory");

		m_lastUpdatedTime = currentTime;
		m_lastUpdatedFrameCount = m_frameCount;
	}

	m_frameDelta = currentTime - m_framePreviousEndTime;
	m_framePreviousEndTime = currentTime;
	m_frameCount = (m_frameCount + 1) % UINT64_MAX;
}

CrTime CrFrameTime::GetFrameDelta()
{
	return m_frameDelta;
}

uint64_t CrFrameTime::GetFrameCount()
{
	return m_frameCount;
}
