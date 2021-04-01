#pragma once

#include <cstdint>

#include "Core/Time/CrTime.h"

class CrFrameTime
{
public:
	
	static void IncrementFrameCount();

	static CrTime GetFrameDelta();

	static uint64_t GetFrameCount();

private:

	static uint64_t			m_frameCount;

	static CrTime			m_frameDelta;

	static CrTime			m_framePreviousEndTime;

	static CrTime			m_lastUpdatedTime;

	static uint64_t			m_lastUpdatedFrameCount;
};