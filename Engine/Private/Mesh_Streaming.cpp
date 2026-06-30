#include"EnginePch.h"
#include"Mesh_Streaming.h"

CMesh_Streaming::CMesh_Streaming(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent(pDevice,pContext)
{
}

CMesh_Streaming::CMesh_Streaming(const CMesh_Streaming& Prototype)
	:CComponent(Prototype), m_iNumMeshes{Prototype.m_iNumMeshes },
	m_pSharedVB{Prototype.m_pSharedVB},
	m_pSharedIB{Prototype.m_pSharedIB},
	m_Desc{ Prototype.m_Desc }
#ifdef _DEBUG
	,m_pBoundingBox{Prototype.m_pBoundingBox }
#endif
{
	Safe_AddRef(m_pSharedVB);
	Safe_AddRef(m_pSharedIB);
}

HRESULT CMesh_Streaming::Initialize_Prototype(_uint iNumMeshes)
{
	m_iNumMeshes = iNumMeshes;
	m_Desc = new vector<CModel_Manager::SHARED_DATA_DESC>;
	//모델매니저에서 버퍼 풀 주소 받기.
	m_vecIndices = new vector<_uint>[m_iNumMeshes];
	m_vecVertexPos = new vector<_float3>[m_iNumMeshes];
	return S_OK;
}

HRESULT CMesh_Streaming::Initialize_Clone(void* pArg)
{
	return S_OK;
}

HRESULT CMesh_Streaming::Render(_uint iMeshIndex)
{
	//Model_Manager에서 바인딩 된 놈들만 한 번에 Draw. 디퍼드 컨텍스트 이용할 거면 나중에 따로 생성.
	//LOD3번은 로딩 다 될 때까지 기다려야함. 
	if (m_Desc->empty())
		CRASH("Failed");

	if (iMeshIndex >= m_Desc->size())
		return E_FAIL;

	const CModel_Manager::SHARED_DATA_DESC& RenderDesc = (*m_Desc)[iMeshIndex];

	//임시 하드코딩
	//나중에 방식을 오프셋을 건드리기 vs 바인딩을 버퍼 하나에서 오프셋을 바꾸기에서 선택할것.
	m_pContext->DrawIndexed(RenderDesc.NumIndices, RenderDesc.IndexOffset, RenderDesc.VertexOffset);
	//m_pContext->DrawIndexed(RenderDesc.NumIndices, 0, 0);
	return S_OK;
}

void CMesh_Streaming::Set_Buffers(ID3D11Buffer* pSharedVB, ID3D11Buffer* pSharedIB)
{
	m_pSharedVB = pSharedVB;
	m_pSharedIB = pSharedIB;
	Safe_AddRef(m_pSharedVB);
	Safe_AddRef(m_pSharedIB);
}

HRESULT CMesh_Streaming::Bind_Resources(_uint iMeshIndex)
{
	ID3D11Buffer* Buffers[] = {
		m_pSharedVB,
	};

	_uint Strides[] = {
		m_iVertexStride,
	};

	_uint Offsets[] = {
		0,
	};
	m_pContext->IASetVertexBuffers(0, 1, Buffers, Strides, Offsets);
	m_pContext->IASetIndexBuffer(m_pSharedIB, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT CMesh_Streaming::Bind_Resources(_uint iMeshIndex, ID3D11DeviceContext* pDC)
{
	ID3D11Buffer* Buffers[] = {
		m_pSharedVB,
	};

	_uint Strides[] = {
		m_iVertexStride,
	};

	_uint Offsets[] = {
		0,
	};
	pDC->IASetVertexBuffers(0, 1, Buffers, Strides, Offsets);
	pDC->IASetIndexBuffer(m_pSharedIB, m_eIndexFormat, 0);
	pDC->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT CMesh_Streaming::Render(_uint iMeshIndex, ID3D11DeviceContext* pDC)
{
	if (m_Desc->empty())
		CRASH("Failed");

	if (iMeshIndex >= m_Desc->size())
		return E_FAIL;

	const CModel_Manager::SHARED_DATA_DESC& RenderDesc = (*m_Desc)[iMeshIndex];

	//임시 하드코딩
	//pDC->DrawIndexed(RenderDesc.NumIndices, RenderDesc.IndexOffset / sizeof(_uint), RenderDesc.VertexOffset / m_iVertexStride);
	pDC->DrawIndexed(RenderDesc.NumIndices, RenderDesc.IndexOffset, RenderDesc.VertexOffset);
	return S_OK;
}

void CMesh_Streaming::Set_RigidData(vector<_float3>& vecVertexPos, vector<_uint>& vecIndices, _uint iMeshIndex)
{
	m_vecIndices[iMeshIndex] = vecIndices;
	m_vecVertexPos[iMeshIndex] = move(vecVertexPos);
}

void CMesh_Streaming::Destroy_RigidData()
{
	Safe_Delete_Array(m_vecVertexPos);
	Safe_Delete_Array(m_vecIndices);
}

#ifdef _DEBUG
void CMesh_Streaming::Ready_BoundingBox()
{
	if (m_pBoundingBox)
		return;
	_float* pMin = new _float[3];
	_float* pMax = new _float[3];

	for (_uint i = 0; i < 3; ++i)
	{
		pMin[i] = FLT_MAX;
		pMax[i] = FLT_MIN;
	}
	for (_uint iMeshIndex = 0; iMeshIndex < m_iNumMeshes; ++iMeshIndex)
	{
		auto& pVertex = m_vecVertexPos[iMeshIndex];
		for (_uint i = 0; i < pVertex.size(); ++i)
		{
			pMax[0] = max(pVertex[i].x, pMax[0]);
			pMax[1] = max(pVertex[i].y, pMax[1]);
			pMax[2] = max(pVertex[i].z, pMax[2]);

			pMin[0] = min(pVertex[i].x, pMin[0]);
			pMin[1] = min(pVertex[i].y, pMin[1]);
			pMin[2] = min(pVertex[i].z, pMin[2]);
		}
	}

	_float3 vCenter = _float3(0.f, 0.f, 0.f);
	_float3 vExtends = {};

	vCenter.x = (pMax[0] + pMin[0]) * 0.5f;
	vCenter.y = (pMax[1] + pMin[1]) * 0.5f;
	vCenter.z = (pMax[2] + pMin[2]) * 0.5f;

	vExtends.x = (pMax[0] - pMin[0]) * 0.5f;
	vExtends.y = (pMax[1] - pMin[1]) * 0.5f;
	vExtends.z = (pMax[2] - pMin[2]) * 0.5f;

	if (!m_pBoundingBox)
		m_pBoundingBox = new BoundingBox(vCenter, vExtends);

	Safe_Delete_Array(pMin);
	Safe_Delete_Array(pMax);
}

_bool CMesh_Streaming::Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance)
{
	_float fMin = FLT_MAX;
	for (_uint iMeshIndex = 0; iMeshIndex < m_iNumMeshes; ++iMeshIndex)
	{
		for (size_t i = 0; i < m_vecIndices[iMeshIndex].size() - 2; i += 3)
		{
			auto& pVertex = m_vecVertexPos[iMeshIndex];
			_float3 vPos[3] = {
				pVertex[m_vecIndices[iMeshIndex][i]],
				pVertex[m_vecIndices[iMeshIndex][i + 1]],
				pVertex[m_vecIndices[iMeshIndex][i + 2]],
			};
			_float fDistance = {};
			if (true == TriangleTests::Intersects(vRayPos, vRayDir,
				XMVectorSetW(XMLoadFloat3(&vPos[0]), 1.f),
				XMVectorSetW(XMLoadFloat3(&vPos[1]), 1.f),
				XMVectorSetW(XMLoadFloat3(&vPos[2]), 1.f), fDistance))
			{
				if (fMin > fDistance)
					fMin = fDistance;
			}
		}
		if (fMin < FLT_MAX)
		{
			*pDistance = fMin;
			return true;
		}

	}
	return false;
}
#endif
CMesh_Streaming* CMesh_Streaming::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumMeshes)
{
	CMesh_Streaming* pInstance = new CMesh_Streaming(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(iNumMeshes)))
	{
		MSG_BOX("Failed to Create : Mesh_Streaming");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMesh_Streaming::Clone(void* pArg)
{
	CMesh_Streaming* pInstance = new CMesh_Streaming(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Mesh_Streaming (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMesh_Streaming::Free()
{
	__super::Free();
	Safe_Release(m_pSharedVB);
	Safe_Release(m_pSharedIB);
	if (!m_isClone)
		Safe_Delete(m_Desc);
	Safe_Delete_Array(m_vecVertexPos);
	Safe_Delete_Array(m_vecIndices);

#ifdef _DEBUG
	Safe_Delete(m_pBoundingBox);
#endif
}