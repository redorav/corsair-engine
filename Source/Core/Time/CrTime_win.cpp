#include "Core/CrCore_pch.h"

#include "CrTime.h"

#include <windows.h>

uint64_t CrTime::GetTicksPerSecond()
{
	LARGE_INTEGER ticksPerSecond;
	QueryPerformanceFrequency(&ticksPerSecond);
	return ticksPerSecond.QuadPart;
}

CrTime CrTime::Current()
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return CrTime(ticks.QuadPart);
}
