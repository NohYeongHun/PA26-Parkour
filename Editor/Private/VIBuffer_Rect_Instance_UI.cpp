#include "EditorPch.h"

#include "VIBuffer_Rect_Instance_UI.h"
#include "GameInstance.h"

CVIBuffer_Rect_Instance_UI::CVIBuffer_Rect_Instance_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer_Instance( pDevice, pContext )
{
}

CVIBuffer_Rect_Instance_UI::CVIBuffer_Rect_Instance_UI(const CVIBuffer_Rect_Instance_UI& Prototype)
	: CVIBuffer_Instance( Prototype )
	, m_vPivot{ Prototype.m_vPivot }
{
}

HRESULT CVIBuffer_Rect_Instance_UI::Initialize_Prototype(const INSTANCE_DESC* pDesc)
{
	const RECT_INSTANCE_UI_DESC* pInstDesc = static_cast<const RECT_INSTANCE_UI_DESC*>(pDesc);

	//m_vPivot = pInstDesc->vPivot;

	m_iNumIndexPerInstance = 6;
	m_iInstanceVertexStride = sizeof(SINGLE_INST_DESC); // ksta : DO NOT USE EMEMENT INCLUDED STRUCT !! IT EXCEEDS AND USE WRONG BUFFER SPACE
	m_iNumInstance = pInstDesc->iNumInstance;
	m_iNumVertices = 4;
	m_iVertexStride = sizeof(VTXPOSTEX);
	m_iNumIndices = 6;
	m_iIndexStride = 2;
	m_iNumVertexBuffers = 2;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_BUFFER_DESC		VBDesc{};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride; // VBDesc.ByteWidth = m_iNumVertices * m_iInstanceVertexStride;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;


	VTXPOSTEX* pVertices = new VTXPOSTEX[m_iNumVertices];

	pVertices[0].vPosition = _float3(-0.5f, 0.5f, 0.f);
	pVertices[0].vTexcoord = _float2(0.f, 0.f);

	pVertices[1].vPosition = _float3(0.5f, 0.5f, 0.f);
	pVertices[1].vTexcoord = _float2(1.f, 0.f);

	pVertices[2].vPosition = _float3(0.5f, -0.5f, 0.f);
	pVertices[2].vTexcoord = _float2(1.f, 1.f);

	pVertices[3].vPosition = _float3(-0.5f, -0.5f, 0.f);
	pVertices[3].vTexcoord = _float2(0.f, 1.f);

	D3D11_SUBRESOURCE_DATA	VBInitialData{};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	D3D11_BUFFER_DESC		IBDesc{};
	IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = m_iIndexStride;

	_ushort* pIndices = new _ushort[m_iNumIndices];

	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = 2;

	pIndices[3] = 0;
	pIndices[4] = 2;
	pIndices[5] = 3;

	D3D11_SUBRESOURCE_DATA	IBInitialData{};
	IBInitialData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);


	m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
	m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_VBInstanceDesc.MiscFlags = 0;
	m_VBInstanceDesc.StructureByteStride = m_iInstanceVertexStride;


	// �� ���̶� �Ҵ��ؾ� ������ ����..? ksta
	m_pVBInstanceVertices = new SINGLE_INST_DESC[m_iNumInstance];
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		SINGLE_INST_DESC* pInstanceVertices = static_cast<SINGLE_INST_DESC*>(m_pVBInstanceVertices);

		pInstanceVertices[i].vSInstRight	= { 1.f, 0.f, 0.f ,0.f };
		pInstanceVertices[i].vSInstUp		= { 0.f, 1.f, 0.f ,0.f };
		pInstanceVertices[i].vSInstLook		= { 0.f, 0.f, 1.f ,0.f };
		pInstanceVertices[i].vSInstTrans	= { 0.f, 0.f, 0.f ,1.f };

		pInstanceVertices[i].vSInstCoordX	= { 0.f, 1.f} ;
		pInstanceVertices[i].vSInstCoordY	= { 0.f, 1.f} ;

		pInstanceVertices[i].vClipTexcoordX = { 0.f, 1.f };			
		pInstanceVertices[i].vClipTexcoordY = { 0.f, 1.f };
	}

	return S_OK;
}

HRESULT CVIBuffer_Rect_Instance_UI::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}	

void CVIBuffer_Rect_Instance_UI::Update_Instances(_float fTimeDelta, vector<SINGLE_INST_DESC>& vecDescs)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	SINGLE_INST_DESC* pInstanceVertices = static_cast<SINGLE_INST_DESC*>(m_pVBInstanceVertices);

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource); // ksta : "D3D11_MAP_WRITE_DISCARD", "D3D11_MAP_WRITE_NO_OVERWRITE"
	SINGLE_INST_DESC* pVertices = static_cast<SINGLE_INST_DESC*>(SubResource.pData);

	m_iNumAvailableInstance = min(m_iNumInstance, static_cast<_uint>(vecDescs.size()));


	for (size_t i = 0; i < m_iNumAvailableInstance; i++)
	{


		// ���⼭ �� Instance�� ��ġ �� ���� ����`
		pVertices[i].vSInstRight	= vecDescs[i].vSInstRight;
		pVertices[i].vSInstUp		= vecDescs[i].vSInstUp   ;
		pVertices[i].vSInstLook		= vecDescs[i].vSInstLook ;
		pVertices[i].vSInstTrans	= vecDescs[i].vSInstTrans;

		pVertices[i].vSInstCoordX	= vecDescs[i].vSInstCoordX;
		pVertices[i].vSInstCoordY	= vecDescs[i].vSInstCoordY;

		pVertices[i].vClipTexcoordX = vecDescs[i].vClipTexcoordX;	// ���� �̻��
		pVertices[i].vClipTexcoordY = vecDescs[i].vClipTexcoordY;	// ���� �̻��

		pVertices[i].matExtraData	= vecDescs[i].matExtraData;
	}

	m_pContext->Unmap(m_pVBInstance, 0);

}


HRESULT CVIBuffer_Rect_Instance_UI::Render()
{
	m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumAvailableInstance, 0, 0, 0);

	return S_OK;

}


CVIBuffer_Rect_Instance_UI* CVIBuffer_Rect_Instance_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CVIBuffer_Rect_Instance_UI::INSTANCE_DESC* pDesc)
{
	CVIBuffer_Rect_Instance_UI* pInstance = new CVIBuffer_Rect_Instance_UI(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pDesc)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Rect_Instance_UI");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CVIBuffer_Rect_Instance_UI::Clone(void* pArg)
{
	CVIBuffer_Rect_Instance_UI* pInstance = new CVIBuffer_Rect_Instance_UI(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_Rect_Instance_UI (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Rect_Instance_UI::Free()
{
	//__super::Free();

	CVIBuffer::Free();

	if (!m_isClone)
		Safe_Delete_Array(m_pVBInstanceVertices);
	Safe_Release(m_pVBInstance);
}