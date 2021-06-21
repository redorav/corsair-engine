#pragma once

#include "Core/FileSystem/ICrFile.h"

#include "Core/CrCoreForwardDeclarations.h"

typedef void* HANDLE;

class CrFileWindows final : public ICrFile
{
public:

	CrFileWindows(const char* filePath, FileOpenFlags::T openFlags, HANDLE fileHandle, uint64_t fileSize);

	~CrFileWindows();

	virtual size_t Read(void* memory, size_t bytes) const override;

	virtual size_t Write(void* memory, size_t bytes) const override;

	virtual void Seek(SeekOrigin::T seekOrigin, int64_t byteOffset) override;

	virtual void Rewind() override;

	virtual uint64_t GetSize() const override;

private:

	HANDLE m_fileHandle = nullptr;

	uint64_t m_fileSize = 0;
};