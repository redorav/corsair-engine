#include "Core/CrCore_pch.h"

#include "Core/FileSystem/CrFixedPath.h"

void CrFixedPath::Normalize()
{
	size_t separatorPosition = m_pathString.find_first_of('\\');

	while (separatorPosition != m_pathString.npos)
	{
		m_pathString[separatorPosition] = '/';
		separatorPosition = m_pathString.find_first_of('\\');
	}
}
