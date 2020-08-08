#pragma once

#include "ICrFile.h"

#include "Core/CrCoreForwardDeclarations.h"

#include <cstdio>

class CrFileANSI final : public ICrFile
{
public:

	CrFileANSI(const char* filePath, FileOpenFlags::T openFlags);

	~CrFileANSI();

	virtual size_t Read(void* memory, size_t bytes) const override;

	virtual void Seek(SeekOrigin::T seekOrigin, int64_t byteOffset) override;

	virtual void Rewind() override;

	virtual uint64_t GetSize() const override;

private:

	FILE* m_file = nullptr;

	uint64_t m_fileSize;
};