#include "CrRendering_pch.h"
#include "CrShaderSources.h"

#include "Core/Containers/CrVector.h"
#include "Core/Containers/CrHashSet.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/Function/CrFixedFunction.h"
#include "Core/CrGlobalPaths.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/String/CrStringUtilities.h"

static CrShaderSources ShaderSources;

static CrString UbershaderEntryFile = "Ubershader.hlsl";

CrShaderSources& CrShaderSources::Get()
{
	return ShaderSources;
}

void CrShaderSources::Initialize()
{
	// Create ubershader temp directory
	m_ubershaderTempDirectory = CrGlobalPaths::GetTempEngineDirectory();
	m_ubershaderTempDirectory /= "Ubershader Temp";

	ICrFile::CreateDirectories(m_ubershaderTempDirectory.c_str());

	// Files that contribute to ubershader building
	// Perhaps more flexible in a text file
	static const CrHashSet<CrString> UbershaderFiles =
	{
		"Brdf.hlsl",
		"Common.hlsl",
		"Surface.hlsl",
		UbershaderEntryFile,
	};

	const CrString& ShaderSourceDirectory = CrGlobalPaths::GetShaderSourceDirectory();

	// Load all the files in this directory and put them in a hashmap based on filename
	ICrFile::ForEachDirectoryEntry(ShaderSourceDirectory.c_str(), false, [this](const CrDirectoryEntry& entry)
	{
		if (!entry.isDirectory)
		{
			CrPath shaderPath = entry.directory;
			shaderPath /= entry.filename.c_str();

			CrFileSharedHandle shaderSourceFile = ICrFile::OpenFile(shaderPath.c_str(), FileOpenFlags::Read);

			if (shaderSourceFile)
			{
				CrString shaderSource(CrStringNoInitialize(), shaderSourceFile->GetSize());
				shaderSource.force_size(shaderSourceFile->GetSize());
				shaderSourceFile->Read(shaderSource.data(), shaderSource.size());

				// Preprocess the shader source to remove anything that cannot
				// add anything meaningful to the final binary, such as whitespace, 
				// tabs, lines, and comments

				CrVector<CrString> lines;
				CrStringUtilities::SplitLines(lines, shaderSource);

				// TODO Add a competent preprocessor here, instead of manually parsing
				CrVector<CrString> preprocessedLines;
				for (CrString& line : lines)
				{
					if (!line.empty())
					{
						CrStringUtilities::EraseAll(line, ' ');
						CrStringUtilities::EraseAll(line, '\t');
						CrStringUtilities::EraseAll(line, '\r');
						CrStringUtilities::EraseAll(line, '\n');

						// Check empty again after we've removed everything
						if (!line.empty())
						{
							char firstChar = line[0];
							if (firstChar != '/')
							{
								preprocessedLines.push_back(line);
							}
						}
					}
				}

				// This source is in a good format for hashing, as any spurious spaces,
				// tabs, formatting of any sort won't affect the hash and therefore not
				// trigger any extra compilations
				// Only consider ubershader files for the ubershader hash
				if (UbershaderFiles.find(entry.filename.c_str()) != UbershaderFiles.end())
				{
					for (const CrString& line : preprocessedLines)
					{
						m_hashableUbershaderSource += line;
					}
				}

				CrString filenameString = entry.filename.c_str();

				m_shaderPaths.insert({ filenameString, shaderPath });
				m_shaderSources.insert({ filenameString, shaderSource });
			}
		}

		return true;
	});

	m_ubershaderHash = CrHash(m_hashableUbershaderSource.c_str(), m_hashableUbershaderSource.length());

	CrLog("Ubershader - Computed Hash: %ull", m_ubershaderHash.GetHash());

	// Resolve ubershader includes upfront. The only thing that really changes an ubershader
	// is defines that we can either pass in from the compiler or inject at the top of the file
	// TODO: Replace with a preprocessor
	{
		bool success = true;

		// Prime the ubershader source with the entry file
		const auto ubershaderEntryIterator = m_shaderSources.find(UbershaderEntryFile);

		if (ubershaderEntryIterator != m_shaderSources.end())
		{
			m_resolvedUbershaderSource = ubershaderEntryIterator->second;

			// For every include declaration, substitute with the actual source
			size_t includePosition = m_resolvedUbershaderSource.find("#include");

			while (includePosition != m_resolvedUbershaderSource.npos)
			{
				// Find the end of the line
				size_t endLine = m_resolvedUbershaderSource.find('\n', includePosition);

				// If there is no end line, we could be at the end of the file. Always
				// assume there is an endLine
				if (endLine == m_resolvedUbershaderSource.npos)
				{
					endLine = m_resolvedUbershaderSource.length();
				}

				bool includeSuccess = false;

				// The include declaration may be malformed so be a little defensive about it
				// We need to get the shader inside the quotes, look it up in the source cache,
				// and replace the include declaration with the block of code behind it. Includes
				// may be repeated, which is fine
				size_t startQuote = m_resolvedUbershaderSource.find('"', includePosition);

				if (startQuote != m_resolvedUbershaderSource.npos)
				{
					size_t endQuote = m_resolvedUbershaderSource.find('"', startQuote + 1);

					if (endQuote != m_resolvedUbershaderSource.npos)
					{
						// Get the filename of the include
						const CrString includeFilename = m_resolvedUbershaderSource.substr(startQuote + 1, endQuote - startQuote - 1);

						const auto& includeFilenameIter = m_shaderSources.find(includeFilename);

						// If the include filename is in the cache, we may proceed to inject it
						if (includeFilenameIter != m_shaderSources.end())
						{
							const CrString& includedSourceFile = m_shaderSources.find(includeFilename)->second;
							m_resolvedUbershaderSource.replace(includePosition, endLine - includePosition, includedSourceFile);
							includeSuccess = true;
						}
						else
						{
							CrLog("Include file %s not found", includeFilename.c_str());
						}
					}
				}

				success &= includeSuccess;
				includePosition = m_resolvedUbershaderSource.find("#include", includePosition + 1);
			}

			if (!success)
			{
				CrLog("Error parsing include declaration. Could not resolve ubershader include");
			}
		}
		else
		{
			CrLog("Ubershader entry file not found in sources");
		}
	}
}

const CrString& CrShaderSources::GetUbershaderSource() const
{
	return m_resolvedUbershaderSource;
}

const CrPath& CrShaderSources::GetUbershaderTempDirectory() const
{
	return m_ubershaderTempDirectory;
}

const CrHash CrShaderSources::GetUbershaderHash() const
{
	return m_ubershaderHash;
}
