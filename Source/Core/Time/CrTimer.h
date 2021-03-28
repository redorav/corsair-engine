#pragma once

#include "Core/Time/CrTime.h"

class CrTimer
{
public:
	
	enum class Mode
	{
		AutoStart,
		Manual
	};

	CrTimer(Mode mode = Mode::AutoStart);

	void Start();
	
	void Stop();

	void Reset();

	bool IsRunning();

	CrTime GetCurrent();

private:

	CrTime m_start;

	CrTime m_accumulated;

	bool m_running;
};