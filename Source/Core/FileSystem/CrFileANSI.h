#pragma once

#include "ICrFile.h"

#include "Core/CrCoreForwardDeclarations.h"

#include <cstdio>

class CrFileANSI final : public ICrFile
{
public:

	CrFileANSI(const char* filePath, FileOpenFlags::T openFlags);

	~CrFileANSI();

	virtual size_t Read(void* memory, size_t bytes) final override;

	virtual void Seek(size_t bytes) final override;

	virtual void Rewind() final override;

	virtual uint64_t GetSize() final override;

private:

	FILE* m_file = nullptr;

	uint64_t m_fileSize;
};