#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Image/CrImageForwardDeclarations.h"

class CrShader;

class CrResourceManager
{
public:

	static CrRenderModelHandle LoadModel(const CrFixedPath& filePath);

	static CrFixedPath GetFullResourcePath(const CrFixedPath& relativePath);

	// TODO Also allow image loading to take a data pointer, so we can do the upload directly via the map
	static CrImageHandle LoadImageFromDisk(const CrFixedPath& filePath);

	static void SaveImageToDisk(const CrImageHandle& image, const CrFixedPath& fullPath);
};
