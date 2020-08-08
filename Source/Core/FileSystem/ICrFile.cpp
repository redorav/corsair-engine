#include "ICrFile.h"

#include "Core/FileSystem/CrFileSystem.h"

#include "Core/SmartPointers/CrSharedPtr.h"

ICrFile::ICrFile(const char* filePath, FileOpenFlags::T openFlags)
{
	m_filePath = filePath;
	m_openFlags = openFlags;
}

CrFileSharedHandle ICrFile::Create(const CrPath& filePath, FileOpenFlags::T openFlags)
{
	return Create(filePath.string().c_str(), openFlags);
}
