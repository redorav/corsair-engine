#include "CrCore_pch.h"

#include "CrStringUtilities.h"

#include "Core/String/CrString.h"
#include "Core/Containers/CrVector.h"

void CrStringUtilities::EraseAll(CrString& input, char needle)
{
	size_t position = input.find(needle);
	while (position != input.npos)
	{
		input.erase(position, 1);
		position = input.find(needle);
	}
}

void CrStringUtilities::EraseAll(CrString& input, const CrString& needle)
{
	size_t position = input.find(needle);
	size_t needleSize = needle.size();
	while (position != input.npos)
	{
		input.erase(position, needleSize);
		position = input.find(needle);
	}
}

void CrStringUtilities::SplitLines(CrVector<CrString>& lines, const CrString& input)
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