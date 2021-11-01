#include "Core/FileSystem/CrPath.h"

void CrPath::Normalize()
{
	size_t separatorPosition = m_pathString.find_first_of('\\');

	while (separatorPosition != m_pathString.npos)
	{
		m_pathString[separatorPosition] = '/';
		separatorPosition = m_pathString.find_first_of('\\');
	}
}
