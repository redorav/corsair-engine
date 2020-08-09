#include "CrFrameTime.h"
#include "Core/String/CrString.h"
#include "Core/Logging/ICrDebug.h"

#include <chrono>
#include <string>

using namespace std::chrono;

uint32_t CrFrameTime::m_frameCount = 0;
double CrFrameTime::m_frameDelta;
double CrFrameTime::m_framePreviousEndTime;

uint32_t CrFrameTime::m_lastUpdatedFrameCount;
double CrFrameTime::m_lastUpdatedTime;

double CrFrameTime::GetTime()
{
	auto time = high_resolution_clock::now();
	return double(time_point_cast<nanoseconds>(time).time_since_epoch().count()) / double(secondNanos);
}

void CrFrameTime::IncrementFrameCount()
{
	double currentTime = GetTime();

	if (currentTime - m_lastUpdatedTime > 1.0f)
	{
		uint32_t frames = m_frameCount - m_lastUpdatedFrameCount;

		CrString str = std::to_string(frames).c_str();

		CrLog("FPS %d", frames);
		CrLog("Delta %f", m_frameDelta);
		CrPrintProcessMemory("Frame Memory");

		m_lastUpdatedTime = currentTime;
		m_lastUpdatedFrameCount = m_frameCount;
	}

	m_frameDelta = currentTime - m_framePreviousEndTime;
	m_framePreviousEndTime = currentTime;

	m_frameCount = (m_frameCount + 1) % UINT32_MAX;
}

float CrFrameTime::GetFrameDelta()
{
	return (float)m_frameDelta;
}

uint32_t CrFrameTime::GetFrameCount()
{
	return m_frameCount;
}
