#pragma once

#include "CrEntity.h"
#include "Rendering/Geometry.h"

class Gizmo : public CrEntity
{
public:

	Gizmo(Vector3 & size)
	{
		CrEntity * centerSphere = new CrEntity("centerSphere");
		centerSphere->mesh = Geometry::CreateSphere(0.3f, 20);
		centerSphere->mesh->GenerateBuffers();

		//centerSphere->forwardMaterial = new Material(ResourceManager::GetShader("main"));
		//centerSphere->forwardMaterial->SetDiffuseMap(TextureManagerGen::GetTexture("square_test.png"));

		gameObjects.push_back(centerSphere);

		CrEntity * xAxis = new Axis(Vector3(0, 0, 1), 4.0f);
		gameObjects.push_back(xAxis);
	}

private:
	class Axis : public CrEntity
	{
	public:
		Axis(const Vector3& axis, float length)
		{
			float cylinderLength = length * 4 / 5;

			CrEntity * axisCylinder = new CrEntity("axisCylinder");
			axisCylinder->position_scal = position_scal + axis * cylinderLength / 2;
			axisCylinder->mesh = Geometry::CreateCylinder(cylinderLength / 18, cylinderLength, 24);
			axisCylinder->mesh->GenerateBuffers();

			axisCylinder->forwardMaterial = new Material(ResourceManager::GetShader("main"));
			//axisCylinder->forwardMaterial->SetDiffuseMap(TextureManagerGen::GetTexture("square_test.png"));

			gameObjects.push_back(axisCylinder);

			float coneLength = length - cylinderLength;
			// Length of axis cone is equal to 1/4 the length of the actual axis
			CrEntity * axisCone = new CrEntity("axisCone");
			axisCone->position_scal = position_scal + axis * (cylinderLength + coneLength / 2);
			axisCone->mesh = Geometry::CreateCone(coneLength / 2, coneLength, 10);
			axisCone->mesh->GenerateBuffers();

			axisCone->forwardMaterial = new Material(ResourceManager::GetShader("main"));
			//axisCone->forwardMaterial->SetDiffuseMap(TextureManagerGen::GetTexture("square_test.png"));

			gameObjects.push_back(axisCone);
		}
	};
};