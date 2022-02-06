#pragma once

#include "ICrStream.h"

#include "Core/FileSystem/ICrFile.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/SmartPointers/CrSharedPtr.h"

template<CrStreamType::T StreamTypeT>
class CrFileStream final : public ICrStream
{
public:

	static bool IsReading() { return StreamTypeT == CrStreamType::Read; }
	static bool IsWriting() { return StreamTypeT == CrStreamType::Write; }

	// Initialize file stream from a path
	CrFileStream(const char* filePath)
	{
		FileOpenFlags::T openFlags = FileOpenFlags::None;

		switch (StreamTypeT)
		{
			case CrStreamType::Read: openFlags = FileOpenFlags::Read; break;
			case CrStreamType::Write: openFlags = FileOpenFlags::Write | FileOpenFlags::ForceCreate; break;
			case CrStreamType::ReadWrite: openFlags = FileOpenFlags::Read | FileOpenFlags::Write | FileOpenFlags::ForceCreate; break;
		}

		m_file = ICrFile::OpenFile(filePath, openFlags);
	}

	// Initialize file stream from an opened file. We need to make sure the required
	// flags are set on the file
	CrFileStream(const CrFileSharedHandle& file)
	{
		if (IsWriting())
		{
			CrAssertMsg(file->GetFlags() & FileOpenFlags::Write, "File needs the write flag");
		}
	
		if (IsReading())
		{
			CrAssertMsg(file->GetFlags() & FileOpenFlags::Read, "File needs the read flag");
		}

		m_file = file;
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

	virtual CrFileStream& operator << (const bool& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (const char& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }

	virtual CrFileStream& operator << (const int8_t& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (const int16_t& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (const int32_t& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (const int64_t& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }

	virtual CrFileStream& operator << (const uint8_t& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (const uint16_t& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (const uint32_t& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (const uint64_t& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }

	virtual CrFileStream& operator << (const float& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }
	virtual CrFileStream& operator << (const double& value) override { CrAssert(IsWriting()); Write(&value, sizeof(value)); return *this; }

	virtual CrFileStream& operator << (CrStreamDataBlob& dataBlob) override
	{
		if(IsReading())
		{
			Read(&dataBlob.size, sizeof(dataBlob.size));
			Read(dataBlob.data, dataBlob.size);
		}
		else
		{
			Write(&dataBlob.size, sizeof(dataBlob.size));
			Write(dataBlob.data, dataBlob.size);
		}

		return *this;
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

	const ICrFile* GetFile() const
	{
		return m_file.get();
	}

	virtual void Read(void* dstBuffer, size_t sizeBytes) override
	{
		m_file->Read(dstBuffer, sizeBytes);
	}

	virtual void Write(const void* srcBuffer, size_t sizeBytes) override
	{
		m_file->Write(srcBuffer, sizeBytes);
	}

private:

	CrFileSharedHandle m_file;
};

typedef CrFileStream<CrStreamType::Read> CrReadFileStream;
typedef CrFileStream<CrStreamType::Write> CrWriteFileStream;