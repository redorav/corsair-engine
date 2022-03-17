#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/String/CrFixedString.h"
#include "Core/String/CrString.h"

// This class is a fixe memory replacement for std::filesystem::path. The idea is that it doesn't allocate any memory
// during all the operations at the cost of more memory at runtime. Extensions on top of this are possible, such as a
// dynamic CrPath (for storing optimally), shorter CrPaths (for extensions), etc.
class CrPath
{
public:

	static const size_t npos = (size_t) - 1;

	CrPath() {}

	CrPath(const char* path) { m_pathString = path; Normalize(); }

	CrPath(const char* path, size_t count) { m_pathString = CrFixedString512(path, count); Normalize(); }

	CrPath(const char* path, size_t offset, size_t count) { m_pathString = CrFixedString512(path + offset, path + offset + count); Normalize(); }

	CrPath(const CrString& path) : CrPath(path.c_str()) {}

	CrPath(const CrPath& path, size_t count) : CrPath(path.c_str(), count) {}

	CrPath(const CrPath& path, size_t offset, size_t count) : CrPath(path.c_str(), offset, count) {}

	template <typename CharType>
	CrPath& append_convert(const CharType* other)
	{
		m_pathString.append_convert(other); return *this;
	}

	const char* c_str() const { return m_pathString.c_str(); }

	int compare(const char* s)
	{
		return m_pathString.compare(s);
	}

	int comparei(const char* s)
	{
		return m_pathString.comparei(s);
	}

	CrPath extension() const
	{
		size_t lastDot;
		if (has_extension_internal(lastDot))
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

	size_t find_last_of(char c) const
	{
		return m_pathString.find_last_of(c);
	}

	size_t find_last_of(const char* s) const
	{
		return m_pathString.find_last_of(s);
	}

	bool has_extension() const
	{
		size_t lastDot;
		return has_extension_internal(lastDot);
	}

	CrPath parent_path() const
	{
		size_t lastSeparator = m_pathString.find_last_of("/");

		if (lastSeparator != m_pathString.npos)
		{
			return CrPath(*this, lastSeparator);
		}
		else
		{
			return CrPath();
		}
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
	// Then replacement is appended as if by operator += (replacement).
	CrPath& replace_extension(const char* extension)
	{
		size_t lastDot;
		if (has_extension_internal(lastDot))
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

	void resize(size_t n)
	{
		m_pathString.resize(n);
	}

	void resize(size_t n, char c)
	{
		m_pathString.resize(n, c);
	}

	bool operator == (const char* path) const
	{
		return m_pathString == path;
	}

	bool operator == (const CrPath& path) const
	{
		return m_pathString == path.m_pathString;
	}

	bool operator != (const char* path) const
	{
		return m_pathString != path;
	}

	bool operator != (const CrPath& path) const
	{
		return m_pathString != path.m_pathString;
	}

	CrPath operator + (const char* str) const
	{
		CrPath newPath = *this;
		newPath.m_pathString += str;
		return newPath;
	}

	CrPath operator + (const CrPath& path) const
	{
		return *this + path.c_str();
	}

	CrPath& operator += (const char* str)
	{
		m_pathString += str;
		return *this;
	}

	CrPath operator / (const char* str) const
	{
		CrPath newPath = *this;
		newPath.AddTrailingSeparator();
		newPath.m_pathString += str;
		return newPath;
	}

	CrPath operator / (const CrPath& path) const
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

	// https://en.cppreference.com/w/cpp/filesystem/path/extension
	// If the filename() component of the generic - format path contains a period(.), and is not one of the special filesystem elements dot 
	// or dot-dot, then the extension is the substring beginning at the rightmost period (including the period) and until the end of the pathname.
	// If the first character in the filename is a period, that period is ignored (a filename like ".profile" is not treated as an extension)
	// If the pathname is either . or .., or if filename() does not contain the . character, then empty path is returned.
	bool has_extension_internal(size_t& lastDot) const
	{
		lastDot = m_pathString.find_last_of(".");
		size_t lastSeparator = m_pathString.find_last_of("/");

		if (lastDot != m_pathString.npos && // If there is a dot
			lastDot > 0 && // The dot is not at the beginning of the path
			m_pathString[lastDot - 1] != '/' && // And the previous character is neither a / nor a . (special characters)
			m_pathString[lastDot - 1] != '.')
		{
			if (lastSeparator != m_pathString.npos)
			{
				return lastDot > lastSeparator;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return false;
		}
	}

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