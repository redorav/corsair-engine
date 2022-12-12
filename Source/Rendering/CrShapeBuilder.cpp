#include "CrRendering_pch.h"

#include "CrShapeBuilder.h"
#include "CrCommonVertexLayouts.h"

#include "CrRendering.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"

CrRenderMeshHandle CrShapeBuilder::CreateQuad(const CrQuadDescriptor& descriptor)
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

	CrRenderMeshHandle quadMesh = CrRenderMeshHandle(new CrRenderMesh());
	quadMesh->AddVertexBuffer(positionBuffer);
	quadMesh->AddVertexBuffer(additionalBuffer);
	quadMesh->SetIndexBuffer(indexBuffer);

	return quadMesh;
}

CrRenderMeshHandle CrShapeBuilder::CreateCube(const CrCubeDescriptor& descriptor)
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

	// Compute number of vertices per row
	uint32_t vertexCountX = 2 + descriptor.subdivisionX;
	uint32_t vertexCountY = 2 + descriptor.subdivisionY;
	uint32_t vertexCountZ = 2 + descriptor.subdivisionZ;

	uint32_t quadCountX = vertexCountX - 1;
	uint32_t quadCountY = vertexCountY - 1;
	uint32_t quadCountZ = vertexCountZ - 1;

	uint32_t vertexCount = 2 * (vertexCountY * vertexCountZ + vertexCountX * vertexCountZ + vertexCountX * vertexCountY);

	// Number of quads per face, times 2 faces (positive and negative) * 2 triangles per quad * 3 vertices per triangle
	uint32_t indexCount = (quadCountY * quadCountZ + quadCountX * quadCountZ + quadCountX * quadCountY) * 2 * 2 * 3;

	const CrRenderDeviceSharedHandle& renderDevice = ICrRenderSystem::GetRenderDevice();
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
					float3 cubePosition;

					switch (face)
					{
						case cr3d::CubemapFace::PositiveX: cubePosition = { 1.0f, 1.0f - fh, fw }; break;
						case cr3d::CubemapFace::NegativeX: cubePosition = { 0.0f, 1.0f - fh, 1.0f - fw }; break;

						case cr3d::CubemapFace::PositiveY: cubePosition = { fw, 1.0f, 1.0f - fh }; break;
						case cr3d::CubemapFace::NegativeY: cubePosition = { fw, 0.0f, fh }; break;

						case cr3d::CubemapFace::PositiveZ: cubePosition = { 1.0f - fw, 1.0f - fh, 1.0f }; break;
						case cr3d::CubemapFace::NegativeZ: cubePosition = { fw, 1.0f - fh, 0.0f }; break;
					}

					cubePosition = cubePosition * 2.0f - 1.0f;

					positionData[currentVertex].position = { (half)cubePosition.x, (half)cubePosition.y, (half)cubePosition.z };

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
				uint32_t baseVertex = currentFaceVertexCount + h * vertexCountW;

				for (uint32_t w = 0; w < quadCountW; ++w)
				{
					indexData[currentIndex++] = (uint16_t)(baseVertex + w);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + 1);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + vertexCountW);

					indexData[currentIndex++] = (uint16_t)(baseVertex + w + vertexCountW);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + 1);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + 1 + vertexCountW);
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

	CrRenderMeshHandle cubeMesh = CrRenderMeshHandle(new CrRenderMesh());
	cubeMesh->AddVertexBuffer(positionBuffer);
	cubeMesh->AddVertexBuffer(additionalBuffer);
	cubeMesh->SetIndexBuffer(indexBuffer);

	return cubeMesh;
}

CrRenderMeshHandle CrShapeBuilder::CreateSphere(const CrSphereDescriptor& descriptor)
{
	uint32_t vertexCountFace = 2 + descriptor.subdivision;

	uint32_t quadCountFace = vertexCountFace - 1;

	uint32_t vertexCount = 2 * 3 * (vertexCountFace * vertexCountFace);

	// Number of quads per face, times 2 faces (positive and negative) * 2 triangles per quad * 3 vertices per triangle
	uint32_t indexCount = (quadCountFace * quadCountFace) * 3 * 2 * 2 * 3;

	const CrRenderDeviceSharedHandle& renderDevice = ICrRenderSystem::GetRenderDevice();
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
			uint32_t quadCountW = quadCountFace, quadCountH = quadCountFace;
			uint32_t vertexCountW = vertexCountFace, vertexCountH = vertexCountFace;

			uint32_t faceVertexCount = 0;

			float dw = 1.0f / quadCountW;
			float dh = 1.0f / quadCountH;

			float fh = 0.0f;
			for (uint32_t h = 0; h < vertexCountH; ++h, fh += dh)
			{
				float fw = 0.0f;
				for (uint32_t w = 0; w < vertexCountW; ++w, fw += dw)
				{
					float3 cubePosition;

					switch (face)
					{
						case cr3d::CubemapFace::PositiveX: cubePosition = { 1.0f, 1.0f - fh, fw }; break;
						case cr3d::CubemapFace::NegativeX: cubePosition = { 0.0f, 1.0f - fh, 1.0f - fw }; break;

						case cr3d::CubemapFace::PositiveY: cubePosition = { fw, 1.0f, 1.0f - fh }; break;
						case cr3d::CubemapFace::NegativeY: cubePosition = { fw, 0.0f, fh }; break;

						case cr3d::CubemapFace::PositiveZ: cubePosition = { 1.0f - fw, 1.0f - fh, 1.0f }; break;
						case cr3d::CubemapFace::NegativeZ: cubePosition = { fw, 1.0f - fh, 0.0f }; break;
					}

					float3 spherePosition = normalize(cubePosition * 2.0f - 1.0f);

					spherePosition *= descriptor.radius;

					positionData[currentVertex].position = { (half)spherePosition.x, (half)spherePosition.y, (half)spherePosition.z };

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
				uint32_t baseVertex = currentFaceVertexCount + h * vertexCountW;

				for (uint32_t w = 0; w < quadCountW; ++w)
				{
					indexData[currentIndex++] = (uint16_t)(baseVertex + w);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + 1);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + vertexCountW);

					indexData[currentIndex++] = (uint16_t)(baseVertex + w + vertexCountW);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + 1);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + 1 + vertexCountW);
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

	CrRenderMeshHandle sphereMesh = CrRenderMeshHandle(new CrRenderMesh());
	sphereMesh->AddVertexBuffer(positionBuffer);
	sphereMesh->AddVertexBuffer(additionalBuffer);
	sphereMesh->SetIndexBuffer(indexBuffer);

	return sphereMesh;
}
