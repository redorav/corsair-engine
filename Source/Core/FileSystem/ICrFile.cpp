#include "ICrFile.h"

#include "Core/FileSystem/CrFileSystem.h"

#include "Core/SmartPointers/CrSharedPtr.h"

ICrFile::ICrFile(const char* filePath, FileOpenFlags::T openFlags)
{
	m_filePath = filePath;
	m_openFlags = openFlags;
}

CrFileUniqueHandle ICrFile::CreateUnique(const char* filePath, FileOpenFlags::T openFlags)
{
	return CrFileUniqueHandle(OpenRaw(filePath, openFlags));
}

CrFileSharedHandle ICrFile::OpenFile(const char* filePath, FileOpenFlags::T openFlags)
{
	return CrFileSharedHandle(OpenRaw(filePath, openFlags));
}

CrFileSharedHandle ICrFile::OpenFile(const CrPath& filePath, FileOpenFlags::T openFlags)
{
	return OpenFile(filePath.string().c_str(), openFlags);
}