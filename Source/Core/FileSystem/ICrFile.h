#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/FileSystem/CrPath.h"

#include "Core/String/CrFixedString.h"

#include <cstdint>

class ICrFile;
using CrFileSharedHandle = CrSharedPtr<ICrFile>;
using CrFileUniqueHandle = CrUniquePtr<ICrFile>;

struct CrDirectoryEntry
{
	CrFixedString512 filename;
	bool isDirectory = false;
};

using FileIteratorFn = CrFixedFunction<12, bool(const CrDirectoryEntry& entry)>;

namespace FileOpenFlags
{
	enum T : uint32_t
	{
		Read        = 1 << 0, // Read from the file
		Write       = 1 << 1, // Write to the file
		Append      = 1 << 2, // Append to the file
		Create      = 1 << 3, // Create file if it does not exist
		ForceCreate = 1 << 4, // Always create file, overwriting previous contents
	};
}

inline FileOpenFlags::T operator | (FileOpenFlags::T a, FileOpenFlags::T b)
{
	return static_cast<FileOpenFlags::T>(static_cast<int>(a) | static_cast<int>(b));
}

namespace SeekOrigin
{
	enum T : uint8_t
	{
		Begin,
		Current,
		End,
	};
}

class ICrFile
{
public:

	ICrFile(const char* filePath, FileOpenFlags::T openFlags);

	virtual ~ICrFile() {}

	virtual size_t Read(void* memory, size_t bytes) const = 0;

	virtual size_t Write(void* memory, size_t bytes) const = 0;

	virtual void Seek(SeekOrigin::T seekOrigin, int64_t byteOffset) = 0;

	virtual void Rewind() = 0;

	virtual uint64_t GetSize() const = 0;

	const char* GetFilePath() const;

	static CrFileUniqueHandle OpenUnique(const char* filePath, FileOpenFlags::T openFlags);

	// TODO Move this elsewhere when we have a FileDevice
	// We need to be able to create different files for different platforms, functionality, etc
	// Functions like FileExists also would want to be part of the file device
	static CrFileSharedHandle OpenFile(const char* filePath, FileOpenFlags::T openFlags);

	// Query if file exists
	static bool FileExists(const char* filePath);

	// Query if directory exists
	static bool DirectoryExists(const char* filePath);

	// Create directory, including the tree that leads up to it
	// If folder was created successfully or folder already exists, return true
	static bool CreateDirectories(const char* directoryPath);

	// Create directory if you know that the rest of the directory structure is present
	// If folder was created successfully or folder already exists, return true
	static bool CreateDirectorySingle(const char* directoryPath);

	// Iterate over directory entries
	static bool ForEachDirectoryEntry(const char* directoryName, const FileIteratorFn& fn);

private:

	static ICrFile* OpenRaw(const char* filePath, FileOpenFlags::T openFlags);

	CrPath m_filePath;

	FileOpenFlags::T m_openFlags;
};

inline const char* ICrFile::GetFilePath() const
{
	return m_filePath.c_str();
}