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

void CrFrameTime::IncrementFrameCount()
{
	CrTime currentTime = CrTime::Current();

	if ((currentTime - m_lastUpdatedTime).AsSeconds() > 1.0f)
	{
		//CrPrintProcessMemory("Frame Memory");

		m_lastUpdatedTime = currentTime;
		m_lastUpdatedFrameCount = m_frameCount;
	}

	// If the previous end time hasn't been initialized yet, set it to the current time
	if (m_framePreviousEndTime.AsTicks() == 0)
	{
		m_framePreviousEndTime = currentTime;
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
	m_frameDeltaAverage += (m_frameDelta - m_frameDeltaAverage) / 64;
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

uint64_t CrFrameTime::GetFrameIndex()
{
	return m_frameCount;
}