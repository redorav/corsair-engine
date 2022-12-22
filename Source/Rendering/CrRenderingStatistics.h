#pragma once

class CrRenderingStatistics
{
public:

	static void Reset();

	static void AddDrawcall();

	static void AddVertices(uint32_t vertexCount);

	static void AddInstances(uint32_t instanceCount);

	static uint32_t GetDrawcallCount();

	static uint32_t GetVertexCount();

	static uint32_t GetInstanceCount();

private:

	static uint32_t m_drawcallCount;

	static uint32_t m_vertexCount;

	static uint32_t m_instanceCount;
};

inline void CrRenderingStatistics::Reset()
{
	m_drawcallCount = 0;
	m_vertexCount = 0;
	m_instanceCount = 0;
}

inline void CrRenderingStatistics::AddDrawcall()
{
	m_drawcallCount++;
}

inline void CrRenderingStatistics::AddVertices(uint32_t vertexCount)
{
	m_vertexCount += vertexCount;
}

inline void CrRenderingStatistics::AddInstances(uint32_t instanceCount)
{
	m_instanceCount += instanceCount;
}

inline uint32_t CrRenderingStatistics::GetDrawcallCount()
{
	return m_drawcallCount;
}

inline uint32_t CrRenderingStatistics::GetVertexCount()
{
	return m_vertexCount;
}

inline uint32_t CrRenderingStatistics::GetInstanceCount()
{
	return m_instanceCount;
}