#pragma once

#include "EASTL/sort.h"

template <typename RandomAccessIterator>
void CrQuicksort(RandomAccessIterator first, RandomAccessIterator last)
{
	eastl::quick_sort(first, last);
}