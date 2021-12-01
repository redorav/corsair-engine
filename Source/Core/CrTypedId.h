#pragma once

#include "Core/CrNumericLimits.h"

template<typename T, typename U, U DefaultValueT = CrNumericLimits<U>::max()>
class CrTypedId
{
public:

	enum : U { DefaultValue = DefaultValueT };

	static const U MaxId = CrNumericLimits<U>::max();

	CrTypedId() : id(DefaultValue) {}
	explicit CrTypedId(U id) : id(id) {}

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