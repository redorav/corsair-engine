#pragma once

#include "ICrModelDecoder.h"

class CrMesh;
using CrMeshSharedHandle = CrSharedPtr<CrMesh>;

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;

struct aiScene;
struct aiMesh;
struct aiMaterial;

class CrModelDecoderASSIMP final : public ICrModelDecoder
{
public:
	virtual CrRenderModelSharedHandle Decode(const CrFileSharedHandle& file) override;

private:
	void LoadMesh(CrMeshSharedHandle& renderMesh, const aiMesh* mesh);
	bool LoadMaterial(CrMaterialSharedHandle&, const aiMaterial* material, const CrPath& relativePath);
};