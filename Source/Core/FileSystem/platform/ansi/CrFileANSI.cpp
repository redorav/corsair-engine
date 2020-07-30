#include "CrFileANSI.h"

#include "SmartPointers/CrSharedPtr.h"

CrFileSharedHandle ICrFile::Create(const char* filePath, FileOpenFlags::T openFlags)
{
	return CrFileSharedHandle(new CrFileANSI(filePath, openFlags));
}

CrFileANSI::CrFileANSI(const char* filePath, FileOpenFlags::T openFlags)
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

size_t CrFileANSI::Read(void* memory, size_t bytes)
{
	return fread(memory, 1, bytes, m_file);
}

void CrFileANSI::Seek(SeekOrigin::T seekOrigin, int64_t byteOffset)
{
#if defined(_WIN32)
	_fseeki64(m_file, bytes, SEEK_SET);
#else

#endif
}

void CrFileANSI::Rewind()
{
	rewind(m_file);
}

uint64_t CrFileANSI::GetSize()
{
	return m_fileSize;
}
