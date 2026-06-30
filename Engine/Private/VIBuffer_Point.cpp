#include "EnginePch.h"
#include "VIBuffer_Point.h"

CVIBuffer_Point::CVIBuffer_Point(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
:CVIBuffer(pDevice,pContext)
{
}

CVIBuffer_Point::CVIBuffer_Point(const CVIBuffer_Point& Prototype)
    :CVIBuffer(Prototype)
{
}

HRESULT CVIBuffer_Point::Initialize_Prototype()
{
    m_ePrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    m_iNumVertexBuffers = 1;
    m_iNumVertices = 1;
    m_iVertexStride = sizeof(VTXPOS);


    D3D11_BUFFER_DESC		VBDesc{};
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = m_iVertexStride;

    VTXPOS* pVertices = new VTXPOS[m_iNumVertices];

    pVertices[0].vPosition = _float3(0.0f, 0.0f, 0.f);

    D3D11_SUBRESOURCE_DATA	VBInitialData{};
    VBInitialData.pSysMem = pVertices;

    if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
        return E_FAIL;

    Safe_Delete_Array(pVertices);

    return S_OK;
}

HRESULT CVIBuffer_Point::Initialize_Clone(void* pArg)
{
    return S_OK;
}

HRESULT CVIBuffer_Point::Render()
{
    m_pContext->Draw(m_iNumVertices, 0);
    return S_OK;
}

HRESULT CVIBuffer_Point::Bind_Resources()
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
    m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

    return S_OK;
}
CVIBuffer_Point* CVIBuffer_Point::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CVIBuffer_Point* pInstance = new CVIBuffer_Point(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : VIBuffer_Point");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CVIBuffer_Point::Clone(void* pArg)
{
    CVIBuffer_Point* pClone = new CVIBuffer_Point(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : VIBuffer_Point (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CVIBuffer_Point::Free()
{
    __super::Free();
}
