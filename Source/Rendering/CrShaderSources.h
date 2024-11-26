#pragma once

#include "Core/Containers/CrHashMap.h"
#include "Core/String/CrString.h"
#include "Core/FileSystem/CrFixedPath.h"
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

	static void Initialize();

	static void Deinitialize();

	const CrString& GetUbershaderSource() const;

	const CrFixedPath& GetUbershaderTempDirectory() const;

	const CrHash GetUbershaderHash() const;

private:

	CrShaderSources();

	CrHashMap<CrString, CrFixedPath> m_shaderPaths;

	CrHashMap<CrString, CrString> m_shaderSources;

	CrHashMap<CrString, CrString> m_ubershaderSources;

	// Path to the temp folder for built ubershaders
	CrFixedPath m_ubershaderTempDirectory;

	// Ubershader source with resolved includes
	CrString m_resolvedUbershaderSource;

	// Ubershader source stripped of text elements that cannot affect
	// the final binary (in order to hash it)
	CrString m_hashableUbershaderSource;

	// Ubershader hash computed with current sources (used to determine whether shaders in cache are usable)
	CrHash m_ubershaderHash;
};

extern CrShaderSources* ShaderSources;