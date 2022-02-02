#include "CrFileANSI.h"

#include "Core/Logging/ICrDebug.h"

#include "SmartPointers/CrSharedPtr.h"

ICrFile* ICrFile::CreateRaw(const char* filePath, FileOpenFlags::T openFlags)
{
	return new CrFileANSI(filePath, openFlags);
}

CrFileANSI::CrFileANSI(const char* filePath, FileOpenFlags::T openFlags) : ICrFile(filePath, openFlags)
{
	const char* fopenFlags = "";

	if ((openFlags & FileOpenFlags::Read) && (openFlags & FileOpenFlags::Append))
	{
		fopenFlags = "ab+";
	}
	else if ((openFlags & FileOpenFlags::Read) && (openFlags & FileOpenFlags::Write))
	{
		fopenFlags = "rb+";
	}
	else if (openFlags & FileOpenFlags::Append)
	{
		fopenFlags = "ab";
	}
	else if (openFlags & FileOpenFlags::Write)
	{
		fopenFlags = "wb";
	}
	else if (openFlags & FileOpenFlags::Read)
	{
		fopenFlags = "rb";
	}

	m_file = fopen(filePath, fopenFlags);

#if defined(_WIN32)
	_fseeki64(m_file, 0, SEEK_END);
	m_fileSize = _ftelli64(m_file);
#else
	// Not implemented
#endif

	if(!(openFlags & FileOpenFlags::Append))
	{
		rewind(m_file);
	}
}

CrFileANSI::~CrFileANSI()
{
	fclose(m_file);
}

size_t CrFileANSI::ReadPS(void* memory, size_t bytes) const
{
	return fread(memory, 1, bytes, m_file);
}

size_t CrFileANSI::WritePS(void* memory, size_t bytes) const
{
	return fwrite(memory, 1, bytes, m_file);
}

void CrFileANSI::Seek(SeekOrigin::T seekOrigin, int64_t byteOffset)
{
	int fseekOrigin = SEEK_SET;

	switch (seekOrigin)
	{
		case SeekOrigin::Begin:
			fseekOrigin = SEEK_SET;
			break;
		case SeekOrigin::Current:
			fseekOrigin = SEEK_CUR;
			break;
		case SeekOrigin::End:
			fseekOrigin = SEEK_END;
			break;
	}

#if defined(_WIN32)
	_fseeki64(m_file, byteOffset, fseekOrigin);
#else
	#error implement
#endif
}

void CrFileANSI::Rewind()
{
	rewind(m_file);
}

uint64_t CrFileANSI::GetSize() const
{
	return m_fileSize;
}
