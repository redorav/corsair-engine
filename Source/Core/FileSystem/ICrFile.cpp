#include "ICrFile.h"

#include "Core/FileSystem/CrFileSystem.h"

#include "Core/SmartPointers/CrSharedPtr.h"

CrFileSharedHandle ICrFile::Create(const CrPath& filePath, FileOpenFlags::T openFlags)
{
	return Create(filePath.string().c_str(), openFlags);
}
