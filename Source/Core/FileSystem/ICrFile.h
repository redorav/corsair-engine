#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/FileSystem/CrFixedPath.h"
#include "Core/Logging/ICrDebug.h"

#include "Core/String/CrFixedString.h"
#include "Core/SmartPointers/CrIntrusivePtr.h"

#include "stdint.h"

struct CrDirectoryEntry
{
	// Points to the directory passed in to the find function. Useful to build full path
	const char* directory = nullptr;

	// Name of the file
	CrFixedPath filename;

	// Whether entry is a directory
	bool isDirectory = false;
};

using FileIteratorFunction = CrFixedFunction<32, bool(const CrDirectoryEntry& entry)>;

namespace FileOpenFlags
{
	enum T : uint32_t
	{
		None        = 0 << 0,
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

class ICrFile final : public CrIntrusivePtrInterface
{
public:

	ICrFile(const char* filePath, FileOpenFlags::T openFlags, void* fileHandle, uint64_t fileSize);

	~ICrFile();

	size_t Read(void* memory, size_t bytes) const;

	size_t Write(const void* memory, size_t bytes) const;

	void Seek(SeekOrigin::T seekOrigin, int64_t byteOffset);

	void Rewind();

	uint64_t GetSize() const;

	FileOpenFlags::T GetFlags() const { return m_openFlags; }

	const char* GetFilePath() const;

	static CrFileUniqueHandle OpenUnique(const char* filePath, FileOpenFlags::T openFlags);

	// TODO Move this elsewhere when we have a FileDevice
	// We need to be able to create different files for different platforms, functionality, etc
	// Functions like FileExists also would want to be part of the file device
	static CrFileHandle OpenFile(const char* filePath, FileOpenFlags::T openFlags);

	// Copy a file to a different location
	static bool FileCopy(const char* sourcefilePath, const char* destinationFilePath, bool replace = false);

	// Unconditionally delete a file. If successfully deleted or file didn't exist, return true
	static bool FileDelete(const char* filePath);

	// Query if file exists
	static bool FileExists(const char* filePath);

	// Move a file to a different location
	static bool FileMove(const char* sourcefilePath, const char* destinationFilePath, bool replace = false);

	// Query if directory exists
	static bool DirectoryExists(const char* directoryPath);

	// Create directory, including the tree that leads up to it
	// If folder was created successfully or folder already exists, return true
	static bool CreateDirectories(const char* directoryPath);

	// Create directory if you know that the rest of the directory structure is present
	// If folder was created successfully or folder already exists, return true
	static bool CreateDirectorySingle(const char* directoryPath);

	// Iterate over directory entries
	static bool ForEachDirectoryEntry(const char* directoryPath, bool recursive, const FileIteratorFunction& function);

private:

	size_t ReadPS(void* memory, size_t bytes) const;

	size_t WritePS(const void* memory, size_t bytes) const;

	static ICrFile* OpenRaw(const char* filePath, FileOpenFlags::T openFlags);

	CrFixedPath m_filePath;

	FileOpenFlags::T m_openFlags;

	void* m_fileHandle = nullptr;

	uint64_t m_fileSize = 0;
};

inline const char* ICrFile::GetFilePath() const
{
	return m_filePath.c_str();
}

inline size_t ICrFile::Read(void* memory, size_t bytes) const
{
	CrAssertMsg(m_openFlags & FileOpenFlags::Read, "File must have read flag");
	return ReadPS(memory, bytes);
}

inline size_t ICrFile::Write(const void* memory, size_t bytes) const
{
	CrAssertMsg(m_openFlags & FileOpenFlags::Write, "File must have write flag");
	return WritePS(memory, bytes);
}