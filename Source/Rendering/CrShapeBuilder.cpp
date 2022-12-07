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
	uint32_t indexCount = quadCountX * quadCountY * 3 * 2;

	CrVertexBufferSharedHandle positionBuffer = renderDevice->CreateVertexBuffer(cr3d::MemoryAccess::GPUOnlyRead, PositionVertexDescriptor, vertexCount);
	CrVertexBufferSharedHandle additionalBuffer = renderDevice->CreateVertexBuffer(cr3d::MemoryAccess::GPUOnlyRead, AdditionalVertexDescriptor, vertexCount);

	float4 colorAsByte = descriptor.color * 255.0f;

	ComplexVertexPosition* positionBufferData = (ComplexVertexPosition*)renderDevice->BeginBufferUpload(positionBuffer->GetHardwareBuffer());
	ComplexVertexAdditional* additionalBufferData = (ComplexVertexAdditional*)renderDevice->BeginBufferUpload(additionalBuffer->GetHardwareBuffer());
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
				positionBufferData[currentVertex].position = { (half)(fx * 2.0f - 1.0f), 0.0_h, (half)((1.0f - fy) * 2.0f - 1.0f) };

				additionalBufferData[currentVertex].uv = { (half)fx, (half)fy };

				additionalBufferData[currentVertex].normal = { 0, 255, 0 };

				additionalBufferData[currentVertex].tangent = { 255, 0, 0 };

				additionalBufferData[currentVertex].color = { (uint8_t)colorAsByte.r, (uint8_t)colorAsByte.g, (uint8_t)colorAsByte.b, (uint8_t)colorAsByte.a };

				currentVertex++;
			}
		}

		CrAssertMsg(vertexCount == currentVertex, "Mismatch in number of vertices");
	}
	renderDevice->EndBufferUpload(positionBuffer->GetHardwareBuffer());
	renderDevice->EndBufferUpload(additionalBuffer->GetHardwareBuffer());

	CrIndexBufferSharedHandle indexBuffer = ICrRenderSystem::GetRenderDevice()->CreateIndexBuffer(cr3d::MemoryAccess::GPUOnlyRead, cr3d::DataFormat::R16_Uint, indexCount);
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