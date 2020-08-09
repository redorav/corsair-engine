#pragma once

#include <cstdint>

class CrFrameTime
{
public:
	
	static double GetTime();

	static void IncrementFrameCount();

	static float GetFrameDelta();

	static uint32_t GetFrameCount();

private:

	static uint32_t			m_frameCount;
	static double			m_frameDelta;
	static double			m_framePreviousEndTime;

	static double			m_lastUpdatedTime;
	static uint32_t			m_lastUpdatedFrameCount;

	static const uint64_t	secondNanos = 1000000000;
	static const uint64_t	secondMicros = 1000000;
};