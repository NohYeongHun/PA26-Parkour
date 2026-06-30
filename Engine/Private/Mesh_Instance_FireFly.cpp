#include"EnginePch.h"
#include "Mesh_Instance_FireFly.h"
#include "GameInstance.h"

CMesh_Instance_FireFly::CMesh_Instance_FireFly(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CVIBuffer_Instance(pDevice, pContext)
{
}

CMesh_Instance_FireFly::CMesh_Instance_FireFly(const CMesh_Instance_FireFly& Prototype)
	:CVIBuffer_Instance(Prototype),
	m_IsEdit{ Prototype.m_IsEdit }
{
}

HRESULT CMesh_Instance_FireFly::Initialize_Prototype(_fmatrix PreTransformMatrix, _bool IsEdit, void* pArg, ifstream& InputFile, _float* MinPos, _float* MaxPos)
{
	VTXMESH* pVertices = { nullptr };
	_uint* pIndices = { nullptr };
	m_IsEdit = IsEdit;
	m_iVertexStride = sizeof(VTXMESH);
	m_iNumVertexBuffers = 2;

	InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
	pVertices = new VTXMESH[m_iNumVertices];
	InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
	m_iNumIndices = m_iNumIndices * 3;
	m_iNumIndexPerInstance = m_iNumIndices;
	pIndices = new _uint[m_iNumIndices];
	InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));

	InputFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXMESH) * m_iNumVertices);
	InputFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

	for (size_t i = 0; i < m_iNumVertices; ++i)
	{
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));

		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vTangent, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));
#ifdef _DEBUG
		MaxPos[0] = max(pVertices[i].vPosition.x, MaxPos[0]);
		MaxPos[1] = max(pVertices[i].vPosition.y, MaxPos[1]);
		MaxPos[2] = max(pVertices[i].vPosition.z, MaxPos[2]);

		MinPos[0] = min(pVertices[i].vPosition.x, MinPos[0]);
		MinPos[1] = min(pVertices[i].vPosition.y, MinPos[1]);
		MinPos[2] = min(pVertices[i].vPosition.z, MinPos[2]);
		m_VertexPositions.push_back(pVertices[i].vPosition);
#endif
	}


	D3D11_BUFFER_DESC   VBDesc = {};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	D3D11_SUBRESOURCE_DATA VBInitialData = {};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
#pragma endregion

#ifdef _DEBUG
	for (size_t i = 0; i < m_iNumIndices; ++i)
		m_Indices.push_back(pIndices[i]);
#endif

#pragma region INDEX
	m_iIndexStride = 4;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_BUFFER_DESC IBDesc = {};
	IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = m_iIndexStride;

	D3D11_SUBRESOURCE_DATA IBInitialData = {};
	IBInitialData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

	if (!m_IsEdit)
	{
		CMesh_Instance_FireFly::MESH_INST_DESC* pDesc = static_cast<CMesh_Instance_FireFly::MESH_INST_DESC*>(pArg);
		//MESH_INST_DESC* pDesc = static_cast<MESH_INST_DESC*>(pArg);
		if (!pDesc)
			CRASH("Failed");

		m_iNumInstance = pDesc->iNumInstance;
		//m_TransformMatrices = pDesc->pTransformMatrix;
		m_iInstanceVertexStride = sizeof(VTXINSTANCE_MESH_FIREFLY);
		m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
		m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
		m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		m_VBInstanceDesc.MiscFlags = 0;
		m_VBInstanceDesc.StructureByteStride = m_iInstanceVertexStride;

		m_pVBInstanceVertices = new VTXINSTANCE_MESH_FIREFLY[m_iNumInstance];
		VTXINSTANCE_MESH_FIREFLY* pVBInstanceVertices = static_cast<VTXINSTANCE_MESH_FIREFLY*>(m_pVBInstanceVertices);
		for (_uint i = 0; i < m_iNumInstance; ++i)
		{
			memcpy(&pVBInstanceVertices[i].vRight, &pDesc->pTransformMatrix[i].m[0], sizeof(_float4));
			memcpy(&pVBInstanceVertices[i].vUp, &pDesc->pTransformMatrix[i].m[1], sizeof(_float4));
			memcpy(&pVBInstanceVertices[i].vLook, &pDesc->pTransformMatrix[i].m[2], sizeof(_float4));
			memcpy(&pVBInstanceVertices[i].vTranslation, &pDesc->pTransformMatrix[i].m[3], sizeof(_float4));

			pVBInstanceVertices[i].vPerMove.x = m_pGameInstance->Rand(pDesc->vPerMoveCos.x, pDesc->vPerMoveCos.y);
			pVBInstanceVertices[i].vPerMove.y = m_pGameInstance->Rand(pDesc->vPerMoveSin.x, pDesc->vPerMoveSin.y);
			pVBInstanceVertices[i].vPerMove.z = m_pGameInstance->Rand(pDesc->vPerMoveSin2.x, pDesc->vPerMoveSin2.y);
			pVBInstanceVertices[i].vRange = m_pGameInstance->Rand(pDesc->vRange.x, pDesc->vRange.y);
		}
	}
	return S_OK;
}

HRESULT CMesh_Instance_FireFly::Initialize_Clone(void* pArg)
{
	if (m_IsEdit)
	{
		if (m_pVBInstance)
			Safe_Release(m_pVBInstance);

		MESH_INST_DESC* pDesc = static_cast<MESH_INST_DESC*>(pArg);

		m_iNumInstance = pDesc->iNumInstance;
		//m_TransformMatrices = pDesc->pTransformMatrix;
		m_iInstanceVertexStride = sizeof(VTXINSTANCE_MESH_FIREFLY);
		m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
		m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
		m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		m_VBInstanceDesc.MiscFlags = 0;
		m_VBInstanceDesc.StructureByteStride = m_iInstanceVertexStride;

		m_pVBInstanceVertices = new VTXINSTANCE_MESH_FIREFLY[m_iNumInstance];
		VTXINSTANCE_MESH_FIREFLY* pVBInstanceVertices = static_cast<VTXINSTANCE_MESH_FIREFLY*>(m_pVBInstanceVertices);

		for (_uint i = 0; i < m_iNumInstance; ++i)
		{
			memcpy(&pVBInstanceVertices[i].vRight, &pDesc->pTransformMatrix[i].m[0], sizeof(_float4));
			memcpy(&pVBInstanceVertices[i].vUp, &pDesc->pTransformMatrix[i].m[1], sizeof(_float4));
			memcpy(&pVBInstanceVertices[i].vLook, &pDesc->pTransformMatrix[i].m[2], sizeof(_float4));
			memcpy(&pVBInstanceVertices[i].vTranslation, &pDesc->pTransformMatrix[i].m[3], sizeof(_float4));

			pVBInstanceVertices[i].vPerMove.x = m_pGameInstance->Rand(pDesc->vPerMoveCos.x, pDesc->vPerMoveCos.y);
			pVBInstanceVertices[i].vPerMove.y = m_pGameInstance->Rand(pDesc->vPerMoveSin.x, pDesc->vPerMoveSin.y);
			pVBInstanceVertices[i].vPerMove.z = m_pGameInstance->Rand(pDesc->vPerMoveSin2.x, pDesc->vPerMoveSin2.y);
			pVBInstanceVertices[i].vRange = m_pGameInstance->Rand(pDesc->vRange.x, pDesc->vRange.y);
		}
	}

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CMesh_Instance_FireFly::Change_InstanceInfo(_uint iNumInstance, _fmatrix fMatrix)
{
	D3D11_MAPPED_SUBRESOURCE SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);
	VTXINSTANCE_MESH_FIREFLY* pVertices = static_cast<VTXINSTANCE_MESH_FIREFLY*>(SubResource.pData);
	memcpy(&pVertices[iNumInstance], &fMatrix, sizeof(_float4x4));
	m_pContext->Unmap(m_pVBInstance, 0);
}

void CMesh_Instance_FireFly::FireFly_Move(_fvector vPos)
{
	D3D11_MAPPED_SUBRESOURCE SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);
	VTXINSTANCE_MESH_FIREFLY* pVertices = static_cast<VTXINSTANCE_MESH_FIREFLY*>(SubResource.pData);
	for (_uint i = 0; i < m_iNumInstance; ++i)
		XMStoreFloat4(&pVertices[i].vTranslation, vPos);
	m_pContext->Unmap(m_pVBInstance, 0);
}
#endif

CMesh_Instance_FireFly* CMesh_Instance_FireFly::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PreTransformMatrix, _bool IsEdit, void* pArg, ifstream& InputFile, _float* MinPos, _float* MaxPos)
{
	CMesh_Instance_FireFly* pInstance = new CMesh_Instance_FireFly(pDevice, pContext);

	//혹시 메쉬 인스턴스 생성할 때 pArg 받으면 랜덤성 말고 정해진 대로 하게 할것.

	if (FAILED(pInstance->Initialize_Prototype(PreTransformMatrix, IsEdit, pArg, InputFile, MinPos, MaxPos)))
	{
		MSG_BOX("Failed to Create : Mesh_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CMesh_Instance_FireFly* CMesh_Instance_FireFly::Clone(void* pArg)
{
	return new CMesh_Instance_FireFly(*this);
}
void CMesh_Instance_FireFly::Free()
{
    __super::Free();
	if (m_IsEdit)
		Safe_Delete_Array(m_pVBInstanceVertices);
}
