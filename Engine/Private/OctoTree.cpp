#include "EnginePch.h"
#include "OctoTree.h"

#include "CubeCell.h"
#include "GameInstance.h"

COctoTree::COctoTree()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

void COctoTree::SetUp_OctoTree(_float3 vCenter, _float3 vExtent)
{
	Safe_Release(m_pRootCell);
	m_pRootCell = CCubeCell::Create(vCenter, vExtent, 0);
	ASSERT_CRASH(m_pRootCell);
}

void COctoTree::Add_To_OctoTree(CStaticObject* pObject, const BoundingBox* pBox)
{
	ASSERT_CRASH(m_pRootCell);

	_float fMinMax[ENUM_CLASS(CCubeCell::MINMAX::END)] = {};

	fMinMax[ENUM_CLASS(CCubeCell::MINMAX::MIN_X)] = pBox->Center.x - pBox->Extents.x * 0.5f;
	fMinMax[ENUM_CLASS(CCubeCell::MINMAX::MAX_X)] = pBox->Center.x + pBox->Extents.x * 0.5f;
	fMinMax[ENUM_CLASS(CCubeCell::MINMAX::MIN_Y)] = pBox->Center.y - pBox->Extents.y * 0.5f;
	fMinMax[ENUM_CLASS(CCubeCell::MINMAX::MAX_Y)] = pBox->Center.y + pBox->Extents.y * 0.5f;
	fMinMax[ENUM_CLASS(CCubeCell::MINMAX::MIN_Z)] = pBox->Center.z - pBox->Extents.z * 0.5f;
	fMinMax[ENUM_CLASS(CCubeCell::MINMAX::MAX_Z)] = pBox->Center.z + pBox->Extents.z * 0.5f;

	m_pRootCell->Add_Object(pObject, fMinMax);
}

void COctoTree::Clear_OctoTree()
{
	Safe_Release(m_pRootCell);
}

void COctoTree::Update()
{
	if (nullptr == m_pRootCell)
		return;

	m_iLODCnt.store(0, memory_order_release);

	//if(true == m_pGameInstance->IsWorkFinish())
	m_pRootCell->Update(XMLoadFloat4(m_pGameInstance->Get_CamPos()));
}

COctoTree* COctoTree::Create()
{
    return new COctoTree();
}

void COctoTree::Free()
{
	__super::Free();

	Safe_Release(m_pRootCell);

	Safe_Release(m_pGameInstance);
}
