#pragma once

template<CrStreamType::T StreamTypeT>
class CrMemoryStream final : public ICrStream
{
public:

	static const bool IsReading() { return StreamTypeT == CrStreamType::Read; }
	static const bool IsWriting() { return StreamTypeT == CrStreamType::Write; }

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

	virtual CrMemoryStream& operator << (CrStreamRawData& rawData)
	{
		if (IsReading())
		{
			memcpy(rawData.data, m_currentPointer, rawData.size);
		}
		else
		{
			memcpy(m_currentPointer, rawData.data, rawData.size);
		}

		m_currentPointer += rawData.size;

		return *this;
	}

	template<typename T, typename enable_if<std::is_enum<T>::value, bool>::type = true>
	CrMemoryStream& operator << (T& value)
	{
		IsReading() ? Read(value) : Write(value); return *this;
	}

	virtual CrMemoryStream& operator << (CrString& value) override
	{
		uint32_t stringSize;

		if (IsReading())
		{
			*this << stringSize;
			value.resize(stringSize);
			memcpy(value.data(), m_currentPointer, stringSize);
		}
		else
		{
			stringSize = (uint32_t)value.size();
			Write(stringSize);
			memcpy(m_currentPointer, value.data(), stringSize);
		}

		m_currentPointer += stringSize;

		return *this;
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

	uint8_t* m_currentPointer;
	uint8_t* m_baseDataPointer;
};

typedef CrMemoryStream<CrStreamType::Read> CrReadMemoryStream;
typedef CrMemoryStream<CrStreamType::Write> CrWriteMemoryStream;