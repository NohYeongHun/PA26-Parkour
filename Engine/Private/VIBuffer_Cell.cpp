#include "EnginePch.h"
#include "VIBuffer_Cell.h"

CVIBuffer_Cell::CVIBuffer_Cell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Cell::CVIBuffer_Cell(const CVIBuffer_Cell& Prototype)
    : CVIBuffer { Prototype }
{
}

HRESULT CVIBuffer_Cell::Initialize(const _float3* pPoints)
{
#pragma region VERTEX
    m_iNumVertices = 3;
    m_iVertexStride = sizeof(VTXPOS);
    m_iNumVertexBuffers = 1;

    D3D11_BUFFER_DESC   VBDesc = {};
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = m_iVertexStride;

    VTXPOS* pVertices = new VTXPOS[m_iNumVertices];
    memcpy(pVertices, pPoints, sizeof(_float3) * m_iNumVertices);

    D3D11_SUBRESOURCE_DATA VBInitialData = {};
    VBInitialData.pSysMem = pVertices;

    if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
        return E_FAIL;

    Safe_Delete_Array(pVertices);
#pragma endregion

#pragma region INDEX
    m_iNumIndices = 3;
    m_iIndexStride = 2;
    m_eIndexFormat = DXGI_FORMAT_R16_UINT;
    //m_ePrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
    m_ePrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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
    //pIndices[3] = 0;

    D3D11_SUBRESOURCE_DATA IBInitialData = {};
    IBInitialData.pSysMem = pIndices;

    if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
        return E_FAIL;

    Safe_Delete_Array(pIndices);
#pragma endregion

    return S_OK;
}

CVIBuffer_Cell* CVIBuffer_Cell::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float3* pPoints)
{
    CVIBuffer_Cell* pInstance = new CVIBuffer_Cell(pDevice, pContext);

    if (FAILED(pInstance->Initialize(pPoints)))
    {
        MSG_BOX("Failed to Create : VIBuffer_Cell");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CVIBuffer_Cell::Clone(void* pArg)
{
    CVIBuffer_Cell* pClone = new CVIBuffer_Cell(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : VIBuffer_Cell (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CVIBuffer_Cell::Free()
{
    __super::Free();
}
