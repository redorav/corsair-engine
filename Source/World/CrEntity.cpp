#include "CrEntity.h"

CrEntity::CrEntity() : CrEntity("")
{

}

CrEntity::CrEntity(const crstl::string& name)
	: m_transform(float4x4::identity())
	, m_name(name)
{

}

void CrEntity::SetParent(CrEntity* const parent)
{
	m_parent = parent;

	// Update matrix to be parented
}