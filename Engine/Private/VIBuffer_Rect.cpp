#include "EnginePch.h"
#include "VIBuffer_Rect.h"

CVIBuffer_Rect::CVIBuffer_Rect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Rect::CVIBuffer_Rect(const CVIBuffer_Rect& Prototype)
    : CVIBuffer { Prototype }
{
}

HRESULT CVIBuffer_Rect::Initialize_Prototype()
{

#pragma region VERTEX
     m_iNumVertices = 4;
    m_iVertexStride = sizeof(VTXPOSTEX);
    m_iNumVertexBuffers = 1;

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
    m_iNumIndices = 6;
    m_iIndexStride = 2;
    m_eIndexFormat = DXGI_FORMAT_R16_UINT;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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

    return S_OK;
}

HRESULT CVIBuffer_Rect::Initialize_Clone(void* pArg)
{
    return S_OK;
}

CVIBuffer_Rect* CVIBuffer_Rect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CVIBuffer_Rect* pInstance = new CVIBuffer_Rect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : VIBuffer_Rect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CVIBuffer_Rect::Clone(void* pArg)
{
    CVIBuffer_Rect* pClone = new CVIBuffer_Rect(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : VIBuffer_Rect (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CVIBuffer_Rect::Free()
{
    __super::Free();
}
