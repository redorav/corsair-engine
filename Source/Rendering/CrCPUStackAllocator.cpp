#pragma once

#include "CrRendering_pch.h"

#include "CrCPUStackAllocator.h"

CrCPUStackAllocator::~CrCPUStackAllocator()
{
	delete[] m_memoryBasePointer;
}

void CrCPUStackAllocator::Initialize(uint32_t size)
{
	m_poolSize = size;

	m_memoryBasePointer = new uint8_t[m_poolSize];
	m_currentPointer = m_memoryBasePointer;
}

void CrCPUStackAllocator::Reset()
{
	m_currentPointer = m_memoryBasePointer;
}