#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/String/CrFixedString.h"

class CrPathString
{
public:

	CrPathString() {}

	CrPathString(const char* path) { m_pathString = path; Normalize(); }

	CrPathString(const char* path, size_t count) { m_pathString = CrFixedString512(path, count); Normalize(); }

	CrPathString(const char* path, size_t offset, size_t count) { m_pathString = CrFixedString512(path + offset, path + offset + count); Normalize(); }

	CrPathString(const CrString& path) : CrPathString(path.c_str()) {}

	CrPathString(const CrPathString& path, size_t count) : CrPathString(path.c_str(), count) {}

	CrPathString(const CrPathString& path, size_t offset, size_t count) : CrPathString(path.c_str(), offset, count) {}

	const char* c_str() const { return m_pathString.c_str(); }

	CrPathString extension() const
	{
		size_t lastDot = m_pathString.find_last_of(".");
		if (lastDot != m_pathString.npos)
		{
			return CrPathString(*this, lastDot, m_pathString.length() - lastDot);
		}
		else
		{
			return CrPathString();
		}
	}

	CrPathString filename() const
	{
		size_t lastSeparator = m_pathString.find_last_of("/");
		return CrPathString(*this, lastSeparator + 1, m_pathString.length() - (lastSeparator + 1));
	}

	bool has_extension() const
	{
		return m_pathString.find_last_of(".") != m_pathString.npos;
	}

	CrPathString parent_path() const
	{
		return CrPathString(*this, m_pathString.find_last_of("/"));
	}

	CrPathString& remove_filename()
	{
		size_t lastSeparator = m_pathString.find_last_of("/");
		if (lastSeparator != m_pathString.length() - 1)
		{
			m_pathString.resize(lastSeparator + 1); // Include the separator
		}

		return *this;
	}

	CrPathString& replace_extension(const char* extension)
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

	CrPathString operator / (const char* str)
	{
		CrPathString newPath = *this;
		newPath.AddTrailingSeparator();
		newPath.m_pathString += str;
		return newPath;
	}

	CrPathString operator / (const CrPathString& path)
	{
		return *this / path.c_str();
	}

	CrPathString& operator /= (const char* str)
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