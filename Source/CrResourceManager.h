#pragma once

#include "Core/CrCoreForwardDeclarations.h"

class CrShader;

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class CrImage;
using CrImageHandle = CrSharedPtr<CrImage>;

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class CrResourceManager
{
public:

	static CrRenderModelSharedHandle LoadModel(const CrPathString& filePath);

	static CrPathString GetFullResourcePath(const CrPathString& relativePath);

	// TODO Also allow image loading to take a data pointer, so we can do the upload directly via the map
	static CrImageHandle LoadImageFromDisk(const CrPathString& filePath);
};
