#include "Core/Logging/ICrDebug.h"

template<typename T>
inline T AlignUpPow2(T input, uintptr_t pow2)
{
	uintptr_t uInput = (uintptr_t)input;
	CrAssertMsg(pow2 && ((pow2 & (pow2 - 1)) == 0), "Not a power of 2");
	return (T)((uInput + pow2 - 1) & ~(pow2 - 1));
}

template<typename T>
inline T AlignUp256(T input)
{
	return AlignUpPow2(input, 256);
}