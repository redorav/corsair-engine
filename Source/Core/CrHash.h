#pragma once

#define XXH_INLINE_ALL
#include <xxhash.h>

class CrHash
{
public:
	uint64_t m_hash;

	CrHash() : m_hash(0) {}
	CrHash(const CrHash& hash) : m_hash(hash.m_hash) {}

	void Reset()
	{
		m_hash = 0;
	}

	template<typename T>
	CrHash(void* data, const T& datatype)
	{
		m_hash = ComputeHash(data, datatype);
	}

	CrHash(void* data, size_t dataSize)
	{
		m_hash = ComputeHash(data, dataSize);
	}

	CrHash& operator <<= (const CrHash& other)
	{
		uint64_t hashes[2] = { m_hash, other.m_hash };
		m_hash = ComputeHash(hashes, hashes);
		return *this;
	}

private:

	template<typename T>
	static uint64_t ComputeHash(void* data, const T&)
	{
		// TODO Select hash function at compile time based on the size of datatype.
		// http://aras-p.info/blog/2016/08/09/More-Hash-Function-Tests/
		return XXH64(data, sizeof(T), 100);
	}
};

inline CrHash operator << (const CrHash& first, const CrHash& second)
{
	uint64_t hashes[2] = { first.m_hash, second.m_hash };
	return CrHash(hashes, sizeof(hashes));
}

template<typename T>
class CrAutoHashable
{
public:
	const CrHash& GetHash() const
	{
		return m_hash;
	}

	void Hash()
	{
		static T dummy;
		m_hash.Reset(); // The hash also takes part in the hashing because it's stored. Reset for consistent hashing
		m_hash = CrHash(this, dummy);
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

	const CrHash& GetHash()
	{
		return m_hash;
	}

protected:

	CrHashable() {} // Don't allow instances of this type

private:

	CrHash ComputeHash()
	{
		static_assert(eastl::false_type::value, "Child class of CrHashable must implement ComputeHash");
	}

	CrHash m_hash;
};