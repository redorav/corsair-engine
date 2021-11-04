#pragma once

#include "Core/Containers/CrHashMap.h"
#include "Core/String/CrString.h"
#include "Core/FileSystem/CrPath.h"
#include "Core/CrHash.h"

// Stores all the sources for our shaders (on a desktop development build)
// so that we can recompile them as necessary. Also has the ability to
// resolve includes. The reason we want to load the shader source is that
// we need to hash the incoming code easily, to determine whether a shader
// needs to be recompiled or not.
// 
// TODO File watchers for fast reload. These file watchers will notify other
// systems that their sources have changed and need recompilation
class CrShaderSources
{
public:

	static CrShaderSources& Get();

	void Initialize();

	const CrString& GetUbershaderSource() const;

	const CrPath& GetUbershaderTempDirectory() const;

private:

	CrHashMap<CrString, CrPath> m_shaderPaths;

	CrHashMap<CrString, CrString> m_shaderSources;

	CrHashMap<CrString, CrString> m_ubershaderSources;

	// Path to the temp folder for built ubershaders
	CrPath m_ubershaderTempDirectory;

	// Path of the ubershader hash
	CrPath m_ubershaderHashPath;

	// Ubershader source with resolved includes
	CrString m_resolvedUbershaderSource;

	// Ubershader source stripped of text elements that cannot affect
	// the final binary (in order to hash it)
	CrString m_hashableUbershaderSource;

	// Ubershader hash (used to determine whether shaders in cache are usable)
	CrHash m_ubershaderHash;
};