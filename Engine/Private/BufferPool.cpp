#include"EnginePch.h"
#include "BufferPool.h"
#include"GameInstance.h"
CBufferPool::CBufferPool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:m_pGameInstance(CGameInstance::GetInstance()),
	m_pDevice(pDevice),
	m_pContext(pContext)
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CBufferPool::Initialize(_uint iVertexSize, _uint iIndexSize)
{
	D3D11_BUFFER_DESC VBDesc{};

	_uint iMegaBite = 1024 * 1024;
	VBDesc.ByteWidth = iVertexSize * iMegaBite;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, nullptr, &m_pVertexBufferPool)))
		CRASH("Failed");



	D3D11_BUFFER_DESC IBDesc{};
	IBDesc.ByteWidth = iIndexSize * iMegaBite;
	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = m_iVertexStride;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, nullptr, &m_pIndexBufferPool)))
		CRASH("Failed");

	m_pVertexFreeList = CFreeList::Create(iVertexSize);
	m_pIndexFreeList = CFreeList::Create(iIndexSize);

    return S_OK;
}

_uint CBufferPool::Allocate_Vertex(_uint iVertexSize)
{
	return m_pVertexFreeList->Allocate(iVertexSize);
}

_uint CBufferPool::Allocate_Index(_uint iIndexSize)
{
	return m_pIndexFreeList->Allocate(iIndexSize);
}

void CBufferPool::FreeMemory_Vertex(_uint iVertexOffset, _uint iVertexSize)
{
	m_pVertexFreeList->Free(iVertexOffset, iVertexSize);
}

void CBufferPool::FreeMemory_Index(_uint iIndexOffSet, _uint iIndexSize)
{
	m_pIndexFreeList->Free(iIndexOffSet, iIndexSize);
}

HRESULT CBufferPool::Bind_BufferPool()
{
	ID3D11Buffer* Buffers[] = {
m_pVertexBufferPool,
	};

	_uint Strides[] = {
		m_iVertexStride,
	};

	_uint Offsets[] = {
		0,
	};
	m_pContext->IASetVertexBuffers(0, 1, Buffers, Strides, Offsets);
	m_pContext->IASetIndexBuffer(m_pIndexBufferPool, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT CBufferPool::Bind_BufferPool(ID3D11DeviceContext* pDC)
{
	ID3D11Buffer* Buffers[] = {
		m_pVertexBufferPool,
	};

	_uint Strides[] = {
		m_iVertexStride,
	};

	_uint Offsets[] = {
		0,
	};
	pDC->IASetVertexBuffers(0, 1, Buffers, Strides, Offsets);
	pDC->IASetIndexBuffer(m_pIndexBufferPool, m_eIndexFormat, 0);
	pDC->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

void CBufferPool::Clear_Resource()
{
	m_pVertexFreeList->Clear_Resource();
	m_pIndexFreeList->Clear_Resource();
}


CBufferPool* CBufferPool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iVertexSize, _uint iIndexSize)
{
	CBufferPool* pInstance = new CBufferPool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iVertexSize, iIndexSize)))
	{
		MSG_BOX("Failed to Create : BufferPool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBufferPool::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pVertexFreeList);
	Safe_Release(m_pIndexFreeList);
	Safe_Release(m_pVertexBufferPool);
	Safe_Release(m_pIndexBufferPool);
}
