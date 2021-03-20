#pragma once

#include "ICrModelDecoder.h"
#include <stdint.h>
#include <vector>
#include <map>

#include "Rendering/CrRenderingForwardDeclarations.h"

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
	CrMeshSharedHandle LoadMesh(const tinygltf::Model* modelData, const tinygltf::Mesh* meshData, CrMaterialSharedHandle material, std::map<int, CrTextureSharedHandle>& textureTable);

	template<typename T>
	void LoadAttribute(std::vector<T>& targetBuffer, const unsigned char* sourceBuffer, size_t numComponents, int32_t componentSize, size_t componentStride)
	{
		for (size_t componentIndex = 0; componentIndex < numComponents; ++componentIndex)
		{
			memcpy(&targetBuffer[componentIndex], sourceBuffer, componentSize);
			sourceBuffer += componentStride;
		}
	}
};