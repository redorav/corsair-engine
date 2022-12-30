#include "Core/CrCore_pch.h"

#include <malloc.h>

// EASTL requires the user to implement a couple of operator new to name memory allocations and other details
// We'll implement the most basic ones possible here
// https://stackoverflow.com/questions/42565582/how-to-track-memory-usage-using-eastl

void* operator new[](size_t size, const char* /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
	return malloc(size);
}

void* operator new[](size_t size, size_t /*alignment*/, size_t /*alignmentOffset*/, const char* /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
	return malloc(size);
}