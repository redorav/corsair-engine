#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include <type_traits>

namespace CrStreamType
{
	enum T
	{
		Read,
		Write,
		ReadWrite
	};
};

struct CrStreamDataBlob
{
	CrStreamDataBlob() {}
	CrStreamDataBlob(void* data, uint64_t size) : data(data), size((uint32_t)size) {}
	CrStreamDataBlob(void* data, uint32_t size) : data(data), size(size) {}
	void* data = nullptr;
	uint32_t size = 0;
};

// This pure virtual interface is used to enforce the functions in the
// streams, but in practice the best way to use the streams is to pass
// it in to a templated function and avoid the virtual function calls.
class ICrStream
{
public:

	virtual ~ICrStream() {}

	virtual ICrStream& operator << (bool& value) = 0;
	virtual ICrStream& operator << (char& value) = 0;

	virtual ICrStream& operator << (int8_t& value) = 0;
	virtual ICrStream& operator << (int16_t& value) = 0;
	virtual ICrStream& operator << (int32_t& value) = 0;
	virtual ICrStream& operator << (int64_t& value) = 0;

	virtual ICrStream& operator << (uint8_t& value) = 0;
	virtual ICrStream& operator << (uint16_t& value) = 0;
	virtual ICrStream& operator << (uint32_t& value) = 0;
	virtual ICrStream& operator << (uint64_t& value) = 0;

	virtual ICrStream& operator << (float& value) = 0;
	virtual ICrStream& operator << (double& value) = 0;

	virtual ICrStream& operator << (crstl::string& value) = 0;

	virtual ICrStream& operator << (CrStreamDataBlob& dataBlob) = 0;

	virtual ICrStream& operator << (const bool& value) = 0;
	virtual ICrStream& operator << (const char& value) = 0;

	virtual ICrStream& operator << (const int8_t& value) = 0;
	virtual ICrStream& operator << (const int16_t& value) = 0;
	virtual ICrStream& operator << (const int32_t& value) = 0;
	virtual ICrStream& operator << (const int64_t& value) = 0;

	virtual ICrStream& operator << (const uint8_t& value) = 0;
	virtual ICrStream& operator << (const uint16_t& value) = 0;
	virtual ICrStream& operator << (const uint32_t& value) = 0;
	virtual ICrStream& operator << (const uint64_t& value) = 0;

	virtual ICrStream& operator << (const float& value) = 0;
	virtual ICrStream& operator << (const double& value) = 0;

	virtual void Read(void* dstBuffer, size_t sizeBytes) = 0;
	virtual void Write(const void* srcBuffer, size_t sizeBytes) = 0;
};

// Add operators that work with any type of stream. This allows us
// to have common templated code for all types of streams. The only
// downside is having to expose the Read/Write family of functions

template<typename StreamT, typename T>
StreamT& operator << (StreamT& stream, crstl::vector<T>& value)
{
	uint32_t size = (uint32_t)value.size();
	stream << size;

	if (stream.IsReading())
	{
		value.resize(size);
	}

	if (crstl_is_trivially_copyable(T))
	{
		if (stream.IsReading())
		{
			stream.Read(value.data(), size);
		}
		else
		{
			stream.Write(value.data(), size);
		}
	}
	else
	{
		for (T& v : value)
		{
			stream << v; // Assumes v has a compatible streaming operator
		}
	}

	return stream;
}

// Stream enum types
template<typename StreamT, typename T, typename std::enable_if<std::is_enum<T>::value || std::is_trivially_copyable<T>::value, bool>::type* = nullptr>
StreamT& operator << (StreamT& stream, T& value)
{
	stream.IsReading() ? stream.Read(&value, sizeof(value)) : stream.Write(&value, sizeof(value)); return stream;
}