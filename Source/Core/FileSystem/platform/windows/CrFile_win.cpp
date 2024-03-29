#include "Core/CrCore_pch.h"

#include "Core/FileSystem/ICrFile.h"

#include "Core/Logging/ICrDebug.h"
#include "Core/Function/CrFixedFunction.h"

#include <windows.h>

// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
ICrFile* ICrFile::OpenRaw(const char* filePath, FileOpenFlags::T openFlags)
{
	DWORD dwDesiredAccess = 0;
	DWORD dwShareMode = 0;
	LPSECURITY_ATTRIBUTES lpSecurityAttributes = nullptr;
	DWORD dwCreationDisposition = 0;
	DWORD dwFlagsAndAttributes = 0;
	HANDLE hTemplateFile = nullptr;

	if (openFlags & FileOpenFlags::Read)
	{
		dwDesiredAccess |= GENERIC_READ;
	}

	if (openFlags & FileOpenFlags::Write)
	{
		dwDesiredAccess |= GENERIC_WRITE;
	}

	// https://stackoverflow.com/questions/14469607/difference-between-open-always-and-create-always-in-createfile-of-windows-api
	if (openFlags & FileOpenFlags::Create)
	{
		dwCreationDisposition = OPEN_ALWAYS;
	}
	else if (openFlags & FileOpenFlags::ForceCreate)
	{
		dwCreationDisposition = CREATE_ALWAYS;
	}
	else
	{
		dwCreationDisposition = OPEN_EXISTING;
	}

	// Allow other processes to access this file, but only for reading
	dwShareMode = FILE_SHARE_READ;

	dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;

	HANDLE fileHandle = ::CreateFileA
	(
		filePath,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile
	);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		return nullptr;
	}

	// Note that if the return value is INVALID_FILE_SIZE (0xffffffff), an application must call GetLastError 
	// to determine whether the function has succeeded or failed. The reason the function may appear to fail 
	// when it has not is that lpFileSizeHigh could be non-NULL or the file size could be 0xffffffff. In this 
	// case, GetLastError will return NO_ERROR (0) upon success. Because of this behavior, it is recommended 
	// that you use GetFileSizeEx instead.
	LARGE_INTEGER liFileSize;
	GetFileSizeEx(fileHandle, &liFileSize);

	uint64_t fileSize = liFileSize.QuadPart;

	ICrFile* file = new ICrFile(filePath, openFlags, fileHandle, fileSize);

	if (!(openFlags & FileOpenFlags::Append))
	{
		file->Rewind();
	}

	return file;
}

ICrFile::~ICrFile()
{
	CloseHandle(m_fileHandle);
	// Handle error
}

size_t ICrFile::ReadPS(void* memory, size_t bytes) const
{
	DWORD numberOfBytesRead;
	ReadFile(m_fileHandle, memory, (DWORD)bytes, &numberOfBytesRead, nullptr);
	return (size_t)numberOfBytesRead;
}

size_t ICrFile::WritePS(const void* memory, size_t bytes) const
{
	DWORD numberOfBytesWritten;
	WriteFile(m_fileHandle, memory, (DWORD)bytes, &numberOfBytesWritten, nullptr);
	return (size_t)numberOfBytesWritten;
}

void ICrFile::Seek(SeekOrigin::T seekOrigin, int64_t byteOffset)
{
	LARGE_INTEGER liByteOffset;
	liByteOffset.QuadPart = byteOffset;
	
	DWORD dwMoveMethod = FILE_BEGIN;

	switch (seekOrigin)
	{
		case SeekOrigin::Begin:
			dwMoveMethod = FILE_BEGIN;
			break;
		case SeekOrigin::Current:
			dwMoveMethod = FILE_CURRENT;
			break;
		case SeekOrigin::End:
			dwMoveMethod = FILE_END;
			break;
	}

	DWORD result = SetFilePointer
	(
		m_fileHandle,
		liByteOffset.LowPart,
		&liByteOffset.HighPart,
		dwMoveMethod
	);

	if (result == INVALID_SET_FILE_POINTER)
	{
		// Handle error
	}
}

void ICrFile::Rewind()
{
	SetFilePointer(m_fileHandle, 0, nullptr, FILE_BEGIN);
}

uint64_t ICrFile::GetSize() const
{
	return m_fileSize;
}

bool ICrFile::FileCopy(const char* sourcefilePath, const char* destinationFilePath, bool replace)
{
	if (replace)
	{
		DeleteFileA(destinationFilePath);
	}

	return CopyFileA(sourcefilePath, destinationFilePath, true) != 0;
}

bool ICrFile::FileDelete(const char* filePath)
{
	bool success = DeleteFileA(filePath);
	
	if (!success)
	{
		return true;
	}
	else
	{
		DWORD errorCode = GetLastError();
		if (errorCode == ERROR_FILE_NOT_FOUND)
		{
			return true;
		}

		return false;
	}
}

bool ICrFile::FileExists(const char* filePath)
{
	WIN32_FILE_ATTRIBUTE_DATA attributeData;
	
	if (GetFileAttributesExA(filePath, GetFileExInfoStandard, &attributeData))
	{
		return attributeData.dwFileAttributes != INVALID_FILE_ATTRIBUTES && !(attributeData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}
	else
	{
		return false;
	}
}

bool ICrFile::FileMove(const char* sourcefilePath, const char* destinationFilePath, bool replace)
{
	return MoveFileExA(sourcefilePath, destinationFilePath, replace ? MOVEFILE_REPLACE_EXISTING : 0) != 0;
}

bool ICrFile::DirectoryExists(const char* directoryPath)
{
	WIN32_FILE_ATTRIBUTE_DATA attributeData;

	if (GetFileAttributesExA(directoryPath, GetFileExInfoStandard, &attributeData))
	{
		return attributeData.dwFileAttributes != INVALID_FILE_ATTRIBUTES && (attributeData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}
	else
	{
		return false;
	}
}

bool ICrFile::CreateDirectorySingle(const char* directoryPath)
{
	bool success = CreateDirectoryA(directoryPath, nullptr);

	if (success)
	{
		return true;
	}
	else
	{
		DWORD errorCode = GetLastError();
		if (errorCode == ERROR_ALREADY_EXISTS)
		{
			return true;
		}
	}

	return false;
}

bool ICrFile::ForEachDirectoryEntry(const char* directoryPath, bool recursive, const FileIteratorFunction& function)
{
	WIN32_FIND_DATAW findData;

	CrFixedWString512 wPath;
	wPath.append_convert(directoryPath);
	wPath.append(L"/*.*"); // Find everything inside this folder

	// Find first file. Bear in mind "file" is a misnomer as it actually finds any entry,
	// whether directory, etc
	HANDLE hFind = FindFirstFileW(wPath.c_str(), &findData);
	bool continueIterating = true;

	// If we found at least one file, start iterating
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			bool isDot = findData.cFileName[0] == L'.' && findData.cFileName[1] == L'\0';
			bool isDoubleDot = findData.cFileName[0] == L'.' && findData.cFileName[1] == L'.' && findData.cFileName[2] == L'\0';

			// Ignore special cases of '.' and '..' to be more in line with the C++ spec
			if (!isDot && !isDoubleDot)
			{
				CrDirectoryEntry entry;
				entry.directory = directoryPath;
				entry.filename.append_convert<wchar_t>(findData.cFileName);
				entry.isDirectory = findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

				if (recursive && entry.isDirectory)
				{
					CrFixedString512 wSubPath;
					wSubPath.append(directoryPath);
					wSubPath.append("/");
					wSubPath.append(entry.filename.c_str());
					ForEachDirectoryEntry(wSubPath.c_str(), recursive, function);
				}

				continueIterating = function(entry);
			}

			// Find next file
		} while (continueIterating && FindNextFileW(hFind, &findData) != 0);
	}

	FindClose(hFind);

	return true;
}