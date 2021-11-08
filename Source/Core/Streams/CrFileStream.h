#pragma once

#include "ICrStream.h"
#include "Core/FileSystem/ICrFile.h"

template<CrStreamType::T StreamTypeT>
class CrFileStream final : public ICrStream
{
public:

	static const bool IsReading() { return StreamTypeT == CrStreamType::Read; }
	static const bool IsWriting() { return StreamTypeT == CrStreamType::Write; }

	CrFileStream(const char* filePath)
	{
		FileOpenFlags::T openFlags = FileOpenFlags::Read;

		switch (StreamTypeT)
		{
			case CrStreamType::Read: openFlags = FileOpenFlags::Read; break;
			case CrStreamType::Write: openFlags = FileOpenFlags::Write | FileOpenFlags::ForceCreate; break;
			case CrStreamType::ReadWrite: openFlags = FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::ForceCreate; break;
		}

		m_file = ICrFile::OpenUnique(filePath, openFlags);
	}

	virtual CrFileStream& operator << (bool& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (char& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }

	virtual CrFileStream& operator << (int8_t& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (int16_t& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (int32_t& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (int64_t& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }

	virtual CrFileStream& operator << (uint8_t& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (uint16_t& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (uint32_t& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (uint64_t& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }

	virtual CrFileStream& operator << (float& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (double& value) override { IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this; }

	virtual CrFileStream& operator << (CrStreamRawData& rawData)
	{
		if(IsReading())
		{
			Read(&rawData.size, sizeof(rawData.size));
			Read(rawData.data, rawData.size);
		}
		else
		{
			Write(&rawData.size, sizeof(rawData.size));
			Write(rawData.data, rawData.size);
		}

		return *this;
	}

	virtual uint64_t Size() const override { return m_sizeBytes; }

	template<typename T, typename std::enable_if<std::is_enum<T>::value, bool>::type = true>
	CrFileStream& operator<<(T& value)
	{
		IsReading() ? Read(&value, sizeof(value)) : Write(&value, sizeof(value)); return *this;
	}

	virtual CrFileStream& operator << (CrString& value) override
	{
		if (IsReading())
		{
			uint32_t stringSize;
			*this << stringSize;
			value.resize(stringSize);
			Read(value.data(), stringSize);
		}
		else
		{
			uint32_t stringSize = (uint32_t)value.size();
			Write(&stringSize, sizeof(stringSize));
			Write(value.data(), stringSize);
		}

		return *this;
	}

	template<typename T>
	CrFileStream& operator << (CrVector<T>& value)
	{
		uint32_t size = (uint32_t)value.size();
		*this << size;

		if (IsReading())
		{
			value.resize(size);
		}

		if (std::is_fundamental<T>::value)
		{
			CrStreamRawData data(value.data(), value.size());
			*this << data;
		}
		else
		{
			for (T& v : value)
			{
				*this << v; // Assumes v has a compatible streaming operator
			}
		}

		return *this;
	}

	uint64_t GetFileSize() const
	{
		return m_file->GetSize();
	}

private:

	void Read(void* dstBuffer, size_t sizeBytes)
	{
		uint64_t bytesRead = m_file->Read(dstBuffer, sizeBytes);
		m_sizeBytes += bytesRead;
	}

	void Write(void* srcBuffer, size_t sizeBytes)
	{
		uint64_t bytesWritten = m_file->Write(srcBuffer, sizeBytes);
		m_sizeBytes += bytesWritten;
	}

	uint64_t m_sizeBytes = 0; // Size of bytes read or written to (file knows its total size)

	CrFileUniqueHandle m_file;
};

typedef CrFileStream<CrStreamType::Read> CrReadFileStream;
typedef CrFileStream<CrStreamType::Write> CrWriteFileStream;