#include "CrRendering_pch.h"

#include "CrShapeBuilder.h"
#include "CrCommonVertexLayouts.h"

#include "CrRendering.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"

CrRenderMeshSharedHandle CrShapeBuilder::CreateQuad(const CrQuadDescriptor& descriptor)
{
	// XZ Plane: Quad faces up
	// 
	//               Z
	//   A           ^           B
	//    +----------|----------+
	//    |          |          |
	//    |          |          |
	//    |          |          |
	//    |          |          |
	//    |          +-------------> X
	//    |                     |
	//    |                     |
	//    |                     |
	//    |                     |
	//    +---------------------+
	//   C                       D
	//
	// A: xyz (-1, 0,  1) uv (0, 0)
	// B: xyz ( 1, 0,  1) uv (1, 0)
	// C: xyz (-1, 0, -1) uv (0, 1)
	// D: xyz ( 1, 0, -1) uv (1, 1)

	const CrRenderDeviceSharedHandle& renderDevice = ICrRenderSystem::GetRenderDevice();

	CrRenderMeshSharedHandle quadMesh = CrRenderMeshSharedHandle(new CrRenderMesh());

	// Compute number of vertices
	uint32_t vertexCountX = 2 + descriptor.subdivisionX;
	uint32_t vertexCountY = 2 + descriptor.subdivisionY;
	uint32_t vertexCount = vertexCountX * vertexCountY;

	uint32_t quadCountX = vertexCountX - 1;
	uint32_t quadCountY = vertexCountY - 1;

	// Compute number of indices (number of quads * 2 triangles per quad * 3 vertices per triangle)
	uint32_t indexCount = quadCountX * quadCountY * 2 * 3;

	CrVertexBufferHandle positionBuffer = renderDevice->CreateVertexBuffer(cr3d::MemoryAccess::GPUOnlyRead, PositionVertexDescriptor, vertexCount);
	CrVertexBufferHandle additionalBuffer = renderDevice->CreateVertexBuffer(cr3d::MemoryAccess::GPUOnlyRead, AdditionalVertexDescriptor, vertexCount);

	float4 colorAsByte = descriptor.color * 255.0f;

	ComplexVertexPosition* positionData = (ComplexVertexPosition*)renderDevice->BeginBufferUpload(positionBuffer->GetHardwareBuffer());
	ComplexVertexAdditional* additionalData = (ComplexVertexAdditional*)renderDevice->BeginBufferUpload(additionalBuffer->GetHardwareBuffer());
	{
		float dx = 1.0f / quadCountX;
		float dy = 1.0f / quadCountY;

		uint32_t currentVertex = 0;

		float fy = 0.0f;
		for (uint32_t y = 0; y < vertexCountY; ++y, fy += dy)
		{
			float fx = 0.0f;
			for (uint32_t x = 0; x < vertexCountX; ++x, fx += dx)
			{
				positionData[currentVertex].position = { (half)(fx * 2.0f - 1.0f), 0.0_h, (half)((1.0f - fy) * 2.0f - 1.0f) };

				additionalData[currentVertex].uv = { (half)fx, (half)fy };

				additionalData[currentVertex].normal = { 0, 255, 0 };

				additionalData[currentVertex].tangent = { 255, 0, 0 };

				additionalData[currentVertex].color = { (uint8_t)colorAsByte.r, (uint8_t)colorAsByte.g, (uint8_t)colorAsByte.b, (uint8_t)colorAsByte.a };

				currentVertex++;
			}
		}

		CrAssertMsg(vertexCount == currentVertex, "Mismatch in number of vertices");
	}
	renderDevice->EndBufferUpload(positionBuffer->GetHardwareBuffer());
	renderDevice->EndBufferUpload(additionalBuffer->GetHardwareBuffer());

	CrIndexBufferHandle indexBuffer = renderDevice->CreateIndexBuffer(cr3d::MemoryAccess::GPUOnlyRead, cr3d::DataFormat::R16_Uint, indexCount);
	uint16_t* indexData = (uint16_t*)renderDevice->BeginBufferUpload(indexBuffer->GetHardwareBuffer());
	{
		uint32_t currentIndex = 0;

		for (uint32_t y = 0; y < quadCountY; ++y)
		{
			for (uint32_t x = 0; x < quadCountX; ++x)
			{
				indexData[currentIndex++] = (uint16_t)(x);
				indexData[currentIndex++] = (uint16_t)(x + 1);
				indexData[currentIndex++] = (uint16_t)(x + vertexCountX);

				indexData[currentIndex++] = (uint16_t)(x + vertexCountX);
				indexData[currentIndex++] = (uint16_t)(x + 1);
				indexData[currentIndex++] = (uint16_t)(x + 1 + vertexCountX);
			}
		}

		CrAssertMsg(indexCount == currentIndex, "Mismatch in number of indices");
	}
	renderDevice->EndBufferUpload(indexBuffer->GetHardwareBuffer());

	quadMesh->AddVertexBuffer(positionBuffer);
	quadMesh->AddVertexBuffer(additionalBuffer);

	quadMesh->SetIndexBuffer(indexBuffer);

	return quadMesh;
}

CrRenderMeshSharedHandle CrShapeBuilder::CreateCube(const CrCubeDescriptor& descriptor)
{
	//     A +--------+  B
	//      /        /|
	//     /        / |
	//  C +--------+ D|
	//    |        |  |
	//    |   E    |  +  F
	//    |        | /
	//    |        |/
	//    +--------+
	//  G            H
	//
	// A: xyz (-1,  1,  1) uv (0, 0)
	// B: xyz ( 1,  1,  1) uv (1, 0)
	// C: xyz (-1,  1, -1) uv (0, 1)
	// D: xyz ( 1,  1, -1) uv (1, 1)
	// E: xyz (-1, -1,  1)
	// F: xyz ( 1, -1,  1)
	// G: xyz (-1, -1, -1)
	// H: xyz ( 1, -1, -1)

	const CrRenderDeviceSharedHandle& renderDevice = ICrRenderSystem::GetRenderDevice();

	CrRenderMeshSharedHandle sphereMesh = CrRenderMeshSharedHandle(new CrRenderMesh());

	// Compute number of vertices
	uint32_t vertexCountX = 2 + descriptor.subdivisionX;
	uint32_t vertexCountY = 2 + descriptor.subdivisionY;
	uint32_t vertexCountZ = 2 + descriptor.subdivisionZ;

	uint32_t quadCountX = vertexCountX - 1;
	uint32_t quadCountY = vertexCountY - 1;
	uint32_t quadCountZ = vertexCountZ - 1;

	uint32_t vertexCount = 2 * (vertexCountY * vertexCountZ + vertexCountX * vertexCountZ + vertexCountX * vertexCountY);

	// Number of quads per face, times 2 faces (positive and negative) * 2 triangles per quad * 3 vertices per triangle
	uint32_t indexCount = (quadCountY * quadCountZ + quadCountX * quadCountZ + quadCountX * quadCountY) * 2 * 2 * 3;

	CrVertexBufferHandle positionBuffer = renderDevice->CreateVertexBuffer(cr3d::MemoryAccess::GPUOnlyRead, PositionVertexDescriptor, vertexCount);
	CrVertexBufferHandle additionalBuffer = renderDevice->CreateVertexBuffer(cr3d::MemoryAccess::GPUOnlyRead, AdditionalVertexDescriptor, vertexCount);
	CrIndexBufferHandle indexBuffer = renderDevice->CreateIndexBuffer(cr3d::MemoryAccess::GPUOnlyRead, cr3d::DataFormat::R16_Uint, indexCount);

	float4 colorAsByte = descriptor.color * 255.0f;

	ComplexVertexPosition* positionData = (ComplexVertexPosition*)renderDevice->BeginBufferUpload(positionBuffer->GetHardwareBuffer());
	ComplexVertexAdditional* additionalData = (ComplexVertexAdditional*)renderDevice->BeginBufferUpload(additionalBuffer->GetHardwareBuffer());
	uint16_t* indexData = (uint16_t*)renderDevice->BeginBufferUpload(indexBuffer->GetHardwareBuffer());
	{
		uint32_t currentVertex = 0;
		uint32_t currentIndex = 0;
		uint32_t currentFaceVertexCount = 0;

		for (cr3d::CubemapFace::T face = cr3d::CubemapFace::PositiveX; face < cr3d::CubemapFace::Count; ++face)
		{
			uint32_t quadCountW = 1, quadCountH = 1;
			uint32_t vertexCountW = 1, vertexCountH = 1;

			switch (face)
			{
				case cr3d::CubemapFace::PositiveX: 
				case cr3d::CubemapFace::NegativeX:
					quadCountW = quadCountZ; vertexCountW = vertexCountZ;
					quadCountH = quadCountY; vertexCountH = vertexCountY;
					break;
				case cr3d::CubemapFace::PositiveY:
				case cr3d::CubemapFace::NegativeY:
					quadCountW = quadCountX; vertexCountW = vertexCountX;
					quadCountH = quadCountZ; vertexCountH = vertexCountZ;
					break;
				case cr3d::CubemapFace::PositiveZ:
				case cr3d::CubemapFace::NegativeZ:
					quadCountW = quadCountX; vertexCountW = vertexCountX;
					quadCountH = quadCountY; vertexCountH = vertexCountY;
					break;
			}

			uint32_t faceVertexCount = 0;

			float dw = 1.0f / quadCountW;
			float dh = 1.0f / quadCountH;

			float fh = 0.0f;
			for (uint32_t h = 0; h < vertexCountH; ++h, fh += dh)
			{
				float fw = 0.0f;
				for (uint32_t w = 0; w < vertexCountW; ++w, fw += dw)
				{
					float invfw = 1.0f - fw;
					float invfh = 1.0f - fh;

					switch (face)
					{
						case cr3d::CubemapFace::PositiveX: positionData[currentVertex].position = {  1.0_h, (half)(invfh * 2.0f - 1.0f), (half)(fw * 2.0f - 1.0f) }; break;
						case cr3d::CubemapFace::NegativeX: positionData[currentVertex].position = { -1.0_h, (half)(invfh * 2.0f - 1.0f), (half)(invfw * 2.0f - 1.0f) }; break;

						case cr3d::CubemapFace::PositiveY: positionData[currentVertex].position = { (half)(fw * 2.0f - 1.0f),  1.0_h, (half)(invfh * 2.0f - 1.0f) }; break;
						case cr3d::CubemapFace::NegativeY: positionData[currentVertex].position = { (half)(fw * 2.0f - 1.0f), -1.0_h, (half)(fh * 2.0f - 1.0f) }; break;

						case cr3d::CubemapFace::PositiveZ: positionData[currentVertex].position = { (half)(invfw * 2.0f - 1.0f), (half)(invfh * 2.0f - 1.0f),  1.0_h }; break;
						case cr3d::CubemapFace::NegativeZ: positionData[currentVertex].position = { (half)(fw * 2.0f - 1.0f), (half)(invfh * 2.0f - 1.0f), -1.0_h }; break;
					}

					additionalData[currentVertex].uv = { (half)fw, (half)fh };

					additionalData[currentVertex].normal = { 0, 255, 0 };

					additionalData[currentVertex].tangent = { 255, 0, 0 };

					additionalData[currentVertex].color = { (uint8_t)colorAsByte.r, (uint8_t)colorAsByte.g, (uint8_t)colorAsByte.b, (uint8_t)colorAsByte.a };

					currentVertex++;
					faceVertexCount++;
				}
			}

			for (uint32_t h = 0; h < quadCountH; ++h)
			{
				for (uint32_t w = 0; w < quadCountW; ++w)
				{
					indexData[currentIndex++] = (uint16_t)(currentFaceVertexCount + w);
					indexData[currentIndex++] = (uint16_t)(currentFaceVertexCount + w + 1);
					indexData[currentIndex++] = (uint16_t)(currentFaceVertexCount + w + vertexCountW);

					indexData[currentIndex++] = (uint16_t)(currentFaceVertexCount + w + vertexCountW);
					indexData[currentIndex++] = (uint16_t)(currentFaceVertexCount + w + 1);
					indexData[currentIndex++] = (uint16_t)(currentFaceVertexCount + w + 1 + vertexCountW);
				}
			}

			currentFaceVertexCount += faceVertexCount;
		}

		CrAssertMsg(vertexCount == currentVertex, "Mismatch in number of vertices");
		CrAssertMsg(indexCount == currentIndex, "Mismatch in number of indices");
	}
	renderDevice->EndBufferUpload(positionBuffer->GetHardwareBuffer());
	renderDevice->EndBufferUpload(additionalBuffer->GetHardwareBuffer());
	renderDevice->EndBufferUpload(indexBuffer->GetHardwareBuffer());

	sphereMesh->AddVertexBuffer(positionBuffer);
	sphereMesh->AddVertexBuffer(additionalBuffer);

	sphereMesh->SetIndexBuffer(indexBuffer);

	return sphereMesh;
}