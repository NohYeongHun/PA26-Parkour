#include "EnginePch.h"
#include "VIBuffer_FXMesh_Instance.h"
#include "GameInstance.h"

CVIBuffer_FXMesh_Instance::CVIBuffer_FXMesh_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CVIBuffer_Instance{ pDevice, pContext }
{
}

CVIBuffer_FXMesh_Instance::CVIBuffer_FXMesh_Instance(const CVIBuffer_FXMesh_Instance& Prototype)
    : CVIBuffer_Instance{ Prototype }
    , m_pCBBuffer{ Prototype.m_pCBBuffer }
    , m_pSRV{ Prototype.m_pSRV }
    , m_pSRVBuffer{ Prototype.m_pSRVBuffer }
	, m_pDefaultUAVBufer { Prototype.m_pDefaultUAVBufer}
{
    Safe_AddRef(m_pCBBuffer);
    Safe_AddRef(m_pSRVBuffer);
    Safe_AddRef(m_pSRV);
	Safe_AddRef(m_pDefaultUAVBufer);
}

HRESULT CVIBuffer_FXMesh_Instance::Initialize_Prototype(_fmatrix PreTransformMatrix, const _char* pFilePath, const INSTANCE_DESC* pDesc)
{
    const MESH_FXINSTANCE_DESC* pMeshDesc = static_cast<const MESH_FXINSTANCE_DESC*>(pDesc);

    m_iInstanceVertexStride = sizeof(VTXINSTACNE_FXMESH);     //일단 구상중인것이 가능하다면, 이값 써도 됨. 안되면 구조체 추가해주자.
    m_iNumInstance = pMeshDesc->iNumInstance;

    ifstream EMeshFile(pFilePath, ios::binary);
    if (false == EMeshFile.is_open())
    {
        MSG_BOX("Failed Open : Effect_Mesh");
        return E_FAIL;
    }

    //처음에 읽는 정보는 매쉬개수, 로드파일에서 매쉬개수 먼저 읽음.
    //이펙트 매쉬는 단일 매쉬라 이 정보가 필요없어서 read로 넘겨줘야함.
    _uint MeshIndex = {};
    EMeshFile.read(reinterpret_cast<_char*>(&MeshIndex), sizeof(_uint));

    VTXMESH* pVertices = { nullptr };
    EMeshFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
    pVertices = new VTXMESH[m_iNumVertices];

    _uint* pIndices = { nullptr };
    EMeshFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
    m_iNumIndices = m_iNumIndices * 3;
    m_iNumIndexPerInstance = m_iNumIndices;
    pIndices = new _uint[m_iNumIndices];

    //매쉬 버퍼가 텍스처 개수 알고있어야하나?
    //매쉬 이펙트에 넣어줄거 같은데 일단 데이터 내부에서 읽을려면 넘겨줘야함 나중에 고민해보고 수정하자
    _uint MaterialIndex = {};
    EMeshFile.read(reinterpret_cast<_char*>(&MaterialIndex), sizeof(_uint));

    EMeshFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXMESH) * m_iNumVertices);
    EMeshFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

    for (size_t i = 0; i < m_iNumVertices; ++i)
    {
        XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vTangent, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));
        XMStoreFloat3(&pVertices[i].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));
    }

    m_iNumVertexBuffers = 2;
    m_iVertexStride = sizeof(VTXMESH);

    // 버텍스 버퍼 생성
    D3D11_BUFFER_DESC VBDesc = {};

    VBDesc.Usage = D3D11_USAGE_DEFAULT;
    VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VBDesc.CPUAccessFlags = 0;
    VBDesc.MiscFlags = 0;
    VBDesc.StructureByteStride = sizeof(VTXMESH);
    VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;

    D3D11_SUBRESOURCE_DATA VBInitialData = {};
    VBInitialData.pSysMem = pVertices;

    if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
        return E_FAIL;

    Safe_Delete_Array(pVertices);


    //인덱스 버퍼 생성
    m_eIndexFormat = DXGI_FORMAT_R32_UINT;
    m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_iIndexStride = sizeof(_uint);

    D3D11_BUFFER_DESC IBDesc = {};

    IBDesc.Usage = D3D11_USAGE_DEFAULT;
    IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IBDesc.CPUAccessFlags = 0;
    IBDesc.MiscFlags = 0;
    IBDesc.StructureByteStride = sizeof(_uint);
    IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;

    D3D11_SUBRESOURCE_DATA IBInitialData = {};
    IBInitialData.pSysMem = pIndices;

    if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
        return E_FAIL;

    Safe_Delete_Array(pIndices);

    //매쉬 인스턴싱해서 사용해보고자 하는중.
    //클론이 각각 가져야할 Desc 데이터 버퍼 (그리기용)
    m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
    m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
    m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    m_VBInstanceDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
    m_VBInstanceDesc.MiscFlags = 0;
    m_VBInstanceDesc.StructureByteStride = m_iInstanceVertexStride; //구조체 사이즈

    m_pVBInstanceVertices = new VTXINSTACNE_FXMESH[m_iNumInstance];
    FXMESH_SRV* pSRV = new FXMESH_SRV[m_iNumInstance];

    if (pMeshDesc->IsSpawnBox)
    {
        for (size_t i = 0; i < m_iNumInstance; i++)
        {
            VTXINSTACNE_FXMESH* pInstanceVertices = static_cast<VTXINSTACNE_FXMESH*>(m_pVBInstanceVertices);

            _float fScale = m_pGameInstance->Rand(pMeshDesc->vSize.x, pMeshDesc->vSize.y);

            //각자 속도 다르게할건지
            pSRV[i].fSpeed = m_pGameInstance->Rand(pMeshDesc->vSpeed.x, pMeshDesc->vSpeed.y);

            //매쉬 설정 다양하게 필요할듯.
            //파티클처럼 랜덤적으로 흩뿌려지는 것도 필요하고 , 특정 설정값 잡아주면 그 매쉬는 간격 유지되는 링 상태로 생성 되면서 특정 방향을 볼 수 있게 해줘야할거 같은데 ?
            //일단 움직임은 나중에 설정값으로 추가해주고, 파티클처럼만 만드는거 테스트 먼저 진행하자.

            //사이즈 담기
            pInstanceVertices[i].vRight = _float4(fScale, 0.f, 0.f, 0.f);
            pInstanceVertices[i].vUp = _float4(0.f, fScale, 0.f, 0.f);
            pInstanceVertices[i].vLook = _float4(0.f, 0.f, fScale, 0.f);
            pInstanceVertices[i].vTranslation = _float4(
                m_pGameInstance->Rand(pMeshDesc->vCenter.x - pMeshDesc->vRange.x * 0.5f, pMeshDesc->vCenter.x + pMeshDesc->vRange.x * 0.5f),
                m_pGameInstance->Rand(pMeshDesc->vCenter.y - pMeshDesc->vRange.y * 0.5f, pMeshDesc->vCenter.y + pMeshDesc->vRange.y * 0.5f),
                m_pGameInstance->Rand(pMeshDesc->vCenter.z - pMeshDesc->vRange.z * 0.5f, pMeshDesc->vCenter.z + pMeshDesc->vRange.z * 0.5f),
                1.f
            );

            pInstanceVertices[i].vLifeTime = _float2(0.f, pMeshDesc->fLifeTime);

            pSRV[i].DefaultPos = pInstanceVertices[i].vTranslation;
        }
    }
    else if (pMeshDesc->IsSpawnRing)
    {
        for (size_t i = 0; i < m_iNumInstance; i++)
        {
            VTXINSTACNE_FXMESH* pInstanceVertices = static_cast<VTXINSTACNE_FXMESH*>(m_pVBInstanceVertices);

            _float fScale = m_pGameInstance->Rand(pMeshDesc->vSize.x, pMeshDesc->vSize.y);

            pSRV[i].fSpeed = m_pGameInstance->Rand(pMeshDesc->vSpeed.x, pMeshDesc->vSpeed.y);

            //링의 반지름 범위 최소, 최대
            _float fMin = pMeshDesc->fRmin;
            _float fMax = pMeshDesc->fRmax;

            _float fAngle = {};
            if (!pMeshDesc->IsRingAngle)
            {
                //센터 기준 X,Z를 원형으로 퍼지게 해주기 위해 앵글을 0 ~ 360도가 나오게 설정.
                fAngle = m_pGameInstance->Rand(0.f, XM_2PI);
            }
            else
            {
                _int Index = i;
                _float fStartRadian = XMConvertToRadians(pMeshDesc->fDegreeAngle.x);
                _float fSweepRadian = XMConvertToRadians(pMeshDesc->fDegreeAngle.y);

                fAngle = fStartRadian + ((_float)Index  / (m_iNumInstance - 1)) * fSweepRadian;
            }
            //반지름 최소,최대에 곱해 MIN~MAX의 랜덤값이 나올 수 있게 해주기 위한 값.
            _float fRatio = m_pGameInstance->Rand(0.f, 1.f);

            //sqrt는 제곱근을 계산해주는 함수, sqrt(4) -> 2 / 여기서 나온 Radius가 실질적 반지름의 랜덤 값임.
            _float fRadius = sqrt(fRatio * ((fMax * fMax) - (fMin * fMin)) + (fMin * fMin));

            //Angle의 값은 0 ~ 360도, / 0이면 cos값 1, sin 0 / 180이면 -1 , 0 / 즉, 이값으로 왼쪽 오른쪽 위 아래 방향이 정해지는 것.
            _float fPosX = fRadius * cosf(fAngle);
            _float fPosZ = fRadius * sinf(fAngle);

            pInstanceVertices[i].vTranslation = _float4(
                pMeshDesc->vCenter.x + fPosX,
                pMeshDesc->vCenter.y,                   //센터값일단 평평하게 설정, 랜덤값 주고싶으면 값 하나 더 받아와야함.
                pMeshDesc->vCenter.z + fPosZ,
                1.f
            );

            //사이즈 , 회전 담기.
            if (pMeshDesc->IsInWard)
            {
                _vector vPos = XMVectorSet(pInstanceVertices[i].vTranslation.x, pInstanceVertices[i].vTranslation.y, pInstanceVertices[i].vTranslation.z, 0.f);
                _vector vCenter = XMVectorSet(pMeshDesc->vCenter.x, pMeshDesc->vCenter.y, pMeshDesc->vCenter.z, 0.f);

                _vector vLook = XMVector3Normalize(vPos - vCenter);

                _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

                _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));
                vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight)); 

                if (pMeshDesc->fPitch != 0.f)
                {
                    _float Pitch = XMConvertToRadians(pMeshDesc->fPitch);

                    _matrix Rot = XMMatrixRotationAxis(vRight, Pitch);

                    vUp = XMVector3TransformNormal(vUp, Rot);
                    vLook = XMVector3TransformNormal(vLook, Rot);

                    vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));
                    vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));
                }

                vRight = XMVector3Normalize(vRight) * fScale;
                vUp = XMVector3Normalize(vUp) * fScale;
                vLook = XMVector3Normalize(vLook) * fScale;

                XMStoreFloat4(&pInstanceVertices[i].vRight, vRight);
                XMStoreFloat4(&pInstanceVertices[i].vUp, vUp);
                XMStoreFloat4(&pInstanceVertices[i].vLook, vLook);

            }
            else 
            {
                pInstanceVertices[i].vRight = _float4(fScale, 0.f, 0.f, 0.f);
                pInstanceVertices[i].vUp = _float4(0.f, fScale, 0.f, 0.f);
                pInstanceVertices[i].vLook = _float4(0.f, 0.f, fScale, 0.f);
            };

            pInstanceVertices[i].vLifeTime = _float2(0.f, pMeshDesc->fLifeTime);

            pSRV[i].DefaultPos = pInstanceVertices[i].vTranslation;
        }
    }

    //SRV용 버퍼 생성, 인스턴싱 객체가 각각 가져야할 값, 불변할 값
    D3D11_BUFFER_DESC SRV_BufferDesc = {};
    SRV_BufferDesc.StructureByteStride = sizeof(FXMESH_SRV);
    SRV_BufferDesc.ByteWidth = SRV_BufferDesc.StructureByteStride * m_iNumInstance;
    SRV_BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;				//불변
    SRV_BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;		//리소스
    SRV_BufferDesc.CPUAccessFlags = 0;
    SRV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA SRVInitialData{};
    SRVInitialData.pSysMem = pSRV;

    if (FAILED(m_pDevice->CreateBuffer(&SRV_BufferDesc, &SRVInitialData, &m_pSRVBuffer)))
        return E_FAIL;

    Safe_Delete_Array(pSRV);

    //SRV 버퍼를 통해 리소스뷰 생성
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Buffer.FirstElement = 0;
    SRVDesc.Buffer.NumElements = SRV_BufferDesc.ByteWidth / SRV_BufferDesc.StructureByteStride;

    if (FAILED(m_pDevice->CreateShaderResourceView(m_pSRVBuffer, &SRVDesc, &m_pSRV)))
        return E_FAIL;

    //CB 버퍼 생성, 공용으로 가질 값들 수정가능값
    FXMESH_CB* pCB = new FXMESH_CB;
    pCB->vPivot = pMeshDesc->vPivot;
    pCB->fTimeDelta = 0.1f;
    pCB->IsLoop = pMeshDesc->IsLoop ? 1 : 0;
    pCB->fSpreadWeight = pMeshDesc->fSpreadWeight;
    pCB->fDropWeight = pMeshDesc->fDropWeight;
    pCB->fRotattionWeight = pMeshDesc->fRotationWeight;

    D3D11_BUFFER_DESC CB_BufferDesc = {};
    CB_BufferDesc.StructureByteStride = 0;
    CB_BufferDesc.ByteWidth = sizeof(FXMESH_CB);				 //16바이트 배수로 맞춘 구조체 필요
    CB_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;					//자주변함
    CB_BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		//뷰 없음
    CB_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    CB_BufferDesc.MiscFlags = 0;


    D3D11_SUBRESOURCE_DATA CBInitialData{};
    CBInitialData.pSysMem = pCB;

    if (FAILED(m_pDevice->CreateBuffer(&CB_BufferDesc, &CBInitialData, &m_pCBBuffer)))
        return E_FAIL;

    Safe_Delete(pCB);

    //초기화 전용 UAV버퍼 데이터, 클론끼리 공유해도 상관없음 초기값으로 되돌려주기 위한 값.
    D3D11_BUFFER_DESC Default_UAV_BufferDesc = {};
    Default_UAV_BufferDesc.StructureByteStride = sizeof(VTXINSTACNE_FXMESH);
    Default_UAV_BufferDesc.ByteWidth = Default_UAV_BufferDesc.StructureByteStride * m_iNumInstance;
    Default_UAV_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    Default_UAV_BufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    Default_UAV_BufferDesc.CPUAccessFlags = 0;
    Default_UAV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;


    D3D11_SUBRESOURCE_DATA UAVInitialDesc = {};
    UAVInitialDesc.pSysMem = m_pVBInstanceVertices;

    if (FAILED(m_pDevice->CreateBuffer(&Default_UAV_BufferDesc, &UAVInitialDesc, &m_pDefaultUAVBufer)))
        return E_FAIL;

	return S_OK;
}

//HRESULT CVIBuffer_FXMesh_Instance::Initialize_Clone(void* pArg)
//{
//    if (FAILED(__super::Initialize_Clone(pArg)))
//        return E_FAIL;
//
//    D3D11_BUFFER_DESC UAV_BufferDesc = {};
//    UAV_BufferDesc.StructureByteStride = sizeof(VTXINSTACNE_FXMESH);
//    UAV_BufferDesc.ByteWidth = UAV_BufferDesc.StructureByteStride * m_iNumInstance;
//    UAV_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
//    UAV_BufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
//    UAV_BufferDesc.CPUAccessFlags = 0;
//    UAV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//
//    D3D11_SUBRESOURCE_DATA UAVInitialDesc = {};
//    UAVInitialDesc.pSysMem = m_pVBInstanceVertices;
//
//    if (FAILED(m_pDevice->CreateBuffer(&UAV_BufferDesc, &UAVInitialDesc, &m_pUAVBuffer)))
//        return E_FAIL;
//
//    D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
//    UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
//    UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//    UAVDesc.Buffer.FirstElement = 0;
//    UAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
//    UAVDesc.Buffer.NumElements = UAV_BufferDesc.ByteWidth / UAV_BufferDesc.StructureByteStride;
//
//    if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pUAVBuffer, &UAVDesc, &m_pUAV)))
//        return E_FAIL;
//
//	return S_OK;
//}

HRESULT CVIBuffer_FXMesh_Instance::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	D3D11_BUFFER_DESC UAV_BufferDesc = {};
	UAV_BufferDesc.StructureByteStride = sizeof(VTXINSTACNE_FXMESH);
	UAV_BufferDesc.ByteWidth = UAV_BufferDesc.StructureByteStride * m_iNumInstance;
	UAV_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	UAV_BufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	UAV_BufferDesc.CPUAccessFlags = 0;
	UAV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	if (FAILED(m_pDevice->CreateBuffer(&UAV_BufferDesc, nullptr, &m_pUAVBuffer)))
		return E_FAIL;
	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pUAVBuffer, nullptr, &m_pUAV)))
		return E_FAIL;

	return S_OK;
}

void CVIBuffer_FXMesh_Instance::Bind_CSResources(CComputeShader* pCShader, _float fTimeDelta)
{
    D3D11_MAPPED_SUBRESOURCE	SubResource{};

    //상수버퍼 수정할 값 있으면 여기서 수정해줘야함
    m_pContext->Map(m_pCBBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

    FXMESH_CB* pCB = static_cast<FXMESH_CB*>(SubResource.pData);

    pCB->fTimeDelta = fTimeDelta;

    m_pContext->Unmap(m_pCBBuffer, 0);
    //

    pCShader->Set_ConstantBuffer("CB", m_pCBBuffer);

    pCShader->Set_SRV("g_FXMeshStatic", m_pSRV);

    pCShader->Set_UAV("g_FXMeshState", m_pUAV);

    pCShader->Dispatch(128, 1, 1);

    //GPU에서 복사 진행함. 내부에서 연산작업이 끝났는지 확인하고 복사 진행해준다고 함.
    m_pContext->CopyResource(m_pVBInstance, m_pUAVBuffer);
}

void CVIBuffer_FXMesh_Instance::Reset_UAV()
{
    m_pContext->CopyResource(m_pUAVBuffer, m_pDefaultUAVBufer);
}

CVIBuffer_FXMesh_Instance* CVIBuffer_FXMesh_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath, _fmatrix PreTransformMatrix, const INSTANCE_DESC* pDesc)
{
    CVIBuffer_FXMesh_Instance* pInstance = new CVIBuffer_FXMesh_Instance(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(PreTransformMatrix, pFilePath, pDesc)))
    {
        MSG_BOX("Failed to Create : CVIBuffer_FXMesh_Instance");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CVIBuffer_FXMesh_Instance::Clone(void* pArg)
{
    CVIBuffer_FXMesh_Instance* pClone = new CVIBuffer_FXMesh_Instance(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : CVIBuffer_FXMesh_Instance (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CVIBuffer_FXMesh_Instance::Free()
{
    __super::Free();

    Safe_Release(m_pSRV);
    Safe_Release(m_pCBBuffer);
    Safe_Release(m_pSRVBuffer);
    Safe_Release(m_pUAVBuffer);
    Safe_Release(m_pUAV);
	Safe_Release(m_pDefaultUAVBufer);
}
