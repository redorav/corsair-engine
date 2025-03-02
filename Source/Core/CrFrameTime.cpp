#include "Core/CrCore_pch.h"

#include "CrFrameTime.h"

#include "Core/String/CrString.h"
#include "Core/Logging/ICrDebug.h"

uint64_t CrFrameTime::m_frameCount = 0;
crstl::time CrFrameTime::m_frameDelta;
crstl::time CrFrameTime::m_framePreviousEndTime;

uint64_t CrFrameTime::m_lastUpdatedFrameCount;
crstl::time CrFrameTime::m_lastUpdatedTime;
crstl::time CrFrameTime::m_frameDeltaAverage;
crstl::time CrFrameTime::m_frameDeltaMin;
crstl::time CrFrameTime::m_frameDeltaMax;

void CrFrameTime::IncrementFrameCount()
{
	crstl::time currentTime = crstl::time::now();

	if ((currentTime - m_lastUpdatedTime).seconds() > 1.0f)
	{
		//CrPrintProcessMemory("Frame Memory");

		m_lastUpdatedTime = currentTime;
		m_lastUpdatedFrameCount = m_frameCount;
	}

	// If the previous end time hasn't been initialized yet, set it to the current time
	if (m_framePreviousEndTime.ticks() == 0)
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

crstl::time CrFrameTime::GetFrameDelta()
{
	return m_frameDelta;
}

crstl::time CrFrameTime::GetFrameDeltaAverage()
{
	return m_frameDeltaAverage;
}

crstl::time CrFrameTime::GetFrameDeltaMin()
{
	return m_frameDeltaMin;
}

crstl::time CrFrameTime::GetFrameDeltaMax()
{
	return m_frameDeltaMax;
}

uint64_t CrFrameTime::GetFrameIndex()
{
	return m_frameCount;
}