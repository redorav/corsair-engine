#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/String/CrFixedString.h"

class CrPath
{
public:

	CrPath() {}

	CrPath(const char* path) { m_pathString = path; Normalize(); }

	CrPath(const char* path, size_t count) { m_pathString = CrFixedString512(path, count); Normalize(); }

	CrPath(const char* path, size_t offset, size_t count) { m_pathString = CrFixedString512(path + offset, path + offset + count); Normalize(); }

	CrPath(const CrString& path) : CrPath(path.c_str()) {}

	CrPath(const CrPath& path, size_t count) : CrPath(path.c_str(), count) {}

	CrPath(const CrPath& path, size_t offset, size_t count) : CrPath(path.c_str(), offset, count) {}

	const char* c_str() const { return m_pathString.c_str(); }

	CrPath extension() const
	{
		size_t lastDot = m_pathString.find_last_of(".");
		if (lastDot != m_pathString.npos)
		{
			return CrPath(*this, lastDot, m_pathString.length() - lastDot);
		}
		else
		{
			return CrPath();
		}
	}

	CrPath filename() const
	{
		size_t lastSeparator = m_pathString.find_last_of("/");
		return CrPath(*this, lastSeparator + 1, m_pathString.length() - (lastSeparator + 1));
	}

	bool has_extension() const
	{
		return m_pathString.find_last_of(".") != m_pathString.npos;
	}

	CrPath parent_path() const
	{
		return CrPath(*this, m_pathString.find_last_of("/"));
	}

	CrPath& remove_filename()
	{
		size_t lastSeparator = m_pathString.find_last_of("/");
		if (lastSeparator != m_pathString.length() - 1)
		{
			m_pathString.resize(lastSeparator + 1); // Include the separator
		}

		return *this;
	}

	CrPath& replace_extension(const char* extension)
	{
		bool nonEmptyString = extension && extension[0] != '\0';

		size_t lastDot = m_pathString.find_last_of(".");
		if (lastDot != m_pathString.npos)
		{
			m_pathString.resize(lastDot + nonEmptyString); // Include the dot if valid extension
			m_pathString += extension;
		}

		return *this;
	}

	CrPath operator / (const char* str)
	{
		CrPath newPath = *this;
		newPath.AddTrailingSeparator();
		newPath.m_pathString += str;
		return newPath;
	}

	CrPath operator / (const CrPath& path)
	{
		return *this / path.c_str();
	}

	CrPath& operator /= (const char* str)
	{
		AddTrailingSeparator();
		m_pathString += str;
		return *this;
	}

private:

	void AddTrailingSeparator()
	{
		// Need to check for empty or back will return invalid memory
		if (m_pathString.empty() || m_pathString.back() != '/')
		{
			m_pathString += "/";
		}
	}

	void Normalize();

	CrFixedString512 m_pathString;
};