#pragma once

#define XXH_INLINE_ALL
#include <xxhash.h>

class CrHash
{
public:

	static const uint64_t HashSeed = 100;

	CrHash() : m_hash(0) {}

	CrHash(const CrHash& hash) : m_hash(hash.m_hash) {}

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
	CrHash(T* data)
	{
		m_hash = ComputeHash<T>(data, sizeof(T));
	}

	template<typename T>
	CrHash(T* data, uint64_t dataSize)
	{
		m_hash = ComputeHash<T>(data, dataSize);
	}

	CrHash& operator <<= (CrHash other)
	{
		uint64_t hashes[2] = { m_hash, other.m_hash };
		m_hash = ComputeHash(hashes, sizeof(hashes));
		return *this;
	}

	bool operator == (CrHash other)
	{
		return m_hash == other.m_hash;
	}

	bool operator != (CrHash other)
	{
		return m_hash != other.m_hash;
	}

	// TODO Select hash function at compile time based on the size of datatype.
	// http://aras-p.info/blog/2016/08/09/More-Hash-Function-Tests/
	template<typename T>
	static uint64_t ComputeHash(T* data)
	{
		return XXH64(data, sizeof(T), HashSeed);
	}

	template<typename T>
	static uint64_t ComputeHash(T* data, uint64_t dataSize)
	{
		return XXH64(data, dataSize, HashSeed);
	}

private:

	uint64_t m_hash;
};

inline CrHash operator << (CrHash first, CrHash second)
{
	uint64_t hashes[2] = { first.GetHash(), second.GetHash() };
	return CrHash(hashes, sizeof(hashes));
}

// Auto Hashable is a class that allows arbitrary structs to be hashed easily
// It ensures padding is 0 so that copying or creating new instances doesn't 
// alter the hash. However, it comes at a cost since the structure needs to
// be memset
template<typename T>
class CrAutoHashable
{
public:

	CrAutoHashable()
	{
		memset(this, 0, sizeof(T));
	}

	CrHash GetHash() const
	{
		return m_hash;
	}

	void Hash()
	{
		m_hash.Reset(); // The hash also takes part in the hashing because it's stored. Reset for consistent hashing
		m_hash = CrHash((T*)this, sizeof(T));
	}

private:

	CrHash m_hash;
};