#pragma once

#include "Core/FileSystem/ICrFile.h"

#include "Core/CrCoreForwardDeclarations.h"

#include <cstdio>

typedef void* HANDLE;

class CrFileWindows final : public ICrFile
{
public:

	CrFileWindows(const char* filePath, FileOpenFlags::T openFlags);

	~CrFileWindows();

	virtual size_t Read(void* memory, size_t bytes) final override;

	virtual void Seek(SeekOrigin::T seekOrigin, int64_t byteOffset) final override;

	virtual void Rewind() final override;

	virtual uint64_t GetSize() final override;

private:

	HANDLE m_fileHandle = nullptr;

	uint64_t m_fileSize = 0;
};