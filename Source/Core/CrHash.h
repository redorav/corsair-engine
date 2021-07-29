#pragma once

#define XXH_INLINE_ALL
#include <xxhash.h>

class CrHash
{
public:

	static const uint64_t HashSeed = 100;

	uint64_t m_hash;

	CrHash() : m_hash(0) {}

	CrHash(const CrHash& hash) : m_hash(hash.m_hash) {}

	void Reset()
	{
		m_hash = 0;
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
};

inline CrHash operator << (CrHash first, CrHash second)
{
	uint64_t hashes[2] = { first.m_hash, second.m_hash };
	return CrHash(hashes, sizeof(hashes));
}

template<typename T>
class CrAutoHashable
{
public:

	CrHash GetHash() const
	{
		return m_hash;
	}

	void Hash()
	{
		m_hash.Reset(); // The hash also takes part in the hashing because it's stored. Reset for consistent hashing
		m_hash = CrHash((T*)this);
	}

protected:

	CrAutoHashable() {} // Don't allow instances of this type

private:

	CrHash m_hash;
};

template<typename T>
class CrHashable
{
public:

	void Hash()
	{
		m_hash = static_cast<T*>(this)->ComputeHash();
	}

	CrHash GetHash() const
	{
		return m_hash;
	}

protected:

	CrHashable() {} // Don't allow instances of this type

private:

	CrHash m_hash;
};