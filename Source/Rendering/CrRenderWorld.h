#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Core/Containers/CrVectorSet.h"

class CrRenderModelInstance;

class CrRenderWorld
{
public:

	CrRenderWorld();

	void AddModelInstance(const CrSharedPtr<CrRenderModelInstance>& modelInstance);

	void RemoveModelInstance(const CrSharedPtr<CrRenderModelInstance>& modelInstance);

	template<typename Fn>
	void ForEachRenderModelInstance(Fn fn)
	{
		for (uint32_t i = 0; i < m_renderModelInstances.size(); ++i)
		{
			fn(m_renderModelInstances[i]);
		}
	}

private:

	CrVectorSet<CrSharedPtr<CrRenderModelInstance>> m_renderModelInstances;
};

inline void CrRenderWorld::AddModelInstance(const CrSharedPtr<CrRenderModelInstance>& modelInstance)
{
	m_renderModelInstances.insert(modelInstance);
}

inline void CrRenderWorld::RemoveModelInstance(const CrSharedPtr<CrRenderModelInstance>& modelInstance)
{
	m_renderModelInstances.erase(modelInstance);
}
