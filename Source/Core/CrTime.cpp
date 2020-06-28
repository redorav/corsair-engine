#include "CrTime.h"
#include "Core/String/CrString.h"
#include "Core/Logging/ICrDebug.h"

#include <chrono>
#include <string>

using namespace std::chrono;

uint32_t CrTime::m_frameCount = 0;
double CrTime::m_frameDelta;
double CrTime::m_framePreviousEndTime;

uint32_t CrTime::m_lastUpdatedFrameCount;
double CrTime::m_lastUpdatedTime;

long long CrTime::GetNanoTime()
{
	auto time = high_resolution_clock::now();
	return time_point_cast<nanoseconds>(time).time_since_epoch().count();
}

long long CrTime::GetMicroTime()
{
	auto time = high_resolution_clock::now();
	return time_point_cast<microseconds>(time).time_since_epoch().count();
}

long long CrTime::GetMillisTime()
{
	auto time = high_resolution_clock::now();
	return time_point_cast<milliseconds>(time).time_since_epoch().count();
}

double CrTime::GetTime()
{
	auto time = high_resolution_clock::now();
	return double(time_point_cast<nanoseconds>(time).time_since_epoch().count()) / double(secondNanos);
}

void CrTime::IncrementFrameCount()
{
	double currentTime = GetTime();

	if (currentTime - m_lastUpdatedTime > 1.0f)
	{
		uint32_t frames = m_frameCount - m_lastUpdatedFrameCount;

		CrString str = std::to_string(frames).c_str();

		CrLog("FPS %d", frames);
		CrLog("Delta %f", m_frameDelta);

		m_lastUpdatedTime = currentTime;
		m_lastUpdatedFrameCount = m_frameCount;
	}

	m_frameDelta = currentTime - m_framePreviousEndTime;
	m_framePreviousEndTime = currentTime;

	m_frameCount = (m_frameCount + 1) % UINT32_MAX;
}

float CrTime::GetFrameDelta()
{
	return (float)m_frameDelta;
}

uint32_t CrTime::GetFrameCount()
{
	return m_frameCount;
}
