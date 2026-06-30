#include"EnginePch.h"
#include "VIBuffer_Instance.h"

CVIBuffer_Instance::CVIBuffer_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CVIBuffer(pDevice,pContext)
{
}

CVIBuffer_Instance::CVIBuffer_Instance(const CVIBuffer_Instance& Prototype)
    :CVIBuffer(Prototype)
    , m_iNumInstance(Prototype.m_iNumInstance)
    , m_iNumIndexPerInstance(Prototype.m_iNumIndexPerInstance)
    , m_iInstanceVertexStride(Prototype.m_iInstanceVertexStride)
    , m_VBInstanceDesc(Prototype.m_VBInstanceDesc)
    , m_pVBInstanceVertices(Prototype.m_pVBInstanceVertices)
{
}

HRESULT CVIBuffer_Instance::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CVIBuffer_Instance::Initialize_Prototype(const INSTANCE_DESC* pDesc)
{
	return S_OK;;
}

HRESULT CVIBuffer_Instance::Initialize_Clone(void* pArg)
{
	D3D11_SUBRESOURCE_DATA InitialDesc;
	InitialDesc.pSysMem = m_pVBInstanceVertices;

	if (FAILED(m_pDevice->CreateBuffer(&m_VBInstanceDesc, &InitialDesc, &m_pVBInstance)))
		return E_FAIL;

	return S_OK;
}	

HRESULT CVIBuffer_Instance::Bind_Resources()
{
	ID3D11Buffer* pVertexBuffers[] = {
		m_pVB,
		m_pVBInstance,
	};

	_uint		iVertexStrides[] = {
		m_iVertexStride,
		m_iInstanceVertexStride,
	};

	_uint		iOffsets[] = {
		0,
		0,
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT CVIBuffer_Instance::Bind_Resources(ID3D11DeviceContext* pDC)
{
	ID3D11Buffer* pVertexBuffers[] = {
		m_pVB,
		m_pVBInstance,
	};

	_uint		iVertexStrides[] = {
		m_iVertexStride,
		m_iInstanceVertexStride,
	};

	_uint		iOffsets[] = {
		0,
		0,
	};

	pDC->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	pDC->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	pDC->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT CVIBuffer_Instance::Render()
{
	m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstance, 0, 0, 0);

	return S_OK;

}

HRESULT CVIBuffer_Instance::Render(ID3D11DeviceContext* pDC)
{
	pDC->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstance, 0, 0, 0);
	return S_OK;
}

void CVIBuffer_Instance::Free()
{
	__super::Free();

	//여기서 파티클 똥 발생함, 원형에서 만드는데 클론만 지워서 원형꺼 안지워짐.
	if (!m_isClone)
		Safe_Delete_Array(m_pVBInstanceVertices);

	Safe_Release(m_pVBInstance);
}
