#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Core/String/CrFixedString.h"

#include <cstdint>

class ICrFile;
using CrFileSharedHandle = CrSharedPtr<ICrFile>;

namespace FileOpenFlags
{
	enum T : uint32_t
	{
		Read   = 1 << 0, // Read from the file
		Write  = 1 << 1, // Write to the file
		Append = 1 << 2, // Append to the file
		Create = 1 << 3, // Create file
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

	static const uint32_t MaxFileLength = 512; // Maximum length of a file, avoids dynamic allocation

	ICrFile(const char* filePath, FileOpenFlags::T openFlags);

	virtual ~ICrFile() {}

	virtual size_t Read(void* memory, size_t bytes) const = 0;

	virtual void Seek(SeekOrigin::T seekOrigin, int64_t byteOffset) = 0;

	virtual void Rewind() = 0;

	virtual uint64_t GetSize() const = 0;

	const char* GetFilePath() const;

	static CrFileSharedHandle Create(const CrPath& filePath, FileOpenFlags::T openFlags);

	// TODO Move this elsewhere when we have a FileDevice
	// We need to be able to create different files for different platforms, functionality, etc
	static CrFileSharedHandle Create(const char* filePath, FileOpenFlags::T openFlags);

private:

	CrFixedString<MaxFileLength> m_filePath;

	FileOpenFlags::T m_openFlags;
};

inline const char* ICrFile::GetFilePath() const
{
	return m_filePath.c_str();
}

CrFileSharedHandle Create(const CrPath& filePath, FileOpenFlags::T openFlags);