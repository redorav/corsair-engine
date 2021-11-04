#pragma once

#include "CrRendering_pch.h"

#include "CrCPUStackAllocator.h"

CrCPUStackAllocator::~CrCPUStackAllocator()
{
	delete[] m_memoryBasePointer;
}

void CrCPUStackAllocator::Initialize(size_t size)
{
	m_size = size;
	m_memoryBasePointer = new uint8_t[size];
	m_currentPointer = m_memoryBasePointer;
}

void CrCPUStackAllocator::Reset()
{
	m_currentPointer = m_memoryBasePointer;
}