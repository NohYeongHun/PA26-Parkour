#include "EnginePch.h"
#include "VIBuffer_Cube.h"

CVIBuffer_Cube::CVIBuffer_Cube(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer { pDevice, pContext }
{
}

CVIBuffer_Cube::CVIBuffer_Cube(const CVIBuffer_Cube& Prototype)
    : CVIBuffer { Prototype }
{
}

HRESULT CVIBuffer_Cube::Initialize_Prototype()
{

#pragma region VERTEX
     m_iNumVertices = 8;
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
	pVertices[0].vPosition = _float3(-0.5f, 0.5f, -0.5f);

	pVertices[1].vPosition = _float3(0.5f, 0.5f, -0.5f);

	pVertices[2].vPosition = _float3(0.5f, -0.5f, -0.5f);

	pVertices[3].vPosition = _float3(-0.5f, -0.5f, -0.5f);

	pVertices[4].vPosition = _float3(-0.5f, 0.5f, 0.5f);

	pVertices[5].vPosition = _float3(0.5f, 0.5f, 0.5f);

	pVertices[6].vPosition = _float3(0.5f, -0.5f, 0.5f);

	pVertices[7].vPosition = _float3(-0.5f, -0.5f, 0.5f);

    D3D11_SUBRESOURCE_DATA VBInitialData = {};
    VBInitialData.pSysMem = pVertices;

    if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
        return E_FAIL;

    Safe_Delete_Array(pVertices);
#pragma endregion

#pragma region INDEX
    m_iNumIndices = 36;
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

	/* +x */
	pIndices[0] = 1; pIndices[1] = 5; pIndices[2] = 6;
	pIndices[3] = 1; pIndices[4] = 6; pIndices[5] = 2;

	/* -x */
	pIndices[6] = 4; pIndices[7] = 0; pIndices[8] = 3;
	pIndices[9] = 4; pIndices[10] = 3; pIndices[11] = 7;

	/* +y */
	pIndices[12] = 4; pIndices[13] = 5; pIndices[14] = 1;
	pIndices[15] = 4; pIndices[16] = 1; pIndices[17] = 0;

	/* -y */
	pIndices[18] = 3; pIndices[19] = 2; pIndices[20] = 6;
	pIndices[21] = 3; pIndices[22] = 6; pIndices[23] = 7;

	/* +z */
	pIndices[24] = 5; pIndices[25] = 4; pIndices[26] = 7;
	pIndices[27] = 5; pIndices[28] = 7; pIndices[29] = 6;

	/* -z */
	pIndices[30] = 0; pIndices[31] = 1; pIndices[32] = 2;
	pIndices[33] = 0; pIndices[34] = 2; pIndices[35] = 3;

    D3D11_SUBRESOURCE_DATA IBInitialData = {};
    IBInitialData.pSysMem = pIndices;

    if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
        return E_FAIL;

    Safe_Delete_Array(pIndices);
#pragma endregion

    return S_OK;
}

HRESULT CVIBuffer_Cube::Initialize_Clone(void* pArg)
{
    return S_OK;
}

CVIBuffer_Cube* CVIBuffer_Cube::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CVIBuffer_Cube* pInstance = new CVIBuffer_Cube(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : VIBuffer_Cube");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CVIBuffer_Cube::Clone(void* pArg)
{
    CVIBuffer_Cube* pClone = new CVIBuffer_Cube(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : VIBuffer_Cube (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CVIBuffer_Cube::Free()
{
    __super::Free();
}
