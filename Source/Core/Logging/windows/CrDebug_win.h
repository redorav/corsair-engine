#pragma once

#include "Core/Logging/ICrDebug.h"

class CrDebug : public ICrDebug
{
public:

	CrDebug();

	virtual void Log(const char* file, unsigned long line, const char* func, const char* format...) const override;

	virtual void AssertMsg(bool condition, const char* file, unsigned long line, const char* func, const char* format...) const override;
};