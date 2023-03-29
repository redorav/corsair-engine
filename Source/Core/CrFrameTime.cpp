#include "Core/CrCore_pch.h"

#include "CrFrameTime.h"

#include "Core/String/CrString.h"
#include "Core/Logging/ICrDebug.h"

uint64_t CrFrameTime::m_frameCount = 0;
CrTime CrFrameTime::m_frameDelta;
CrTime CrFrameTime::m_framePreviousEndTime;

uint64_t CrFrameTime::m_lastUpdatedFrameCount;
CrTime CrFrameTime::m_lastUpdatedTime;
CrTime CrFrameTime::m_frameDeltaAverage;
CrTime CrFrameTime::m_frameDeltaMin;
CrTime CrFrameTime::m_frameDeltaMax;

uint32_t			CrFrameTime::m_deltaHistoryIndex;
CrArray<CrTime, 30> CrFrameTime::m_frameDeltaHistory;

void CrFrameTime::IncrementFrameCount()
{
	CrTime currentTime = CrTime::Current();

	if ((currentTime - m_lastUpdatedTime).AsSeconds() > 1.0f)
	{
		uint64_t frames = m_frameCount - m_lastUpdatedFrameCount;

		CrLog("[FPS] %d [DELTA] %f ms", frames, m_frameDelta.AsMilliseconds());
		CrPrintProcessMemory("Frame Memory");

		m_lastUpdatedTime = currentTime;
		m_lastUpdatedFrameCount = m_frameCount;
	}

	m_frameDelta = currentTime - m_framePreviousEndTime;
	m_framePreviousEndTime = currentTime;

	if (m_frameCount > 0)
	{
		m_frameDeltaMin = m_frameDelta < m_frameDeltaMin ? m_frameDelta : m_frameDeltaMin;
		m_frameDeltaMax = m_frameDelta > m_frameDeltaMax ? m_frameDelta : m_frameDeltaMax;
	}

	m_frameCount = (m_frameCount + 1) % UINT64_MAX;

	// Update average

	m_frameDeltaHistory[m_deltaHistoryIndex] = m_frameDelta;
	m_deltaHistoryIndex = (m_deltaHistoryIndex + 1) % (uint32_t)m_frameDeltaHistory.size();

	m_frameDeltaAverage = CrTime();

	for(uint32_t i = 0; i < m_frameDeltaHistory.size(); ++i)
	{
		m_frameDeltaAverage += m_frameDeltaHistory[i];
	}

	m_frameDeltaAverage /= m_frameDeltaHistory.size();
}

CrTime CrFrameTime::GetFrameDelta()
{
	return m_frameDelta;
}

CrTime CrFrameTime::GetFrameDeltaAverage()
{
	return m_frameDeltaAverage;
}

CrTime CrFrameTime::GetFrameDeltaMin()
{
	return m_frameDeltaMin;
}

CrTime CrFrameTime::GetFrameDeltaMax()
{
	return m_frameDeltaMax;
}

uint64_t CrFrameTime::GetFrameCount()
{
	return m_frameCount;
}