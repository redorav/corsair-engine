#pragma once

#include "Core/Logging/ICrDebug.h"

class CrDebugWindows final : public ICrDebug
{
public:

	CrDebugWindows();

	virtual void Log(const char* file, unsigned long line, const char* func, const char* format...) const override;

	virtual void AssertMsg(bool condition, const char* file, unsigned long line, const char* func, const char* format...) const override;

	virtual void WaitForDebugger() const override;

	virtual void PrintCurrentProcessMemory(const char* file, unsigned long line, const char* func, const char* format...) const override;
};