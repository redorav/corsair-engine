#include "CrCore_pch.h"

#include "CrTimer.h"

uint64_t CrTime::TicksPerSecond = CrTime::GetTicksPerSecond();

CrTimer::CrTimer(Mode mode)
{
	Reset();

	if (mode == Mode::AutoStart)
	{
		Start();
	}
}

void CrTimer::Start()
{
	m_start = CrTime::Current();
	m_running = true;
}

void CrTimer::Reset()
{
	m_start = CrTime();
	m_accumulated = CrTime();
	m_running = false;
}

bool CrTimer::IsRunning()
{
	return m_running;
}

void CrTimer::Stop()
{
	if (IsRunning())
	{
		m_accumulated += CrTime::Current() - m_start;
		m_start = CrTime();
		m_running = false;
	}
}

CrTime CrTimer::GetCurrent()
{
	return m_accumulated + (IsRunning() ? (CrTime::Current() - m_start) : CrTime());
}
