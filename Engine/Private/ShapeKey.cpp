#include "EnginePch.h"
#include "ShapeKey.h"

CShapeKey::CShapeKey(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CShapeKey::Initialize(const _char* pShapeKeyName, _uint iNumVertices, const vector<_float3>& vDeltaPos, const vector<_float3>& vDeltaNormal, const _fmatrix& PreTransformationMatrix)
{
	// 1. 이름 저장.
	strcpy_s(m_szName, pShapeKeyName);

	// 2. 정점 개수 저장.
	m_iNumVertices = iNumVertices;

	// 3. Delta Position 저장.
	m_DeltaPositions = vDeltaPos;

	// 4. Delta Normal 저장.
	m_DeltaNormals = vDeltaNormal;

	// 5. PreTransform 곱해주기. => 일단 해봅시다. (나중에 변위값이 이상하면 고치자)
	//for (_uint i = 0; i < m_iNumVertices; ++i)
	//	XMStoreFloat3(&m_DeltaPositions[i], XMVector3TransformCoord(XMLoadFloat3(&m_DeltaPositions[i]), PreTransformationMatrix));

	return S_OK;
}

void CShapeKey::Set_GlobalWeightIndex(_uint iIndex)
{
	m_iGlobalWeightIndex = iIndex;
}



CShapeKey* CShapeKey::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pShapeKeyName
	, _uint iNumVertices, const vector<_float3>& vDeltaPos, const vector<_float3>& vDeltaNormal, const _fmatrix& PreTransformationMatrix)
{
	CShapeKey* pInstance = new CShapeKey(pDevice, pContext);
	if (FAILED(pInstance->Initialize(pShapeKeyName, iNumVertices, vDeltaPos, vDeltaNormal, PreTransformationMatrix)))
	{
		MSG_BOX("Create ShapeKey Failed");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CShapeKey::Free()
{
	CBase::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	m_DeltaPositions.clear();
	m_DeltaNormals.clear();

}
