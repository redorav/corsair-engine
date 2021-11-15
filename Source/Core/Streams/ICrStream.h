#pragma once

#include "Core/CrCoreForwardDeclarations.h"

namespace CrStreamType
{
	enum T
	{
		Read,
		Write,
		ReadWrite
	};
};

struct CrStreamRawData
{
	CrStreamRawData() {}
	CrStreamRawData(void* data, size_t size) : data(data), size((uint32_t)size) {}
	CrStreamRawData(void* data, uint32_t size) : data(data), size(size) {}
	void* data;
	uint32_t size;
};

// This pure virtual interface is used to enforce the functions in the
// streams, but in practice the best way to use the streams is to pass
// it in to a templated function and avoid the virtual function calls.
// However if not possible, the stream
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

	virtual ICrStream& operator << (CrString& value) = 0;

	virtual ICrStream& operator << (CrStreamRawData& rawData) = 0;
};