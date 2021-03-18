#pragma once

#include "ICrModelDecoder.h"

class CrMesh;
using CrMeshSharedHandle = CrSharedPtr<CrMesh>;

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;

namespace tinygltf
{
	class Model;
	struct Mesh;
}

class CrModelDecoderGLTF final : public ICrModelDecoder
{
public:
	virtual CrRenderModelSharedHandle Decode(const CrFileSharedHandle& file) override;

private:
	CrMeshSharedHandle LoadMesh(const tinygltf::Model* modelData, const tinygltf::Mesh* meshData);
};