#pragma once

#include <cstdint>

class CrTime
{
public:
	
	static uint64_t TicksPerSecond;

	CrTime() : m_ticks(0) {}

	static CrTime Current();

	int64_t AsTicks() const;

	float AsSeconds() const;

	float AsMilliseconds() const;

	float AsFPS() const;

	CrTime operator + (const CrTime& other) const;

	CrTime operator - (const CrTime& other) const;

	void operator += (const CrTime& other);

	void operator -= (const CrTime& other);

	bool operator > (const CrTime& other) const;

	bool operator < (const CrTime& other) const;

	bool operator >= (const CrTime& other) const;

	bool operator <= (const CrTime& other) const;

	bool operator == (const CrTime& other) const;

	bool operator != (const CrTime& other) const;

	static uint64_t GetTicksPerSecond();

private:

	explicit CrTime(int64_t ticks) : m_ticks(ticks) {}

	// Time can be an absolute value, a difference or an offset
	int64_t m_ticks = 0;
};

inline uint64_t CrTime::TicksPerSecond = CrTime::GetTicksPerSecond();

inline int64_t CrTime::AsTicks() const
{
	return m_ticks;
}

inline float CrTime::AsSeconds() const
{
	return m_ticks / (float)TicksPerSecond;
}

inline float CrTime::AsMilliseconds() const
{
	return m_ticks * 1000.0f / (float)TicksPerSecond;
}

inline float CrTime::AsFPS() const
{
	return (float)TicksPerSecond / m_ticks;
}

inline CrTime CrTime::operator + (const CrTime& other) const
{
	return CrTime(m_ticks + other.m_ticks);
}

inline CrTime CrTime::operator - (const CrTime& other) const
{
	return CrTime(m_ticks - other.m_ticks);
}

inline void CrTime::operator += (const CrTime& other)
{
	m_ticks += other.m_ticks;
}

inline void CrTime::operator -= (const CrTime& other)
{
	m_ticks -= other.m_ticks;
}

inline bool	CrTime::operator > (const CrTime& other) const
{
	return m_ticks > other.m_ticks;
}

inline bool	CrTime::operator < (const CrTime& other) const
{
	return m_ticks < other.m_ticks;
}

inline bool	CrTime::operator >= (const CrTime& other) const
{
	return m_ticks >= other.m_ticks;
}

inline bool	CrTime::operator <= (const CrTime& other) const
{
	return m_ticks <= other.m_ticks;
}

inline bool	CrTime::operator == (const CrTime& other) const
{
	return m_ticks == other.m_ticks;
}

inline bool	CrTime::operator != (const CrTime& other) const
{
	return m_ticks != other.m_ticks;
}