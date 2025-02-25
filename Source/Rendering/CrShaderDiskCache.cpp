#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrShaderDiskCache.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/ICrShader.h"

#include "Core/CrHash.h"
#include "Core/Streams/CrFileStream.h"
#include "Core/Function/CrFixedFunction.h"

CrShaderDiskCache::CrShaderDiskCache(const CrFixedPath& cachePath, const char* hashFilename, CrHash currentHash)
	: m_cachePath(cachePath)
{
	// We store the hash file at the same level as the folder
	CrFixedPath hashPath = cachePath.parent_path() / hashFilename;
	CrHash storedHash;

	bool clearCache = true;

	// If we have a hash, check whether we've produced the same hash. If so, the cache is valid
	// If not, create the hash file and store it
	if (crstl::file hashFile = crstl::file(hashPath.c_str(), crstl::file_flags::read))
	{
		hashFile.read((void*)&storedHash, sizeof(storedHash));

		// If we have the same hash, the cache is valid and we don't need to do anything about it
		// If not, delete the contents of the folder and start again
		if (storedHash == currentHash)
		{
			clearCache = false;
		}
	}

	if (crstl::exists(cachePath.c_str()))
	{
		if (clearCache)
		{
			crstl::for_each_directory_entry(cachePath.c_str(), true, [](const crstl::directory_entry& entry)
			{
				if (!entry.is_directory)
				{
					CrFixedPath filePath = entry.directory;
					filePath /= entry.filename;
					crstl::delete_file(filePath.c_str());
				}

				return true;
			});
		}
	}
	else
	{
		// Make sure directory exists
		crstl::create_directories(cachePath.c_str());
	}

	if (clearCache)
	{
		crstl::file hashFile = crstl::file(hashPath.c_str(), crstl::file_flags::force_create | crstl::file_flags::write);
		hashFile.write((void*)&currentHash, sizeof(currentHash));
	}
}

CrFixedPath CrShaderDiskCache::CreateCachedFilePath(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const
{
	CrString hashString = CrString(hash.GetHash());
	CrFixedPath cachedBytecodePath = m_cachePath;
	cachedBytecodePath /= hashString.c_str();
	cachedBytecodePath.replace_extension(CrShaderManager::GetShaderBytecodeExtension(graphicsApi));
	return cachedBytecodePath;
}

CrShaderBytecodeHandle CrShaderDiskCache::LoadFromCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi) const
{
	CrFixedPath cachedBytecodePath = CreateCachedFilePath(hash, graphicsApi);
	
	CrReadFileStream cachedBytecodeFile(cachedBytecodePath.c_str());
	
	if (cachedBytecodeFile.GetFile())
	{
		CrShaderBytecodeHandle shaderBytecode(new CrShaderBytecode());
		cachedBytecodeFile << *shaderBytecode.get();
		return shaderBytecode;
	}
	else
	{
		return nullptr;
	}
}

void CrShaderDiskCache::SaveToCache(const CrHash& hash, cr3d::GraphicsApi::T graphicsApi, const CrShaderBytecodeHandle& bytecode) const
{
	if (bytecode)
	{
		CrFixedPath cachedBytecodePath = CreateCachedFilePath(hash, graphicsApi);

		CrWriteFileStream writeFileStream(cachedBytecodePath.c_str());
		writeFileStream << *bytecode.get();
	}
}
