#pragma once

#include "ICrModelDecoder.h"

class CrRenderMesh;
using CrRenderMeshHandle = CrSharedPtr<CrRenderMesh>;

class CrMaterial;
using CrMaterialHandle = CrIntrusivePtr<CrMaterial>;

struct aiScene;
struct aiMesh;
struct aiMaterial;
template<typename TReal> class aiMatrix4x4t;
typedef aiMatrix4x4t<float> aiMatrix4x4;

class CrModelDecoderASSIMP final : public ICrModelDecoder
{
public:

	virtual CrRenderModelHandle Decode(const CrFileHandle& file) override;

	static CrRenderMeshHandle LoadMesh(const aiScene* scene, const aiMesh* mesh, const aiMatrix4x4& transform);

	static CrMaterialHandle LoadMaterial(const aiMaterial* material, const CrPath& relativePath);
};