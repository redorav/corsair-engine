#include "CrRendering_pch.h"

#include "CrShapeBuilder.h"
#include "CrCommonVertexLayouts.h"

#include "CrRendering.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"

#include "Math/CrMath.h"

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

	const CrRenderDeviceHandle& renderDevice = ICrRenderSystem::GetRenderDevice();

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
				float4 position = float4(fx, 0.5f, 1.0f - fy, 1.0f) * 2.0f - 1.0f;

				position = mul(position, descriptor.transform);

				positionData[currentVertex].position = { (half)position.x, (half)position.y, (half)position.z };

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
	//     A +----------+  B
	//      /|         /|
	//     / |        / |
	//  C +----------+ D|
	//    |  | E     |  | F
	//    |  +- - - -|- +
	//    | /        | /
	//    |/         |/
	//    +----------+
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

	const CrRenderDeviceHandle& renderDevice = ICrRenderSystem::GetRenderDevice();
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
		
		struct FaceProperties
		{
			uint32_t quadCountW, quadCountH, vertexCountW, vertexCountH;
			float3 normal;
			float3 tangent;
		};

		const FaceProperties FacePropertyArray[cr3d::CubemapFace::Count] =
		{
			{ quadCountZ, quadCountY, vertexCountZ, vertexCountY, float3( 1.0f,  0.0f,  0.0f) },
			{ quadCountZ, quadCountY, vertexCountZ, vertexCountY, float3(-1.0f,  0.0f,  0.0f) },
			{ quadCountX, quadCountZ, vertexCountX, vertexCountZ, float3( 0.0f,  1.0f,  0.0f) },
			{ quadCountX, quadCountZ, vertexCountX, vertexCountZ, float3( 0.0f, -1.0f,  0.0f) },
			{ quadCountX, quadCountY, vertexCountX, vertexCountY, float3( 0.0f,  0.0f,  1.0f) },
			{ quadCountX, quadCountY, vertexCountX, vertexCountY, float3( 0.0f,  0.0f, -1.0f) }
		};

		for (cr3d::CubemapFace::T face = cr3d::CubemapFace::PositiveX; face < cr3d::CubemapFace::Count; ++face)
		{
			const FaceProperties& faceProperties = FacePropertyArray[face];

			float3 normalAsByte = (faceProperties.normal * 0.5f + 0.5f) * 255.0f;

			uint32_t faceVertexCount = 0;

			float dw = 1.0f / faceProperties.quadCountW;
			float dh = 1.0f / faceProperties.quadCountH;

			float fh = 0.0f;
			for (uint32_t h = 0; h < faceProperties.vertexCountH; ++h, fh += dh)
			{
				float fw = 0.0f;
				for (uint32_t w = 0; w < faceProperties.vertexCountW; ++w, fw += dw)
				{
					float4 cubePosition;

					switch (face)
					{
						case cr3d::CubemapFace::PositiveX: cubePosition = { 1.0f, 1.0f - fh, fw, 1.0f }; break;
						case cr3d::CubemapFace::NegativeX: cubePosition = { 0.0f, 1.0f - fh, 1.0f - fw, 1.0f }; break;

						case cr3d::CubemapFace::PositiveY: cubePosition = { fw, 1.0f, 1.0f - fh, 1.0f }; break;
						case cr3d::CubemapFace::NegativeY: cubePosition = { fw, 0.0f, fh, 1.0f }; break;

						case cr3d::CubemapFace::PositiveZ: cubePosition = { 1.0f - fw, 1.0f - fh, 1.0f, 1.0f }; break;
						case cr3d::CubemapFace::NegativeZ: cubePosition = { fw, 1.0f - fh, 0.0f, 1.0f }; break;
					}

					cubePosition = mul(cubePosition * 2.0f - 1.0f, descriptor.transform);

					positionData[currentVertex].position = { (half)cubePosition.x, (half)cubePosition.y, (half)cubePosition.z };

					additionalData[currentVertex].uv = { (half)fw, (half)fh };

					additionalData[currentVertex].normal = { (uint8_t)normalAsByte.r, (uint8_t)normalAsByte.g, (uint8_t)normalAsByte.b };

					additionalData[currentVertex].tangent = { 255, 0, 0 };

					additionalData[currentVertex].color = { (uint8_t)colorAsByte.r, (uint8_t)colorAsByte.g, (uint8_t)colorAsByte.b, (uint8_t)colorAsByte.a };

					currentVertex++;
					faceVertexCount++;
				}
			}

			for (uint32_t h = 0; h < faceProperties.quadCountH; ++h)
			{
				uint32_t baseVertex = currentFaceVertexCount + h * faceProperties.vertexCountW;

				for (uint32_t w = 0; w < faceProperties.quadCountW; ++w)
				{
					indexData[currentIndex++] = (uint16_t)(baseVertex + w);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + 1);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + faceProperties.vertexCountW);

					indexData[currentIndex++] = (uint16_t)(baseVertex + w + faceProperties.vertexCountW);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + 1);
					indexData[currentIndex++] = (uint16_t)(baseVertex + w + 1 + faceProperties.vertexCountW);
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

	const CrRenderDeviceHandle& renderDevice = ICrRenderSystem::GetRenderDevice();
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

					cubePosition = cubePosition * 2.0f - 1.0f;

					float4 spherePosition = float4(normalize(cubePosition), 1.0f);

					spherePosition = mul(spherePosition, descriptor.transform);

					float3 normal = normalize(spherePosition.xyz);

					positionData[currentVertex].position = { (half)spherePosition.x, (half)spherePosition.y, (half)spherePosition.z };

					additionalData[currentVertex].uv = { (half)fw, (half)fh };

					float3 normalAsByte = (normal * 0.5f + 0.5f) * 255.0f;

					additionalData[currentVertex].normal = { (uint8_t)normalAsByte.r, (uint8_t)normalAsByte.g, (uint8_t)normalAsByte.b };

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

CrRenderMeshHandle CrShapeBuilder::CreateCylinder(const CrCylinderDescriptor& descriptor)
{
	// Number of vertices in ring (not counting the duplicated one)
	uint32_t vertexCountRingLogical = descriptor.subdivisionAxis + 3;

	// Vertices at each ring (minimum of 3) + 1 for when we wrap around (needs different UV)
	uint32_t vertexCountRingPhysical = vertexCountRingLogical + 1;

	// Number of rings * vertex count of each ring + two extra vertices at the bottom and the top
	uint32_t vertexCount = vertexCountRingPhysical * (2 + descriptor.subdivisionLength) + 2;

	// Side quads follow the number of vertices
	uint32_t quadCountSides = vertexCountRingLogical;

	// Triangles for the base follow the number of vertices
	uint32_t triangleCountBase = vertexCountRingLogical;

	uint32_t triangleCount = quadCountSides * 2 + triangleCountBase * 2;

	uint32_t indexCount = triangleCount * 3;

	const CrRenderDeviceHandle& renderDevice = ICrRenderSystem::GetRenderDevice();
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

		// Add top tip vertex
		{
			float4 topTipPosition = mul(float4(0.0f, 1.0f, 0.0f, 1.0f), descriptor.transform);
			positionData[currentVertex].position = { (half)topTipPosition.x, (half)topTipPosition.y, (half)topTipPosition.z };
			additionalData[currentVertex].uv = { 0.5_h, 0.0_h };
			additionalData[currentVertex].normal = { 0, 255, 0 };
			additionalData[currentVertex].tangent = { 255, 0, 0 };
			additionalData[currentVertex].color = { (uint8_t)colorAsByte.r, (uint8_t)colorAsByte.g, (uint8_t)colorAsByte.b, (uint8_t)colorAsByte.a };
			currentVertex++;
		}

		// Indices for top cap
		for (uint32_t v = 0; v < vertexCountRingLogical; ++v)
		{
			indexData[currentIndex++] = 0;
			indexData[currentIndex++] = (uint16_t)(v + 2);
			indexData[currentIndex++] = (uint16_t)(v + 1);
		}

		// Go from the top cap to the bottom
		for (uint32_t l = 0; l <= 1; ++l)
		{
			float fh = 1.0f - 2.0f * l;

			for (uint32_t v = 0; v <= vertexCountRingLogical; ++v)
			{
				float theta = (2.0f * CrMath::Pi * v) / vertexCountRingLogical;
				float fw = (float)v / vertexCountRingLogical;

				float x = cosf(theta);
				float z = sinf(theta);

				float4 position = mul(float4(x, fh, z, 1.0f), descriptor.transform);

				positionData[currentVertex].position = { (half)position.x, (half)position.y, (half)position.z };

				additionalData[currentVertex].uv = { (half)fw, (half)fh };

				float3 normal = float3(x, 0.0f, z);

				float3 normalAsByte = (normal * 0.5f + 0.5f) * 255.0f;

				additionalData[currentVertex].normal = { (uint8_t)normalAsByte.r, (uint8_t)normalAsByte.g, (uint8_t)normalAsByte.b };

				additionalData[currentVertex].tangent = { 255, 0, 0 };

				additionalData[currentVertex].color = { (uint8_t)colorAsByte.r, (uint8_t)colorAsByte.g, (uint8_t)colorAsByte.b, (uint8_t)colorAsByte.a };

				currentVertex++;
			}
		}

		uint32_t currentRingVertexCount = 1; // Account for the top vertex

		for (uint32_t l = 0; l < 1; ++l)
		{
			// Skip the last vertex to add indices
			for (uint32_t v = 0; v < vertexCountRingLogical; ++v)
			{
				uint32_t baseVertex = currentRingVertexCount + v;

				indexData[currentIndex++] = (uint16_t)(baseVertex);
				indexData[currentIndex++] = (uint16_t)(baseVertex + 1);
				indexData[currentIndex++] = (uint16_t)(baseVertex + vertexCountRingPhysical);

				indexData[currentIndex++] = (uint16_t)(baseVertex + vertexCountRingPhysical);
				indexData[currentIndex++] = (uint16_t)(baseVertex + 1);
				indexData[currentIndex++] = (uint16_t)(baseVertex + vertexCountRingPhysical + 1);
			}

			currentRingVertexCount += vertexCountRingPhysical;
		}

		// Indices for bottom cap
		for (uint32_t v = 0; v < vertexCountRingLogical; ++v)
		{
			uint32_t baseVertex = currentVertex - vertexCountRingPhysical;
			indexData[currentIndex++] = (uint16_t)currentVertex;
			indexData[currentIndex++] = (uint16_t)(baseVertex + v + 0);
			indexData[currentIndex++] = (uint16_t)(baseVertex + v + 1);
		}

		// Add bottom tip vertex
		{
			float4 bottomTipPosition = mul(float4(0.0f, -1.0f, 0.0f, 1.0f), descriptor.transform);
			positionData[currentVertex].position = { (half)bottomTipPosition.x, (half)bottomTipPosition.y, (half)bottomTipPosition.z };
			additionalData[currentVertex].uv = { 0.5_h, 1.0_h };
			additionalData[currentVertex].normal = { 0, 0, 0 };
			additionalData[currentVertex].tangent = { 255, 0, 0 };
			additionalData[currentVertex].color = { (uint8_t)colorAsByte.r, (uint8_t)colorAsByte.g, (uint8_t)colorAsByte.b, (uint8_t)colorAsByte.a };
			currentVertex++;
		}

		CrAssertMsg(vertexCount == currentVertex, "Mismatch in number of vertices");
		CrAssertMsg(indexCount == currentIndex, "Mismatch in number of indices");
	}
	renderDevice->EndBufferUpload(positionBuffer->GetHardwareBuffer());
	renderDevice->EndBufferUpload(additionalBuffer->GetHardwareBuffer());
	renderDevice->EndBufferUpload(indexBuffer->GetHardwareBuffer());

	CrRenderMeshHandle cylinderMesh = CrRenderMeshHandle(new CrRenderMesh());
	cylinderMesh->AddVertexBuffer(positionBuffer);
	cylinderMesh->AddVertexBuffer(additionalBuffer);
	cylinderMesh->SetIndexBuffer(indexBuffer);

	return cylinderMesh;
}

CrRenderMeshHandle CrShapeBuilder::CreateCone(const CrConeDescriptor& descriptor)
{
	// Minimum of 3 vertices
	uint32_t vertexCountRingLogical = descriptor.subdivisionAxis + 3;

	// Vertices at each ring (minimum of 3) + 1 for when we wrap around (needs different UV)
	uint32_t vertexCountRingPhysical = vertexCountRingLogical + 1;

	// Number of rings * vertex count of each ring + two extra vertices at the bottom and the top
	uint32_t vertexCount = vertexCountRingPhysical * (descriptor.subdivisionLength + 1) + 2;

	// Side triangles follow the number of vertices
	uint32_t triangleCountSides = vertexCountRingLogical;

	// Triangles for the base follow the number of vertices
	uint32_t triangleCountBase = vertexCountRingLogical;

	uint32_t triangleCount = triangleCountSides + triangleCountBase;

	uint32_t indexCount = triangleCount * 3;

	const CrRenderDeviceHandle& renderDevice = ICrRenderSystem::GetRenderDevice();
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

		// Add top tip vertex
		{
			float4 topTipPosition = mul(float4(0.0f, 1.0f, 0.0f, 1.0f), descriptor.transform);
			positionData[currentVertex].position = { (half)topTipPosition.x, (half)topTipPosition.y, (half)topTipPosition.z };
			additionalData[currentVertex].uv = { 0.5_h, 0.0_h };
			additionalData[currentVertex].normal = { 0, 255, 0 };
			additionalData[currentVertex].tangent = { 255, 0, 0 };
			additionalData[currentVertex].color = { (uint8_t)colorAsByte.r, (uint8_t)colorAsByte.g, (uint8_t)colorAsByte.b, (uint8_t)colorAsByte.a };
			currentVertex++;
		}

		float fh = 0.0f;

		for (uint32_t v = 0; v <= vertexCountRingLogical; ++v)
		{
			float theta = (2.0f * CrMath::Pi * v) / vertexCountRingLogical;
			float fw = (float)v / vertexCountRingLogical;

			float x = cosf(theta);
			float z = sinf(theta);

			float4 position = mul(float4(x, fh, z, 1.0f), descriptor.transform);

			positionData[currentVertex].position = { (half)position.x, (half)position.y, (half)position.z };

			additionalData[currentVertex].uv = { (half)fw, (half)fh };

			float3 normal = float3(x, 0.0f, z);

			float3 normalAsByte = (normal * 0.5f + 0.5f) * 255.0f;

			additionalData[currentVertex].normal = { (uint8_t)normalAsByte.r, (uint8_t)normalAsByte.g, (uint8_t)normalAsByte.b };

			additionalData[currentVertex].tangent = { 255, 0, 0 };

			additionalData[currentVertex].color = { (uint8_t)colorAsByte.r, (uint8_t)colorAsByte.g, (uint8_t)colorAsByte.b, (uint8_t)colorAsByte.a };

			currentVertex++;
		}

		// Triangles for the top
		for (uint32_t v = 0; v < vertexCountRingLogical; ++v)
		{
			indexData[currentIndex++] = (uint16_t)0;
			indexData[currentIndex++] = (uint16_t)(v + 2);
			indexData[currentIndex++] = (uint16_t)(v + 1);
		}

		// Triangles for the base
		for (uint32_t v = 0; v < vertexCountRingLogical; ++v)
		{
			indexData[currentIndex++] = (uint16_t)(currentVertex);
			indexData[currentIndex++] = (uint16_t)(v + 1);
			indexData[currentIndex++] = (uint16_t)(v + 2);
		}

		// Add bottom vertex
		{
			positionData[currentVertex].position = { 0.0_h, 0.0_h, 0.0_h };
			additionalData[currentVertex].uv = { 0.5_h, 1.0_h };
			additionalData[currentVertex].normal = { 0, 0, 0 };
			additionalData[currentVertex].tangent = { 255, 0, 0 };
			additionalData[currentVertex].color = { (uint8_t)colorAsByte.r, (uint8_t)colorAsByte.g, (uint8_t)colorAsByte.b, (uint8_t)colorAsByte.a };
			currentVertex++;
		}

		CrAssertMsg(vertexCount == currentVertex, "Mismatch in number of vertices");
		CrAssertMsg(indexCount == currentIndex, "Mismatch in number of indices");
	}
	renderDevice->EndBufferUpload(positionBuffer->GetHardwareBuffer());
	renderDevice->EndBufferUpload(additionalBuffer->GetHardwareBuffer());
	renderDevice->EndBufferUpload(indexBuffer->GetHardwareBuffer());

	CrRenderMeshHandle coneMesh = CrRenderMeshHandle(new CrRenderMesh());
	coneMesh->AddVertexBuffer(positionBuffer);
	coneMesh->AddVertexBuffer(additionalBuffer);
	coneMesh->SetIndexBuffer(indexBuffer);

	return coneMesh;
}
