#pragma once

#include "Core/CrNumericLimits.h"

template<typename T, typename U, U DefaultValueT = CrNumericLimits<U>::max()>
class CrTypedID
{
public:

	using type = U;

	enum : U { DefaultValue = DefaultValueT };

	static const U MaxId = CrNumericLimits<U>::max();

	CrTypedID() : id(DefaultValue) {}
	explicit CrTypedID(U id) : id(id) {}

	CrTypedID operator - (CrTypedID otherId) { return CrTypedID(id - otherId.id); }
	CrTypedID operator - (U otherId) { return CrTypedID(id - otherId); }

	bool operator < (CrTypedID otherId) { return id < otherId.id; }
	bool operator > (CrTypedID otherId) { return id > otherId.id; }
	bool operator == (CrTypedID otherId) { return id == otherId.id; }
	bool operator != (CrTypedID otherId) { return id != otherId.id; }

	CrTypedID& operator ++ () { id++; return *this; } // Pre-increment
	CrTypedID operator ++ (int) { CrTypedID temp = *this; id++; return temp; } // Post-increment

	U id;
};