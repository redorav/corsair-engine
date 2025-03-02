#pragma once

#include "stdint.h"

#include "crstl/timer.h"

class CrFrameTime
{
public:
	
	static void IncrementFrameCount();

	static crstl::time GetFrameDelta();

	static crstl::time GetFrameDeltaAverage();

	static crstl::time GetFrameDeltaMin();

	static crstl::time GetFrameDeltaMax();

	static uint64_t GetFrameIndex();

private:

	static uint64_t			m_frameCount;

	static crstl::time		m_frameDelta;

	static crstl::time		m_frameDeltaAverage;

	static crstl::time		m_frameDeltaMin;

	static crstl::time		m_frameDeltaMax;

	static crstl::time		m_framePreviousEndTime;

	static crstl::time		m_lastUpdatedTime;

	static uint64_t			m_lastUpdatedFrameCount;

	static uint32_t			m_deltaHistoryIndex;
};