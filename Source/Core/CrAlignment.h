#include "Core/Logging/ICrDebug.h"

// We could use std::align for this functionality, but std::align does not assert that the alignment is a power of 2

inline int AlignPow2(int input, int pow2)
{
	CrAssert(pow2 && ((pow2 & (pow2 - 1)) == 0));
	return (input + pow2 - 1) & ~(pow2 - 1);
}

inline int Align256(int input)
{
	return AlignPow2(input, 256);
}