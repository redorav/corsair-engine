#pragma once

#include "ICrStream.h"
#include "Core/Logging/ICrDebug.h"

template<CrStreamType::T StreamTypeT>
class CrMemoryStream final : public ICrStream
{
public:

	static bool IsReading() { return StreamTypeT == CrStreamType::Read; }
	static bool IsWriting() { return StreamTypeT == CrStreamType::Write; }

	CrMemoryStream(uint8_t* memory)
		: m_baseDataPointer(memory)
		, m_currentPointer(memory) {}

	virtual CrMemoryStream& operator << (bool& value) override { IsReading() ? Read(value) : Write(value); return *this; }
	virtual CrMemoryStream& operator << (char& value) override { IsReading() ? Read(value) : Write(value); return *this; }

	virtual CrMemoryStream& operator << (int8_t& value) override { IsReading() ? Read(value) : Write(value); return *this; }
	virtual CrMemoryStream& operator << (int16_t& value) override { IsReading() ? Read(value) : Write(value); return *this; }
	virtual CrMemoryStream& operator << (int32_t& value) override { IsReading() ? Read(value) : Write(value); return *this; }
	virtual CrMemoryStream& operator << (int64_t& value) override { IsReading() ? Read(value) : Write(value); return *this; }

	virtual CrMemoryStream& operator << (uint8_t& value) override { IsReading() ? Read(value) : Write(value); return *this; }
	virtual CrMemoryStream& operator << (uint16_t& value) override { IsReading() ? Read(value) : Write(value); return *this; }
	virtual CrMemoryStream& operator << (uint32_t& value) override { IsReading() ? Read(value) : Write(value); return *this; }
	virtual CrMemoryStream& operator << (uint64_t& value) override { IsReading() ? Read(value) : Write(value); return *this; }

	virtual CrMemoryStream& operator << (float& value) override { IsReading() ? Read(value) : Write(value); return *this; }
	virtual CrMemoryStream& operator << (double& value) override { IsReading() ? Read(value) : Write(value); return *this; }

	virtual CrMemoryStream& operator << (const bool& value) override { CrAssert(IsWriting()); Write(value); return *this; }
	virtual CrMemoryStream& operator << (const char& value) override { CrAssert(IsWriting()); Write(value); return *this; }

	virtual CrMemoryStream& operator << (const int8_t& value) override { CrAssert(IsWriting()); Write(value); return *this; }
	virtual CrMemoryStream& operator << (const int16_t& value) override { CrAssert(IsWriting()); Write(value); return *this; }
	virtual CrMemoryStream& operator << (const int32_t& value) override { CrAssert(IsWriting()); Write(value); return *this; }
	virtual CrMemoryStream& operator << (const int64_t& value) override { CrAssert(IsWriting()); Write(value); return *this; }

	virtual CrMemoryStream& operator << (const uint8_t& value) override { CrAssert(IsWriting()); Write(value); return *this; }
	virtual CrMemoryStream& operator << (const uint16_t& value) override { CrAssert(IsWriting()); Write(value); return *this; }
	virtual CrMemoryStream& operator << (const uint32_t& value) override { CrAssert(IsWriting()); Write(value); return *this; }
	virtual CrMemoryStream& operator << (const uint64_t& value) override { CrAssert(IsWriting()); Write(value); return *this; }

	virtual CrMemoryStream& operator << (const float& value) override { CrAssert(IsWriting()); Write(value); return *this; }
	virtual CrMemoryStream& operator << (const double& value) override { CrAssert(IsWriting()); Write(value); return *this; }

	virtual CrMemoryStream& operator << (CrStreamDataBlob& dataBlob) override
	{
		*this << dataBlob.size;

		if (IsReading())
		{
			memcpy(dataBlob.data, m_currentPointer, dataBlob.size);
		}
		else
		{
			memcpy(m_currentPointer, dataBlob.data, dataBlob.size);
		}

		m_currentPointer += dataBlob.size;

		return *this;
	}

	virtual CrMemoryStream& operator << (crstl::string& value) override
	{
		uint32_t stringSize;

		if (IsReading())
		{
			*this << stringSize;
			value.resize(stringSize);
			Read(value.data(), stringSize);
		}
		else
		{
			stringSize = (uint32_t)value.size();
			*this << stringSize;
			Write(value.data(), stringSize);
		}

		return *this;
	}

	virtual void Read(void* dstBuffer, size_t sizeBytes) override
	{
		memcpy(dstBuffer, m_currentPointer, sizeBytes);
		m_currentPointer += sizeBytes;
	}

	virtual void Write(const void* srcBuffer, size_t sizeBytes) override
	{
		memcpy(m_currentPointer, srcBuffer, sizeBytes);
		m_currentPointer += sizeBytes;
	}

private:

	template<typename T>
	void Read(T& value)
	{
		value = *(T*)m_currentPointer;
		m_currentPointer += sizeof(value);
	}

	template<typename T>
	void Write(const T& value)
	{
		*((T*)m_currentPointer) = value;
		m_currentPointer += sizeof(value);
	}

	uint8_t* m_baseDataPointer;
	uint8_t* m_currentPointer;
};

typedef CrMemoryStream<CrStreamType::Read> CrReadMemoryStream;
typedef CrMemoryStream<CrStreamType::Write> CrWriteMemoryStream;