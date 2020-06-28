#pragma once

#include "CrRendering_pch.h"

#include "CrCPUStackAllocator.h"

#include "Core/CrAlignment.h"

CrCPUStackAllocator::CrCPUStackAllocator()
{
	// TODO Configurable per platform
	m_poolSize = 8 * 1024 * 1024; // 8 MB

	m_memoryBasePointer = new uint8_t[m_poolSize];
	m_currentPointer = m_memoryBasePointer;
}

CrCPUStackAllocator::~CrCPUStackAllocator()
{
	delete[] m_memoryBasePointer;
}

// Reserve a memory chunk from the constant buffer allocation pool, with size equal to the buffer size
// Then return the pointer. The calling code is responsible for filling in that memory.
void* CrCPUStackAllocator::Allocate(uint32_t bufferSize)
{
	void* bufferPointer = m_currentPointer; // Take the current pointer
	size_t alignedBufferSize = Align256(bufferSize); // Align memory as required
	CrAssertMsg(m_currentPointer + alignedBufferSize < m_memoryBasePointer + m_poolSize, "Ran out of memory in stream!");
	m_currentPointer += alignedBufferSize; // Reserve as many bytes as needed by the buffer
	return bufferPointer;
}

void CrCPUStackAllocator::Reset()
{
	m_currentPointer = m_memoryBasePointer;
}