#pragma once

#include "ICrModelDecoder.h"

class CrRenderMesh;
using CrRenderMeshSharedHandle = CrSharedPtr<CrRenderMesh>;

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

	CrRenderMeshSharedHandle LoadMesh(const aiMesh* mesh);

	CrMaterialSharedHandle LoadMaterial(const aiMaterial* material, const CrPath& relativePath);
};