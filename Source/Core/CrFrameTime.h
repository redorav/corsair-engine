#pragma once

#include <cstdint>

#include "Core/Time/CrTime.h"
#include "Core/Containers/CrArray.h"

class CrFrameTime
{
public:
	
	static void IncrementFrameCount();

	static CrTime GetFrameDelta();

	static CrTime GetFrameDeltaAverage();

	static CrTime GetFrameDeltaMin();

	static CrTime GetFrameDeltaMax();

	static uint64_t GetFrameCount();

private:

	static uint64_t			m_frameCount;

	static CrTime			m_frameDelta;

	static CrTime			m_frameDeltaAverage;

	static CrTime			m_frameDeltaMin;

	static CrTime			m_frameDeltaMax;

	static CrTime			m_framePreviousEndTime;

	static CrTime			m_lastUpdatedTime;

	static uint64_t			m_lastUpdatedFrameCount;

	static uint32_t			m_deltaHistoryIndex;

	static CrArray<CrTime, 30> m_frameDeltaHistory;
};