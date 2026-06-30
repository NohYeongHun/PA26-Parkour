#include "EnginePch.h"
#include "VIBuffer_Decal.h"

CVIBuffer_Decal::CVIBuffer_Decal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer { pDevice, pContext }
{
}

HRESULT CVIBuffer_Decal::Initialize()
{
	return S_OK;
}

HRESULT CVIBuffer_Decal::Render()
{
	m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstance, 0, 0, 0);

	return S_OK;
}

HRESULT CVIBuffer_Decal::Bind_Resources()
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

HRESULT CVIBuffer_Decal::Update_Buffer(const vector<VTXINSTANCE_DECAL>& Datas)
{
	m_iNumInstance = Datas.size();

	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
		
	memcpy(SubResource.pData, Datas.data(), sizeof(VTXINSTANCE_DECAL) * m_iNumInstance);

	m_pContext->Unmap(m_pVBInstance, 0);
		
	return S_OK;
}

void CVIBuffer_Decal::Clear()
{
	m_iNumInstance = 0;
}

void CVIBuffer_Decal::Free()
{
	__super::Free();

	Safe_Release(m_pVBInstance);

}
