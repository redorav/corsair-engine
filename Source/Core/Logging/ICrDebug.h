#pragma once

#include "Core/CrMacros.h"

#include <sstream>

class ICrDebug
{
public:
	virtual void Log(const char* file, unsigned long line, const char* func, const char* format...) const = 0;

	virtual void AssertMsg(bool condition, const char* file, unsigned long line, const char* func, const char* format...) const = 0;

private:

	void AssertMsg(const char* message) const; // Disallow ugly implicit conversions between const char* and bool
};

// We create it in the platform-specific code, which is why we extern it here
extern const ICrDebug& Debug;

#define CrLog(format, ...) Debug.Log(__FILE__, __LINE__, __func__, format, __VA_ARGS__)

#define ERROR_LOG
#define WARNING_LOG
#define INFO_LOG

#if defined(ERROR_LOG)
#define CrLogError(s) CrLog("[Error] %s", s)
#else
#define CrLogError(s)
#endif

#if defined(WARNING_LOG)
#define CrLogWarning(s) CrLog("[Warning] %s", s)
#else
#define CrLogWarning(s)
#endif

#if defined(INFO_LOG)
#define CrLogInfo(s) CrLog("[Info] %s", s)
#endif

#define CrAssertMsg(condition, message, ...) Debug.AssertMsg((condition), __FILE__, __LINE__, __func__, (message), __VA_ARGS__)

#define CrAssert(condition)	CrAssertMsg((condition), "")