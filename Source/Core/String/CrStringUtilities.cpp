#include "Core/CrCore_pch.h"

#include "CrStringUtilities.h"

#include "Core/Containers/CrVector.h"

#include "crstl/string.h"

void CrStringUtilities::SplitLines(CrVector<crstl::string>& lines, const crstl::string& input)
{
	size_t lineStartPosition = 0;
	size_t lineEndPosition = input.find('\n');
	while (lineEndPosition != input.npos)
	{
		size_t length = lineEndPosition - lineStartPosition;
		lines.emplace_back(input.substr(lineStartPosition, length));
		lineStartPosition = lineEndPosition;
		lineEndPosition = input.find('\n', lineEndPosition + 1);
	}
}