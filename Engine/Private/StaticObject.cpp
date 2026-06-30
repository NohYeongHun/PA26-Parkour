#include "EnginePch.h"
#include "StaticObject.h"
#include "GameInstance.h"

CStaticObject::CStaticObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CStaticObject::CStaticObject(const CStaticObject& Prototype)
	: CGameObject { Prototype }
{
}

_float CStaticObject::Compute_Distance(const _fvector& vCamPos)
{
    return XMVectorGetX(XMVector3Length(vCamPos - m_pTransformCom->Get_State(STATE::POSITION)));
}

_uint CStaticObject::IsMaxLOD(_uint iLODIndex)
{
	if (m_iNumLOD <= iLODIndex)
		return m_iNumLOD;
	else
		return iLODIndex;
}

void CStaticObject::Sync_Sectors()
{
	if (nullptr == m_pBoundingBox)
	{
		MSG_BOX("None BoundingBox");
		return;
	}

	_uint iSector = 0;
	for (auto& Sector : m_pGameInstance->Get_ShadowMapSectors())
	{
		if (Sector->Intersects(*m_pBoundingBox))
			m_Sectors.push_back(iSector);
		++iSector;
	}
}

void CStaticObject::Free()
{
	__super::Free();
	Safe_Delete(m_pBoundingBox);
}
