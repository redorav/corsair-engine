#pragma once

#include "ICrModelDecoder.h"

class CrRenderMesh;
using CrRenderMeshSharedHandle = CrSharedPtr<CrRenderMesh>;

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;

struct aiScene;
struct aiMesh;
struct aiMaterial;
template<typename TReal> class aiMatrix4x4t;
typedef aiMatrix4x4t<float> aiMatrix4x4;

class CrModelDecoderASSIMP final : public ICrModelDecoder
{
public:

	virtual CrRenderModelSharedHandle Decode(const CrFileSharedHandle& file) override;

	static CrRenderMeshSharedHandle LoadMesh(const aiScene* scene, const aiMesh* mesh, const aiMatrix4x4& transform);

	static CrMaterialSharedHandle LoadMaterial(const aiMaterial* material, const CrPath& relativePath);
};