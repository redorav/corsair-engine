#pragma once

class CrRenderingStatistics
{
public:

	static void Reset();

	static void AddDrawcall();

	static void AddVertices(uint32_t vertexCount);

	static uint32_t GetDrawcallCount();

	static uint32_t GetVertexCount();

private:

	static uint32_t m_drawcallCount;

	static uint32_t m_vertexCount;
};

inline void CrRenderingStatistics::Reset()
{
	m_drawcallCount = 0;
	m_vertexCount = 0;
}

inline void CrRenderingStatistics::AddDrawcall()
{
	m_drawcallCount++;
}

inline void CrRenderingStatistics::AddVertices(uint32_t vertexCount)
{
	m_vertexCount += vertexCount;
}

inline uint32_t CrRenderingStatistics::GetDrawcallCount()
{
	return m_drawcallCount;
}

inline uint32_t CrRenderingStatistics::GetVertexCount()
{
	return m_vertexCount;
}