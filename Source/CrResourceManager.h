#pragma once

#include "Core/CrCoreForwardDeclarations.h"

class CrShader;
struct aiScene;
struct aiMesh;
struct aiMaterial;

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class CrMesh;
using CrMeshSharedHandle = CrSharedPtr<CrMesh>;

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;

class CrImage;
using CrImageHandle = CrSharedPtr<CrImage>;

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class CrResourceManager
{
public:

	static bool LoadModel(CrRenderModelSharedHandle& renderModel, const CrPath& filePath);

	static CrPath GetFullResourcePath(const CrPath& relativePath);

	// TODO Also allow image loading to take a data pointer, so we can do the upload directly via the map
	static CrImageHandle LoadImageFromDisk(const CrPath& relativePath);

private:

	static void LoadMesh(CrMeshSharedHandle& renderMesh, const aiMesh* aiMesh);

	static bool LoadMaterial(CrMaterialSharedHandle&, const aiMaterial* material, const CrPath& relativePath);

};
