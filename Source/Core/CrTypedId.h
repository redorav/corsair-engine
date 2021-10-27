#pragma once

#include "Core/CrNumericLimits.h"

template<typename T, typename U>
class CrTypedId
{
public:

	static const U MaxId = CrNumericLimits<U>::max();

	CrTypedId() : id(0) {}
	explicit CrTypedId(U id) : id(id) {}

	CrTypedId& operator = (U otherId) { id = otherId; return *this; }

	CrTypedId operator - (CrTypedId otherId) { return CrTypedId(id - otherId.id); }
	CrTypedId operator - (U otherId) { return CrTypedId(id - otherId); }

	bool operator < (CrTypedId otherId) { return id < otherId.id; }
	bool operator > (CrTypedId otherId) { return id > otherId.id; }
	bool operator == (CrTypedId otherId) { return id == otherId.id; }
	bool operator != (CrTypedId otherId) { return id != otherId.id; }

	CrTypedId& operator ++ () { id++; return *this; } // Pre-increment
	CrTypedId operator ++ (int) { CrTypedId temp = *this; id++; return temp; } // Post-increment

	U id;
};