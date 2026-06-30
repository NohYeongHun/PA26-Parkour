#include "EnginePch.h"
#include "VIBuffer.h"

CVIBuffer::CVIBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent { pDevice, pContext }
{
}

CVIBuffer::CVIBuffer(const CVIBuffer& Prototype)
    : CComponent { Prototype },
    m_pVB { Prototype.m_pVB }, m_pIB { Prototype.m_pIB },
    m_iNumVertices { Prototype.m_iNumVertices },
    m_iVertexStride { Prototype.m_iVertexStride },
    m_iNumVertexBuffers { Prototype.m_iNumVertexBuffers },
    m_iNumIndices { Prototype.m_iNumIndices },
    m_iIndexStride { Prototype.m_iIndexStride },
    m_eIndexFormat { Prototype.m_eIndexFormat },
    m_ePrimitiveType { Prototype.m_ePrimitiveType }
{
    Safe_AddRef(m_pVB);
    Safe_AddRef(m_pIB);
}

HRESULT CVIBuffer::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CVIBuffer::Initialize_Clone(void* pArg)
{

    return S_OK;
}

HRESULT CVIBuffer::Render()
{
    m_pContext->DrawIndexed(m_iNumIndices, 0, 0);

    return S_OK;
}

HRESULT CVIBuffer::Render(ID3D11DeviceContext* pDC)
{
	pDC->DrawIndexed(m_iNumIndices, 0, 0);
	return S_OK;
}

HRESULT CVIBuffer::Bind_Resources()
{
    ID3D11Buffer* Buffers[] = {
        m_pVB,
    };

    _uint Strides[] = {
        m_iVertexStride,
    };

    _uint Offsets[] = {
        0,
    };

    m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, Buffers, Strides, Offsets);
    m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
    m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

    return S_OK;
}

HRESULT CVIBuffer::Bind_Resources(ID3D11DeviceContext* pDC)
{
	ID3D11Buffer* Buffers[] = {
		m_pVB,
	};

	_uint Strides[] = {
		m_iVertexStride,
	};

	_uint Offsets[] = {
		0,
	};

	pDC->IASetVertexBuffers(0, m_iNumVertexBuffers, Buffers, Strides, Offsets);
	pDC->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	pDC->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

void CVIBuffer::Free()
{
    __super::Free();

    Safe_Release(m_pVB);
    Safe_Release(m_pIB);
}
