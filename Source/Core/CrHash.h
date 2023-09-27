#pragma once

#include "Core/CrMacros.h"

warnings_off
#define XXH_INLINE_ALL
#include <xxhash.h>
warnings_on

class CrHash
{
public:

	static const uint64_t HashSeed = 100;

	CrHash() : m_hash(0) {}

	explicit CrHash(uint64_t hash) : m_hash(hash) {}

	void Reset()
	{
		m_hash = 0;
	}

	uint64_t GetHash() const
	{
		return m_hash;
	}

	template<typename T>
	CrHash(const T& data)
	{
		m_hash = ComputeHash<T>(&data, sizeof(T));
	}

	template<typename T>
	CrHash(const T* data, uint64_t dataSize)
	{
		m_hash = ComputeHash<T>(data, dataSize);
	}

	CrHash(const char* data, uint32_t length)
	{
		m_hash = ComputeHash<const char>(data, length);
	}

	// Streaming operator, incrementally adds hash by combining existing hash with incoming hash
	CrHash& operator << (CrHash other)
	{
		uint64_t hashes[2] = { m_hash, other.m_hash };
		m_hash = ComputeHash(hashes, sizeof(hashes));
		return *this;
	}

	bool operator == (const CrHash& other) const
	{
		return m_hash == other.m_hash;
	}

	bool operator != (const CrHash& other) const
	{
		return m_hash != other.m_hash;
	}

	// TODO Select hash function at compile time based on the size of datatype.
	// http://aras-p.info/blog/2016/08/09/More-Hash-Function-Tests/
	template<typename T>
	static uint64_t ComputeHash(const T* data)
	{
		return XXH64(data, sizeof(T), HashSeed);
	}

	template<typename T>
	static uint64_t ComputeHash(const T* data, uint64_t dataSize)
	{
		return XXH64(data, dataSize, HashSeed);
	}

private:

	// Don't hash pointers directly
	template<typename T> CrHash(const T* data);
	template<typename T> CrHash(T* data);

	uint64_t m_hash;
};

// Combine two hashes and return the hash of both
inline CrHash operator + (CrHash first, CrHash second)
{
	uint64_t hashes[2] = { first.GetHash(), second.GetHash() };
	return CrHash(hashes, sizeof(hashes));
}

namespace eastl
{
	template<>
	struct hash<CrHash>
	{
		size_t operator()(const CrHash& h) const
		{
			return (size_t)h.GetHash();
		}
	};
};