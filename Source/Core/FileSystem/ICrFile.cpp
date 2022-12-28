#include "CrCore_pch.h"

#include "ICrFile.h"

ICrFile::ICrFile(const char* filePath, FileOpenFlags::T openFlags, void* fileHandle, uint64_t fileSize)
{
	m_filePath = filePath;
	m_openFlags = openFlags;
	m_fileHandle = fileHandle;
	m_fileSize = fileSize;
}

CrFileUniqueHandle ICrFile::OpenUnique(const char* filePath, FileOpenFlags::T openFlags)
{
	return CrFileUniqueHandle(OpenRaw(filePath, openFlags));
}

CrFileHandle ICrFile::OpenFile(const char* filePath, FileOpenFlags::T openFlags)
{
	return CrFileHandle(OpenRaw(filePath, openFlags));
}

bool ICrFile::CreateDirectories(const char* directoryPath)
{
	if (!directoryPath)
	{
		return false;
	}

	bool directoryCreated = CreateDirectorySingle(directoryPath);

	if (directoryCreated)
	{
		return true;
	}
	else
	{
		const char* Separator = "\\/";

		// Start working our way backwards. Chances are most of the path exists
		CrString tempPath = directoryPath;
		size_t separatorPos = tempPath.find_last_of(Separator);

		// Skip trailing slash
		if (separatorPos == tempPath.size() - 1)
		{
			separatorPos = tempPath.find_last_of(Separator, separatorPos - 1);
		}

		while (separatorPos != CrString::npos)
		{
			tempPath[separatorPos] = 0;
			bool subdirectoryCreated = CreateDirectorySingle(tempPath.c_str());
			tempPath[separatorPos] = '/';

			// If we created the subdirectory, we now need to move forward
			if (subdirectoryCreated)
			{
				separatorPos = tempPath.find_first_of(Separator, separatorPos + 1);
				break;
			}
			else
			{
				separatorPos = tempPath.find_last_of(Separator, separatorPos - 1);
			}
		}

		bool allPathsCreated = false;

		// All the paths from here should be correctly created. If any one path fails,
		// we need to assume there is an error
		while (separatorPos != CrString::npos)
		{
			tempPath[separatorPos] = 0;
			bool subdirectoryCreated = CreateDirectorySingle(tempPath.c_str());
			tempPath[separatorPos] = '/';
			if (subdirectoryCreated)
			{
				separatorPos = tempPath.find_first_of(Separator, separatorPos + 1);
				allPathsCreated |= true;
			}
			else
			{
				allPathsCreated = false;
				break;
			}
		}

		return allPathsCreated;
	}
}