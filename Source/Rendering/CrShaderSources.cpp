#include "Rendering/CrRendering_pch.h"
#include "CrShaderSources.h"

#include "Core/Containers/CrHashMap.h"
#include "Core/CrGlobalPaths.h"
#include "Core/Logging/ICrDebug.h"

#include "crstl/filesystem.h"
#include "crstl/vector.h"

CrShaderSources* ShaderSources;

static crstl::string UbershaderEntryFile = "Ubershader.hlsl";

void CrShaderSources::Initialize()
{
	CrAssert(ShaderSources == nullptr);
	ShaderSources = new CrShaderSources();
}

void CrShaderSources::Deinitialize()
{
	CrAssert(ShaderSources != nullptr);
	delete ShaderSources;
	ShaderSources = nullptr;
}

CrShaderSources::CrShaderSources()
{
	// Create ubershader temp directory
	m_ubershaderTempDirectory = CrGlobalPaths::GetTempEngineDirectory();
	m_ubershaderTempDirectory /= "Ubershader Temp";

	crstl::create_directories(m_ubershaderTempDirectory.c_str());

	// Files that contribute to ubershader building
	// Perhaps more flexible in a text file
	static const CrHashSet<crstl::string> UbershaderFiles =
	{
		"BSDF.hlsl",
		"Common.hlsl",
		"Surface.hlsl",
		UbershaderEntryFile,
	};

	const crstl::string& ShaderSourceDirectory = CrGlobalPaths::GetShaderSourceDirectory();

	// Load all the files in this directory and put them in a hashmap based on filename
	crstl::for_each_directory_entry(ShaderSourceDirectory.c_str(), false, [this](const crstl::directory_entry& entry)
	{
		if (!entry.is_directory)
		{
			CrFixedPath shaderPath = entry.directory;
			shaderPath /= entry.filename;

			if (crstl::file shaderSourceFile = crstl::file(shaderPath.c_str(), crstl::file_flags::read))
			{
				crstl::string shaderSource(crstl::ctor_no_initialize, shaderSourceFile.get_size(), shaderSourceFile.get_size());
				shaderSourceFile.read(shaderSource.data(), shaderSource.size());

				// Preprocess the shader source to remove anything that cannot
				// add anything meaningful to the final binary, such as whitespace, 
				// tabs, lines, and comments

				// Reserve an initial number of lines
				crstl::vector<crstl::string> lines;
				lines.reserve(200);

				shaderSource.split('\n', [&lines](const crstl::string_view& view)
				{
					lines.push_back(crstl::string(view));
				});

				// TODO Add a competent preprocessor here, instead of manually parsing
				crstl::vector<crstl::string> preprocessedLines;
				for (crstl::string& line : lines)
				{
					if (!line.empty())
					{
						line.erase_all(' ');
						line.erase_all('\t');
						line.erase_all('\r');
						line.erase_all('\n');

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
				if (UbershaderFiles.find(entry.filename) != UbershaderFiles.end())
				{
					for (const crstl::string& line : preprocessedLines)
					{
						m_hashableUbershaderSource += line;
					}
				}

				crstl::string filenameString = entry.filename;

				m_shaderPaths.insert(filenameString, shaderPath);
				m_shaderSources.insert(filenameString, shaderSource);
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
						const crstl::string includeFilename = m_resolvedUbershaderSource.substr(startQuote + 1, endQuote - startQuote - 1);

						const auto& includeFilenameIter = m_shaderSources.find(includeFilename);

						// If the include filename is in the cache, we may proceed to inject it
						if (includeFilenameIter != m_shaderSources.end())
						{
							const crstl::string& includedSourceFile = m_shaderSources.find(includeFilename)->second;
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

const crstl::string& CrShaderSources::GetUbershaderSource() const
{
	return m_resolvedUbershaderSource;
}

const CrFixedPath& CrShaderSources::GetUbershaderTempDirectory() const
{
	return m_ubershaderTempDirectory;
}

const CrHash CrShaderSources::GetUbershaderHash() const
{
	return m_ubershaderHash;
}