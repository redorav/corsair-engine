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
		size_t lastDot = m_pathString.find_last_of(".");

		return lastDot != m_pathString.npos && // If there is a dot
		       lastDot != 0 && // And that last dot is not at the beginning of the path
		       lastDot > 1 && // And the length is greater than 1
		       m_pathString[lastDot - 1] != '/' &&
		       m_pathString[lastDot - 1] != '.';
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

	// https://en.cppreference.com/w/cpp/filesystem/path/replace_extension
	// Replaces the extension with replacement or removes it when the default value of replacement is used.
	// Firstly, if this path has an extension(), it is removed from the generic-format view of the pathname.
	// Then, a dot character is appended to the generic-format view of the pathname, if replacement is not empty and does not begin with a dot character.
	// Then replacement is appended as if by operator+=(replacement).
	CrPath& replace_extension(const char* extension)
	{
		size_t lastDot = m_pathString.find_last_of(".");

		if (has_extension())
		{
			m_pathString.resize(lastDot);
		}

		if (extension && extension[0] != '\0')
		{
			if (extension[0] != '.')
			{
				m_pathString += ".";
			}

			m_pathString += extension;
		}

		return *this;
	}

	CrPath operator + (const char* str)
	{
		CrPath newPath = *this;
		newPath.m_pathString += str;
		return newPath;
	}

	CrPath operator + (const CrPath& path)
	{
		return *this + path.c_str();
	}

	CrPath& operator += (const char* str)
	{
		m_pathString += str;
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