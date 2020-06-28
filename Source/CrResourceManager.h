#pragma once

#pragma warning(push, 0)

#include "ddspp.h"

#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#pragma warning(pop)

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

	static void ReadTextFile(const CrPath& absolutePath, CrString& text);

	static void ReadTextFile(const CrPath& absolutePath, CrVector<char>& text);

	static void ReadBinaryFile(const CrPath& absolutePath, CrVector<unsigned char>& bytes);

	// TODO Change data to some image container
	// Also allow image loading to take a data pointer, so we can do the upload
	// directly via the map
	static void LoadImageFromDisk(CrImageHandle& image, const CrPath& relativePath);

private:

	static void LoadMesh(CrMeshSharedHandle& renderMesh, const aiMesh* aiMesh);

	static bool LoadMaterial(CrMaterialSharedHandle&, const aiMaterial* material, const CrPath& relativePath);

};
