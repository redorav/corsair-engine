#include "CrFile_win.h"

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Core/Logging/ICrDebug.h"

#include <windows.h>

ICrFile* ICrFile::CreateRaw(const char* filePath, FileOpenFlags::T openFlags)
{
	return new CrFileWindows(filePath, openFlags);
}

// https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
CrFileWindows::CrFileWindows(const char* filePath, FileOpenFlags::T openFlags) : ICrFile(filePath, openFlags)
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
		dwCreationDisposition = OPEN_EXISTING;
	}
	else if (openFlags & FileOpenFlags::ForceCreate)
	{
		dwCreationDisposition = OPEN_ALWAYS;
	}
	else
	{
		dwCreationDisposition = OPEN_EXISTING;
	}

	// Allow other processes to access this file, but only for reading
	dwShareMode = FILE_SHARE_READ;


	dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;

	m_fileHandle = ::CreateFileA
	(
		filePath,
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile
	);

	if (m_fileHandle == INVALID_HANDLE_VALUE)
	{
		CrLog("Invalid handle for file %s", filePath);
	}

	// Note that if the return value is INVALID_FILE_SIZE (0xffffffff), an application must call GetLastError 
	// to determine whether the function has succeeded or failed. The reason the function may appear to fail 
	// when it has not is that lpFileSizeHigh could be non-NULL or the file size could be 0xffffffff. In this 
	// case, GetLastError will return NO_ERROR (0) upon success. Because of this behavior, it is recommended 
	// that you use GetFileSizeEx instead.
	LARGE_INTEGER fileSize;
	GetFileSizeEx(m_fileHandle, &fileSize);

	m_fileSize = fileSize.QuadPart;

	if(!(openFlags & FileOpenFlags::Append))
	{
		Rewind();
	}
}

CrFileWindows::~CrFileWindows()
{
	CloseHandle(m_fileHandle);
	// Handle error
}

size_t CrFileWindows::Read(void* memory, size_t bytes) const
{
	DWORD numberOfBytesRead;
	ReadFile(m_fileHandle, memory, (DWORD)bytes, &numberOfBytesRead, nullptr);
	return (size_t)numberOfBytesRead;
}

size_t CrFileWindows::Write(void* memory, size_t bytes) const
{
	DWORD numberOfBytesWritten;
	WriteFile(m_fileHandle, memory, (DWORD)bytes, &numberOfBytesWritten, nullptr);
	return (size_t)numberOfBytesWritten;
}

void CrFileWindows::Seek(SeekOrigin::T seekOrigin, int64_t byteOffset)
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

void CrFileWindows::Rewind()
{
	SetFilePointer(m_fileHandle, 0, nullptr, FILE_BEGIN);
}

uint64_t CrFileWindows::GetSize() const
{
	return m_fileSize;
}