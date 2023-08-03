#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/String/CrFixedString.h"
#include "Core/String/CrString.h"

// This class is a fixe memory replacement for std::filesystem::path. The idea is that it doesn't allocate any memory
// during all the operations at the cost of more memory at runtime. Extensions on top of this are possible, such as a
// dynamic CrPath (for storing optimally), shorter CrPaths (for extensions), etc.
class CrFixedPath
{
public:

	static const size_t npos = (size_t) - 1;

	CrFixedPath() {}

	CrFixedPath(const char* path) { m_pathString = path; Normalize(); }

	CrFixedPath(const char* path, size_t count) { m_pathString = CrFixedString512(path, count); Normalize(); }

	CrFixedPath(const char* path, size_t offset, size_t count) { m_pathString = CrFixedString512(path + offset, path + offset + count); Normalize(); }

	CrFixedPath(const CrString& path) : CrFixedPath(path.c_str()) {}

	CrFixedPath(const CrFixedPath& path, size_t count) : CrFixedPath(path.c_str(), count) {}

	CrFixedPath(const CrFixedPath& path, size_t offset, size_t count) : CrFixedPath(path.c_str(), offset, count) {}

	template <typename CharType>
	CrFixedPath& append_convert(const CharType* other)
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

	bool empty() const
	{
		return m_pathString.empty();
	}

	CrFixedPath extension() const
	{
		size_t lastDot;
		if (has_extension_internal(lastDot))
		{
			return CrFixedPath(*this, lastDot, m_pathString.length() - lastDot);
		}
		else
		{
			return CrFixedPath();
		}
	}

	CrFixedPath filename() const
	{
		size_t lastSeparator = m_pathString.find_last_of("/");
		return CrFixedPath(*this, lastSeparator + 1, m_pathString.length() - (lastSeparator + 1));
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

	CrFixedPath parent_path() const
	{
		size_t lastSeparator = m_pathString.find_last_of("/");

		if (lastSeparator != m_pathString.npos)
		{
			return CrFixedPath(*this, lastSeparator);
		}
		else
		{
			return CrFixedPath();
		}
	}

	CrFixedPath& remove_filename()
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
	CrFixedPath& replace_extension(const char* extension)
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

	bool operator == (const CrFixedPath& path) const
	{
		return m_pathString == path.m_pathString;
	}

	bool operator != (const char* path) const
	{
		return m_pathString != path;
	}

	bool operator != (const CrFixedPath& path) const
	{
		return m_pathString != path.m_pathString;
	}

	CrFixedPath operator + (const char* str) const
	{
		CrFixedPath newPath = *this;
		newPath.m_pathString += str;
		return newPath;
	}

	CrFixedPath operator + (const CrFixedPath& path) const
	{
		CrFixedPath newPath = *this;
		newPath.m_pathString += path.m_pathString;
		return newPath;
	}

	CrFixedPath& operator += (const char* str)
	{
		m_pathString += str;
		return *this;
	}

	CrFixedPath operator / (const char* str) const
	{
		CrFixedPath newPath = *this;
		newPath.AddTrailingSeparator();
		newPath.m_pathString += str;
		return newPath;
	}

	CrFixedPath operator / (const CrFixedPath& path) const
	{
		CrFixedPath newPath = *this;
		newPath.AddTrailingSeparator();
		newPath.m_pathString += path.m_pathString;
		return newPath;
	}

	CrFixedPath& operator /= (const char* str)
	{
		AddTrailingSeparator();
		m_pathString += str;
		return *this;
	}

	CrFixedPath& operator /= (const CrFixedPath& path)
	{
		AddTrailingSeparator();
		m_pathString += path.m_pathString;
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

		if (lastDot != m_pathString.npos && // If there is a dot
			lastDot > 0 && // The dot is not at the beginning of the path
			m_pathString[lastDot - 1] != '/' && // And the previous character is neither a / nor a . (special characters)
			m_pathString[lastDot - 1] != '.')
		{
			size_t lastSeparator = m_pathString.find_last_of("/");

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
		if (!m_pathString.empty())
		{
			char lastCharacter = m_pathString.back();

			if (lastCharacter != '/' || lastCharacter == '.')
			{
				m_pathString += "/";
			}
		}
	}

	void Normalize();

	CrFixedString512 m_pathString;
};