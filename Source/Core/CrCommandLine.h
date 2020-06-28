#pragma once

#include "Core/String/CrString.h"
#include "Core/Containers/CrVector.h"
#include "Core/Containers/CrHashMap.h"
#include "Core/String/CrString.h"

#include "argh.h"

using CrCommandLine = argh::parser;

namespace crcore
{
	extern CrCommandLine CommandLine;
}