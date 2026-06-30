#include "EnginePch.h"
#include "MeshAnim_Instance.h"

#include "Bone.h"
#include "Shader.h"

#include"VIBuffer_Cube.h"

CMeshAnim_Instance::CMeshAnim_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer_Instance { pDevice, pContext }
{
}

CMeshAnim_Instance::CMeshAnim_Instance(const CMeshAnim_Instance& Prototype)
    : CVIBuffer_Instance{ Prototype }
    , m_VertexPositions { Prototype.m_VertexPositions }
    , m_Indices { Prototype.m_Indices }
	, m_iNumMaxInstance{ Prototype.m_iNumMaxInstance }
	, m_OffsetMatrices{ Prototype.m_OffsetMatrices }
	, m_iNumBones{ Prototype.m_iNumBones }
	, m_iMaterialIndex { Prototype.m_iMaterialIndex }
	, m_pBoneLocalIdxBuf{ Prototype.m_pBoneLocalIdxBuf }
	, m_pBoneLocalIdxSRV{ Prototype.m_pBoneLocalIdxSRV }
{
	if (nullptr == m_pBoneLocalIdxBuf)
		CRASH("Bone Local Index Buffer is nullptr");
	Safe_AddRef(m_pBoneLocalIdxBuf);
	if (nullptr == m_pBoneLocalIdxSRV)
		CRASH("Bone Local Index SRV is nullptr");
	Safe_AddRef(m_pBoneLocalIdxSRV);
}

HRESULT CMeshAnim_Instance::Initialize_Prototype(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _uint iNumInstance, _uint iMatPadding)
{
	m_iNumInstance = m_iNumMaxInstance = iNumInstance;
	m_iNumVertexBuffers = 2;
    if (FAILED(Ready_Mesh_Anim(Bones, PreTransformMatrix, InputFile, iMatPadding)))
        return E_FAIL;
    
#pragma region INSTANCE
	m_iNumIndexPerInstance = m_iNumIndices;
	m_iInstanceVertexStride = sizeof(VTXINSTANCE_ANIMMESH);
	m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
	m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_VBInstanceDesc.MiscFlags = 0;
	m_VBInstanceDesc.StructureByteStride = m_iInstanceVertexStride;

	m_pVBInstanceVertices = new VTXINSTANCE_ANIMMESH[m_iNumInstance];
	VTXINSTANCE_ANIMMESH* pVBInstanceVertices = static_cast<VTXINSTANCE_ANIMMESH*>(m_pVBInstanceVertices);
	//memcpy(pVBInstanceVertices, pDesc->pTransformMatrix, sizeof(_float4x4) * m_iNumInstance);
	ZeroMemory(pVBInstanceVertices, sizeof(VTXINSTANCE_ANIMMESH) * m_iNumInstance);
#pragma endregion

    return S_OK;
}

HRESULT CMeshAnim_Instance::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
    return S_OK;
}

HRESULT CMeshAnim_Instance::Render()
{
	if (m_iNumInstance > 0)
	{
		m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstance, 0, 0, 0);
	}

	return S_OK;
}

HRESULT CMeshAnim_Instance::Render(ID3D11DeviceContext* pDC)
{
	return __super::Render(pDC);
}

#ifdef _DEBUG
_bool CMeshAnim_Instance::Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance)
{
    _float fMin = FLT_MAX;
    for (size_t i = 0; i < m_Indices.size() - 2; i += 3)
    {
        _float3 vPos[3] = {
            m_VertexPositions[m_Indices[i]],
            m_VertexPositions[m_Indices[i+1]],
            m_VertexPositions[m_Indices[i+2]],
        };
        _float fDistance = {};
        if (true == TriangleTests::Intersects(vRayPos, vRayDir, 
            XMVectorSetW(XMLoadFloat3(&vPos[0]), 1.f), 
            XMVectorSetW(XMLoadFloat3(&vPos[1]), 1.f),
            XMVectorSetW(XMLoadFloat3(&vPos[2]), 1.f), fDistance))
        {
            if (fMin > fDistance)
                fMin = fDistance;
        }
    }
    if (fMin < FLT_MAX)
    {
        *pDistance = fMin;
        return true;
    }

    return false;
}
#endif

// 
HRESULT CMeshAnim_Instance::Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, const vector<class CBone*>& Bones)
{
    for (size_t i = 0; i < m_iNumBones; ++i)
    {
        XMStoreFloat4x4(&m_BoneMatrices[i], XMLoadFloat4x4(&m_OffsetMatrices[i]) * XMLoadFloat4x4(Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrix()));
    }

    return pShader->Bind_Matrices(pConstantName, m_BoneMatrices, m_iNumBones);
}

HRESULT CMeshAnim_Instance::Bind_OffsetMatrix(CShader* pShader, const _char* pConstantName)
{
	if(FAILED(pShader->Bind_Texture("g_MeshLocalBoneIndecies", m_pBoneLocalIdxSRV)))
		return E_FAIL;
	return pShader->Bind_Matrices(pConstantName, m_OffsetMatrices.data(), m_OffsetMatrices.size());
}

HRESULT CMeshAnim_Instance::Update_InstanceData(VTXINSTANCE_ANIMMESH* pInstanceDatas, _uint iNumRenderCount)
{
	if (iNumRenderCount > m_iNumMaxInstance)
	{
		CRASH("Instance Count Exceeding Max Number");
		return E_FAIL;
	}
	m_iNumInstance = iNumRenderCount;
	if (nullptr == pInstanceDatas)
	{
		//iNumRenderCount = 0
		//해당 매쉬는 렌더링하지 않음
		return S_OK;
	}
	D3D11_MAPPED_SUBRESOURCE SubResource{};
	//VTXINSTANCE_ANIMMESH* pInstanceVertices = static_cast<VTXINSTANCE_ANIMMESH*>(m_pVBInstanceVertices);
	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);
	VTXINSTANCE_ANIMMESH* pVertices = static_cast<VTXINSTANCE_ANIMMESH*>(SubResource.pData);
	memcpy(pVertices, pInstanceDatas, sizeof(VTXINSTANCE_ANIMMESH) * iNumRenderCount);
	m_pContext->Unmap(m_pVBInstance, 0);

	
	return S_OK;
}

HRESULT CMeshAnim_Instance::Ready_Mesh_Anim(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _uint iMatPadding)
{
    VTXANIMMESH* pVertices = { nullptr };
    _uint* pIndices = { nullptr };

    InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
    pVertices = new VTXANIMMESH[m_iNumVertices];
    ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);
    InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
    m_iNumIndices = m_iNumIndices * 3;
    pIndices = new _uint[m_iNumIndices];
    InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));
	m_iMaterialIndex += iMatPadding;
    InputFile.read(reinterpret_cast<_char*>(&m_iNumBones), sizeof(_uint));

    for (size_t i = 0; i < m_iNumBones; ++i)
    {
        _uint iLength = {};
        // Bone Name Length
        InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
        _char szName[MAX_PATH] = {};
        // Bone Name
        InputFile.read(szName, iLength);
        auto iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool {
            return 0 == strcmp(szName, pBone->Get_Name());
            });

        if (iter == Bones.end())
            return E_FAIL;

		//m_BoneIndices[iter - Bones.begin()] = i;
		m_BoneIndices.push_back(iter - Bones.begin());
        // OffsetMatrix
        _float4x4 OffsetMatrix = {};
        InputFile.read(reinterpret_cast<_char*>(&OffsetMatrix), sizeof(_float4x4));
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));
		m_OffsetMatrices.push_back(OffsetMatrix);
    }

    if (0 == m_iNumBones)
    {
        _float4x4 OffsetMatrix = {};
        XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());
        m_OffsetMatrices.push_back(OffsetMatrix);
        m_BoneIndices.push_back(0);
    }

    InputFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXANIMMESH) * m_iNumVertices);
    InputFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

#pragma region VERTEX
    m_iVertexStride = sizeof(VTXANIMMESH);

    D3D11_BUFFER_DESC   VBDesc = {};
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = m_iVertexStride;

    D3D11_SUBRESOURCE_DATA VBInitialData = {};
    VBInitialData.pSysMem = pVertices;

    if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
        return E_FAIL;

    Safe_Delete_Array(pVertices);
#pragma endregion

#pragma region INDEX
    m_iIndexStride = 4;
    m_eIndexFormat = DXGI_FORMAT_R32_UINT;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    D3D11_BUFFER_DESC IBDesc = {};
    IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBDesc.CPUAccessFlags = 0;
    IBDesc.MiscFlags = 0;
    IBDesc.StructureByteStride = m_iIndexStride;

    D3D11_SUBRESOURCE_DATA IBInitialData = {};
    IBInitialData.pSysMem = pIndices;

    if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
        return E_FAIL;

    Safe_Delete_Array(pIndices);
#pragma endregion

	D3D11_BUFFER_DESC BoneLocalIdxBufDesc = {};
	BoneLocalIdxBufDesc.ByteWidth = sizeof(_uint) * m_BoneIndices.size();
	BoneLocalIdxBufDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BoneLocalIdxBufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BoneLocalIdxBufDesc.CPUAccessFlags = 0;
	BoneLocalIdxBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BoneLocalIdxBufDesc.StructureByteStride = sizeof(_uint);
	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = m_BoneIndices.data();
	if (FAILED(m_pDevice->CreateBuffer(&BoneLocalIdxBufDesc, &subresourceData, &m_pBoneLocalIdxBuf)))
		return E_FAIL;
	if (FAILED(m_pDevice->CreateShaderResourceView(m_pBoneLocalIdxBuf, nullptr, &m_pBoneLocalIdxSRV)))
		return E_FAIL;

    return S_OK;
}

CMeshAnim_Instance* CMeshAnim_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _uint iNumInstance, _uint iMatPadding)
{
	CMeshAnim_Instance* pInstance = new CMeshAnim_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(Bones, PreTransformMatrix, InputFile, iNumInstance, iMatPadding)))
	{
		MSG_BOX("Failed to Create : Mesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMeshAnim_Instance::Clone(void* pArg)
{
	CMeshAnim_Instance* pClone = new CMeshAnim_Instance(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Mesh (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMeshAnim_Instance::Free()
{
	__super::Free();

	Safe_Release(m_pBoneLocalIdxSRV);
	Safe_Release(m_pBoneLocalIdxBuf);
}
