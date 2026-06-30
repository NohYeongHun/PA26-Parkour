#include "EnginePch.h"
#include "CubeCell.h"

#include "GameInstance.h"
#include "StaticObject.h"

#include "OctoTree.h"

CCubeCell::CCubeCell()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CCubeCell::Initialize(_float3 vCenter, _float3 vExtent, _uint iDepth)
{
	_float3 Extent = vExtent;
	if (0 < iDepth)
	{
		XMStoreFloat3(&Extent, XMLoadFloat3(&Extent) * 1.2f);
		m_pBoundingBox = new BoundingBox(vCenter, Extent);
	}
	else
		m_pBoundingBox = new BoundingBox(vCenter, Extent);

	ASSERT_CRASH(m_pBoundingBox);

	m_iDepth = iDepth;

	m_pBoundingBox->GetCorners(m_Corners);
	Compute_MinMax();

	if (MAX_DEPTH == iDepth)
	{
		//cout << "Extent : " << Extent.x << endl;
		return S_OK;
	}

	for (_uint i = 0; i < ENUM_CLASS(CORNER::END); ++i)
	{
		_float3 vOffset = {};
		//vOffset.x = (i & 1) ? 0.5f : -0.5f;
		if (i % 4 == 0 || i % 4 == 3)
			vOffset.x = -0.5f;
		else
			vOffset.x = 0.5f;

		vOffset.y = (i & 2) ? 0.5f : -0.5f;
		vOffset.z = (i & 4) ? -0.5f : 0.5f;

		_float3 vChildCenter = _float3(
			vCenter.x + vOffset.x * Extent.x,
			vCenter.y + vOffset.y * Extent.y,
			vCenter.z + vOffset.z * Extent.z
		);

		_float3 vChildExtent = _float3(
			Extent.x * 0.5f,
			Extent.y * 0.5f,
			Extent.z * 0.5f
		);

		CCubeCell* pCubeCell = CCubeCell::Create(vChildCenter, vChildExtent, m_iDepth + 1);
		ASSERT_CRASH(pCubeCell);
		m_ChildCells.push_back(pCubeCell);
	}

    return S_OK;
}

void CCubeCell::Update(const _fvector& vCamPos, vector<class CStaticObject*>* Container)
{
	// Frustrum, BoundingBox Intersect Check
	if (true == m_pGameInstance->IsIn_WorldSpace(m_pBoundingBox))
	{
		// LOD SetUp
		// Low Depth (Cell 단위)
		//if(m_iDepth >= 3)
		Compute_Cell_LOD(vCamPos);

		for (auto& pObject : m_Objects)
		{
			if (nullptr == pObject)
				continue;
			if (true == m_pGameInstance->IsIn_WorldSpace(pObject->Get_BoundingBox()))
			{
				//if (m_iDepth <= 3)
				m_iLODIndex = Compute_Object_LOD(pObject, vCamPos);
				//pObject->Set_LOD(0);
				m_iLODIndex = pObject->IsMaxLOD(m_iLODIndex);
				pObject->Set_LOD(m_iLODIndex);

				// Root는 각 객체 Push
				if (0 == m_iDepth)
				{
					if (Container)
						Container[pObject->Get_LOD()].push_back(pObject);
				}
				// Leaf는 Local Container에 담은 후 병합
				else
					Container[pObject->Get_LOD()].push_back(pObject);
			}
		}

		// Child Update
		if (0 < m_ChildCells.size())
		{
			for (auto& pCell : m_ChildCells)
			{
				if(0 == m_iDepth)
				{
					m_pGameInstance->Add_Work([=, Cell = pCell, CamPos = vCamPos]() {
						vector<CStaticObject*> Container[4];
						Container[0].reserve(1000);
						Container[1].reserve(1000);
						Container[2].reserve(1000);
						Container[3].reserve(1000);
						Cell->Update(CamPos, Container);
						m_pGameInstance->Add_Render_StaticObject(Container);
						});
				}
				else
					pCell->Update(vCamPos, Container);
			}
		}
	}
}

void CCubeCell::Add_Object(CStaticObject* pObject, const _float* pMinMax)
{
	ASSERT_CRASH(pObject);
	ASSERT_CRASH(pMinMax);

	for (size_t i = 0; i < m_ChildCells.size(); ++i)
	{
		if (true == m_ChildCells[i]->isIn(pMinMax))
		{
			m_ChildCells[i]->Add_Object(pObject, pMinMax);
			return;
		}
	}

	{
		lock_guard<recursive_mutex> lock(m_Mutex);
		m_Objects.push_back(pObject);
	}
}

void CCubeCell::Compute_MinMax()
{
	m_MinMax[ENUM_CLASS(MINMAX::MIN_X)] = m_Corners[ENUM_CLASS(CORNER::LBD)].x;
	m_MinMax[ENUM_CLASS(MINMAX::MAX_X)] = m_Corners[ENUM_CLASS(CORNER::RBD)].x;

	m_MinMax[ENUM_CLASS(MINMAX::MIN_Y)] = m_Corners[ENUM_CLASS(CORNER::LBD)].y;
	m_MinMax[ENUM_CLASS(MINMAX::MAX_Y)] = m_Corners[ENUM_CLASS(CORNER::LBU)].y;

	m_MinMax[ENUM_CLASS(MINMAX::MIN_Z)] = m_Corners[ENUM_CLASS(CORNER::LBD)].z;
	m_MinMax[ENUM_CLASS(MINMAX::MAX_Z)] = m_Corners[ENUM_CLASS(CORNER::LFD)].z;
}

_bool CCubeCell::isIn(const _float* pMinMax)
{
	if (pMinMax[ENUM_CLASS(MINMAX::MIN_X)] < m_MinMax[ENUM_CLASS(MINMAX::MIN_X)])
		return false;

	if (pMinMax[ENUM_CLASS(MINMAX::MAX_X)] > m_MinMax[ENUM_CLASS(MINMAX::MAX_X)])
		return false;

	if (pMinMax[ENUM_CLASS(MINMAX::MIN_Y)] < m_MinMax[ENUM_CLASS(MINMAX::MIN_Y)])
		return false;

	if (pMinMax[ENUM_CLASS(MINMAX::MAX_Y)] > m_MinMax[ENUM_CLASS(MINMAX::MAX_Y)])
		return false;

	if (pMinMax[ENUM_CLASS(MINMAX::MIN_Z)] < m_MinMax[ENUM_CLASS(MINMAX::MIN_Z)])
		return false;

	if (pMinMax[ENUM_CLASS(MINMAX::MAX_Z)] > m_MinMax[ENUM_CLASS(MINMAX::MAX_Z)])
		return false;

	return true;
}

void CCubeCell::Compute_Cell_LOD(const _fvector& vCamPos)
{
	_int iLODCnt = m_iLODCnt.load(memory_order_acquire);
	/*if (iLODCnt > MAX_LOD)
		return;*/

	_float fDistance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_pBoundingBox->Center) - vCamPos));
	m_iLODIndex = static_cast<_uint>(fDistance / g_fLODGap);
	m_iLODCnt.fetch_add(1, memory_order_release);
}

_uint CCubeCell::Compute_Object_LOD(CStaticObject* pObject, const _fvector& vCamPos)
{
	_int iLODCnt = m_iLODCnt.load(memory_order_acquire);
	/*if (iLODCnt > MAX_LOD)
		return m_iLODIndex;*/

	_vector vCenter = XMLoadFloat3(&pObject->Get_BoundingBox()->Center);
	_float fDistance = XMVectorGetX(XMVector3Length(vCenter - vCamPos));

	m_iLODCnt.fetch_add(1, memory_order_release);

	return static_cast<_uint>(fDistance / g_fLODGap);
}

CCubeCell* CCubeCell::Create(_float3 vCenter, _float3 vExtent, _uint iDepth)
{
	CCubeCell* pInstance = new CCubeCell();

	if (FAILED(pInstance->Initialize(vCenter, vExtent, iDepth)))
	{
		MSG_BOX("Failed to Create : CubeCell");
		Safe_Release(pInstance);
	}

    return pInstance;
}

void CCubeCell::Free()
{
	__super::Free();

	Safe_Delete(m_pBoundingBox);
	
	for (auto& Object : m_Objects)
		Safe_Release(Object);
	m_Objects.clear();

	for (auto& Child : m_ChildCells)
		Safe_Release(Child);
	m_ChildCells.clear();

	Safe_Release(m_pGameInstance);
}
