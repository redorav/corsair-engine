#pragma once

#include "Core/String/CrFixedString.h"

#include "stdint.h"

struct CrProcessDescriptor
{
	CrFixedString2048 commandLine;
};

enum class CrProcessResult
{
	Undefined,
	Success,
	Error
};

class CrProcess
{
public:
	
	CrProcess(const CrProcessDescriptor& processDescriptor);

	~CrProcess();

	void Wait(uint64_t timeoutMilliseconds);

	void Wait();

	void ReadStdOut(char* buffer, size_t bufferSize);

	CrProcessResult GetResult() const
	{
		return result;
	}

	int GetReturnValue() const;

private:

	CrProcess()
		: result(CrProcessResult::Undefined)
		, returnValue(-2147483647 - 1)
		, hProcess(nullptr)
		, hStdInput(nullptr)
	{}

	CrProcessResult result;

	int returnValue;

	void* hProcess;

	void* hStdInput; // Input handle that we read from
};

inline void CrProcess::Wait()
{
	Wait(0xffffffffffffffff);
}

inline int CrProcess::GetReturnValue() const
{
	return returnValue;
}