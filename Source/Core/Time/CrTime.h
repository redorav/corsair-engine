#pragma once

#include <cstdint>

class CrTime
{
public:
	
	static uint64_t TicksPerSecond;

	CrTime() : m_ticks(0) {}

	static CrTime Current();

	int64_t AsTicks() const;

	double AsSeconds() const;

	double AsMilliseconds() const;

	double AsMicroseconds() const;

	double AsFPS() const;

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

inline double CrTime::AsSeconds() const
{
	return m_ticks / (double)TicksPerSecond;
}

inline double CrTime::AsMilliseconds() const
{
	return m_ticks * 1000.0 / (double)TicksPerSecond;
}

inline double CrTime::AsMicroseconds() const
{
	return m_ticks * 1000000.0 / (double)TicksPerSecond;
}

inline double CrTime::AsFPS() const
{
	return (double)TicksPerSecond / m_ticks;
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