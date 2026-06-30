#include "EnginePch.h"
#include "VIBuffer_Rect_Instance.h"

CVIBuffer_Rect_Instance::CVIBuffer_Rect_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Rect_Instance::CVIBuffer_Rect_Instance(const CVIBuffer_Rect_Instance& Prototype)
	: CVIBuffer{ Prototype }
	, m_iNumIndexPerInstance{ Prototype.m_iNumIndexPerInstance }
	, m_iInstanceVertexStride{ Prototype.m_iInstanceVertexStride }
	, m_VBInstanceDesc{ Prototype.m_VBInstanceDesc }
	, m_iMaxInstance{ Prototype.m_iMaxInstance }
{
}

HRESULT CVIBuffer_Rect_Instance::Initialize_Prototype(_uint iMaxInstance)
{
	m_iMaxInstance = iMaxInstance;

	m_iNumVertices = 4;
	m_iVertexStride = sizeof(VTXPOSTEX);
	m_iNumVertexBuffers = 2;
	m_iNumIndices = 6;
	m_iIndexStride = 2;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_iNumIndexPerInstance = m_iNumIndices;
	m_iInstanceVertexStride = sizeof(VTXINSTANCE_RECT);

#pragma region VERTEX

	D3D11_BUFFER_DESC   VBDesc = {};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
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

	D3D11_SUBRESOURCE_DATA VBInitialData = {};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
#pragma endregion

#pragma region INDEX

	D3D11_BUFFER_DESC IBDesc = {};
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

	D3D11_SUBRESOURCE_DATA IBInitialData = {};
	IBInitialData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);
#pragma endregion

#pragma region INSTANCE
	m_VBInstanceDesc.ByteWidth = iMaxInstance * m_iInstanceVertexStride;
	m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_VBInstanceDesc.MiscFlags = 0;
	m_VBInstanceDesc.StructureByteStride = m_iInstanceVertexStride;
#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Rect_Instance::Initialize_Clone(void* pArg)
{
	if (FAILED(m_pDevice->CreateBuffer(&m_VBInstanceDesc, nullptr, &m_pVBInstance)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Rect_Instance::Render()
{
	m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstance, 0, 0, 0);

	return S_OK;
}

HRESULT CVIBuffer_Rect_Instance::Bind_Resources()
{
	ID3D11Buffer* pVertexBuffers[] = {
		  m_pVB,
		  m_pVBInstance,
	};

	_uint iVertexStrides[] = {
		m_iVertexStride,
		m_iInstanceVertexStride ,
	};

	_uint iOffset[] = {
		0,
		0,
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffset);
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT CVIBuffer_Rect_Instance::Update_Buffer(const vector<VTXINSTANCE_RECT>& Datas)
{
	m_iNumInstance = Datas.size();

	if (m_iNumInstance >= m_iMaxInstance)
		m_iNumInstance = m_iMaxInstance;

	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);

	memcpy(SubResource.pData, Datas.data(), sizeof(VTXINSTANCE_RECT) * m_iNumInstance);

	m_pContext->Unmap(m_pVBInstance, 0);

	return S_OK;
}

void CVIBuffer_Rect_Instance::Clear()
{
	m_iNumInstance = 0;
}

CVIBuffer_Rect_Instance* CVIBuffer_Rect_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iMaxInstance)
{
	CVIBuffer_Rect_Instance* pInstance = new CVIBuffer_Rect_Instance(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype(iMaxInstance)))
	{
		MSG_BOX("Failed to Created : CVIBuffer_Rect_Instance");
		Safe_Release(pInstance);
	}
    return pInstance;
}

CComponent* CVIBuffer_Rect_Instance::Clone(void* pArg)
{
	CVIBuffer_Rect_Instance* pInstance = new CVIBuffer_Rect_Instance(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CVIBuffer_Rect_Instance");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CVIBuffer_Rect_Instance::Free()
{
	__super::Free();

	Safe_Release(m_pVBInstance);
}
