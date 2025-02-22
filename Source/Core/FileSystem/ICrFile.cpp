#include "Core/CrCore_pch.h"

#include "ICrFile.h"

ICrFile::ICrFile(const char* filePath, FileOpenFlags::T openFlags, void* fileHandle, uint64_t fileSize)
{
	m_filePath = filePath;
	m_openFlags = openFlags;
	m_fileHandle = fileHandle;
	m_fileSize = fileSize;
}

CrFileHandle ICrFile::OpenFile(const char* filePath, FileOpenFlags::T openFlags)
{
	return CrFileHandle(OpenRaw(filePath, openFlags));
}