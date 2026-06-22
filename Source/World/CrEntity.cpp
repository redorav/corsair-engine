#include "CrEntity.h"

CrEntity::CrEntity() : CrEntity("")
{

}

CrEntity::CrEntity(const crstl::string& name)
	: m_qrotation(quaternion::identity())
	, m_name(name)
{

}

void CrEntity::SetParent(CrEntity* const parent)
{
	m_parent = parent;

	// Update matrix to be parented
}