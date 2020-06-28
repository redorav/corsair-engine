#pragma once

#include "CrEntity.h"
#include "Rendering/WireMesh.h"
#include "Rendering/Line.h"

class Grid : public CrEntity
{
public:

	Grid(float width, float height, int gridX, int gridY)
	{
		WireMesh * gridWireMesh = new WireMesh();

		float gridWidth = width / gridX;
		float gridHeight = height / gridY;

		float halfWidth = width * 0.5f;
		float halfHeight = height * 0.5f;

		for (int i = 0; i <= gridX; ++i)
		{
			float x = -halfWidth + i * gridWidth;
			gridWireMesh->AddLine(Line(Vector3(x, -halfHeight, 0), Vector3(x, halfHeight, 0)));
		}

		for (int j = 0; j <= gridY; ++j)
		{
			float y = -halfHeight + j * gridHeight;
			gridWireMesh->AddLine(Line(Vector3(-halfWidth, y, 0), Vector3(halfWidth, y, 0)));
		}

		gridWireMesh->GenerateBuffers();

		debugWireMeshes.push_back(gridWireMesh);
	}
};