#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include <cstdint>

class ICrFile;
using CrFileSharedHandle = CrSharedPtr<ICrFile>;

namespace FileOpenFlags
{
	enum T : uint8_t
	{
		Read   = 1 << 0, // Read from the file
		Write  = 1 << 1, // Write to the file
		Append = 1 << 2, // Append to the file
		Create = 1 << 3, // Create file
	};
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

	virtual ~ICrFile() {}

	virtual size_t Read(void* memory, size_t bytes) = 0;

	virtual void Seek(SeekOrigin::T seekOrigin, int64_t byteOffset) = 0;

	virtual void Rewind() = 0;

	virtual uint64_t GetSize() = 0;

	static CrFileSharedHandle Create(const CrPath& filePath, FileOpenFlags::T openFlags);

	// TODO Move this elsewhere when we have a FileDevice
	// We need to be able to create different files for different platforms, functionality, etc
	static CrFileSharedHandle Create(const char* filePath, FileOpenFlags::T openFlags);
};

CrFileSharedHandle Create(const CrPath& filePath, FileOpenFlags::T openFlags);