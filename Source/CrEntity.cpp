#include "CrEntity.h"

#include "Math/CrTransform.h"

//void GameObject::UpdateTransform()
//{
	//m_worldMatrix = Matrix4::Identity();
	//Matrix3 scaleM = Matrix3::ScaleMatrix(scale.x, scale.y, scale.z);
	//Matrix3 RS = rotation * scaleM;
	//m_worldMatrix.CopyAffineTransform(RS);
	//m_worldMatrix.Translate(position);
//}  

CrEntity::CrEntity() : CrEntity("")
{

}

CrEntity::CrEntity(const CrString& name) 
	: m_qrotation(quaternion::identity())
	, m_name(name)
{

}

void CrEntity::SetParent(CrEntity* const parent)
{
	m_parent = parent;

	// Update matrix to be parented
}

void CrEntity::SetLocalTransform(const CrTransform& localTransform)
{
	*m_localTransform = localTransform;

	// Update world matrix
}
