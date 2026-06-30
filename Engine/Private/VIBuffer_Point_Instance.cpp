#include "EnginePch.h"
#include "VIBuffer_Point_Instance.h"
#include "GameInstance.h"

CVIBuffer_Point_Instance::CVIBuffer_Point_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer_Instance { pDevice, pContext }
{
}

CVIBuffer_Point_Instance::CVIBuffer_Point_Instance(const CVIBuffer_Point_Instance& Prototype)
	: CVIBuffer_Instance{ Prototype }
	, m_vPivot{ Prototype.m_vPivot }
	, m_pSpeeds{ Prototype.m_pSpeeds }
	, m_isLoop{ Prototype.m_isLoop }
	, m_pOptionCBBuffer { Prototype.m_pOptionCBBuffer }
	, m_pSpeedCBBuffer { Prototype.m_pSpeedCBBuffer }
	, m_pSRV { Prototype.m_pSRV }
	, m_pSRVBuffer { Prototype.m_pSRVBuffer}
	, m_pDefaultUAVBufer { Prototype.m_pDefaultUAVBufer }
{
	Safe_AddRef(m_pOptionCBBuffer);
	Safe_AddRef(m_pSpeedCBBuffer);
	Safe_AddRef(m_pSRVBuffer);
	Safe_AddRef(m_pSRV);
	Safe_AddRef(m_pDefaultUAVBufer);
}

HRESULT CVIBuffer_Point_Instance::Initialize_Prototype(const INSTANCE_DESC* pDesc)
{
	const POINT_INSTANCE_DESC* pPointDesc = static_cast<const POINT_INSTANCE_DESC*>(pDesc);

	m_vPivot = pPointDesc->vPivot;
	m_isLoop = pPointDesc->IsLoop;

	m_iInstanceVertexStride = sizeof(VTXINSTANCE_PARTICLE);
	m_iNumInstance = pPointDesc->iNumInstance;
	m_iNumVertices = 1;
	m_iVertexStride = sizeof(VTXPOS);
	m_iNumVertexBuffers = 2;
	m_ePrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

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

	m_VBInstanceDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
	m_VBInstanceDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_VBInstanceDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_VBInstanceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_VBInstanceDesc.MiscFlags = 0;
	m_VBInstanceDesc.StructureByteStride = m_iInstanceVertexStride;

	m_pVBInstanceVertices = new VTXINSTANCE_PARTICLE[m_iNumInstance];
	//m_pSpeeds = new _float[m_iNumInstance];

	PARTICLE_SRV* pSRV = new PARTICLE_SRV[m_iNumInstance];

	if (pPointDesc->IsSpawnBox)
	{
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			VTXINSTANCE_PARTICLE* pInstanceVertices = static_cast<VTXINSTANCE_PARTICLE*>(m_pVBInstanceVertices);

			_float		fScale = m_pGameInstance->Rand(pPointDesc->vSize.x, pPointDesc->vSize.y);
			_float		fLifeTime = m_pGameInstance->Rand(pPointDesc->vLifeTime.x, pPointDesc->vLifeTime.y);

			pSRV[i].fSpeed = m_pGameInstance->Rand(pPointDesc->vSpeed.x, pPointDesc->vSpeed.y);
			pSRV[i].fDelay = pPointDesc->fDelay.y * (i + 1);

			pInstanceVertices[i].vRight = _float4(fScale, 0.f, 0.f, 0.f);
			pInstanceVertices[i].vUp = _float4(0.f, fScale, 0.f, 0.f);
			pInstanceVertices[i].vLook = _float4(0.f, 0.f, fScale, 0.f);
			pInstanceVertices[i].vTranslation = _float4(
				m_pGameInstance->Rand(pPointDesc->vCenter.x - pPointDesc->vRange.x * 0.5f, pPointDesc->vCenter.x + pPointDesc->vRange.x * 0.5f),
				m_pGameInstance->Rand(pPointDesc->vCenter.y - pPointDesc->vRange.y * 0.5f, pPointDesc->vCenter.y + pPointDesc->vRange.y * 0.5f),
				m_pGameInstance->Rand(pPointDesc->vCenter.z - pPointDesc->vRange.z * 0.5f, pPointDesc->vCenter.z + pPointDesc->vRange.z * 0.5f),
				1.f
			);

			pInstanceVertices[i].vLifeTime = _float2(0.f, fLifeTime);

			pSRV[i].DefaultPos = pInstanceVertices[i].vTranslation;
		}
	}
	else if (pPointDesc->IsSpawnRing)
	{
	
		for (size_t i = 0; i < m_iNumInstance; i++)
		{
			VTXINSTANCE_PARTICLE* pInstanceVertices = static_cast<VTXINSTANCE_PARTICLE*>(m_pVBInstanceVertices);

			_float fScale = m_pGameInstance->Rand(pPointDesc->vSize.x, pPointDesc->vSize.y);

			pSRV[i].fSpeed = m_pGameInstance->Rand(pPointDesc->vSpeed.x, pPointDesc->vSpeed.y);

			//링의 반지름 범위 최소, 최대
			_float fMin = pPointDesc->fRmin;
			_float fMax = pPointDesc->fRmax;

			_float fAngle = {};
			if (!pPointDesc->IsRingAngle)
			{
				//센터 기준 X,Z를 원형으로 퍼지게 해주기 위해 앵글을 0 ~ 360도가 나오게 설정.
				fAngle = m_pGameInstance->Rand(0.f, XM_2PI);
			}
			else
			{
				_int Index = i;
				_float fStartRadian = XMConvertToRadians(pPointDesc->fDegreeAngle.x);
				_float fSweepRadian = XMConvertToRadians(pPointDesc->fDegreeAngle.y);

				fAngle = fStartRadian + ((_float)Index / (m_iNumInstance - 1)) * fSweepRadian;
			}
			//반지름 최소,최대에 곱해 MIN~MAX의 랜덤값이 나올 수 있게 해주기 위한 값.
			_float fRatio = m_pGameInstance->Rand(0.f, 1.f);

			//sqrt는 제곱근을 계산해주는 함수, sqrt(4) -> 2 / 여기서 나온 Radius가 실질적 반지름의 랜덤 값임.
			_float fRadius = sqrt(fRatio * ((fMax * fMax) - (fMin * fMin)) + (fMin * fMin));

			//Angle의 값은 0 ~ 360도, / 0이면 cos값 1, sin 0 / 180이면 -1 , 0 / 즉, 이값으로 왼쪽 오른쪽 위 아래 방향이 정해지는 것.
			_float fPosX = fRadius * cosf(fAngle);
			_float fPosZ = fRadius * sinf(fAngle);

			//설정한 Range 값으로 위아래 범위 랜덤잡아주기.
			_float fPosY = m_pGameInstance->Rand(-(pPointDesc->vRange.y * 0.5f), pPointDesc->vRange.y * 0.5f);

			pInstanceVertices[i].vTranslation = _float4(
				pPointDesc->vCenter.x + fPosX,
				pPointDesc->vCenter.y + fPosY,                 
				pPointDesc->vCenter.z + fPosZ,
				1.f
			);

			pInstanceVertices[i].vRight = _float4(fScale, 0.f, 0.f, 0.f);
			pInstanceVertices[i].vUp = _float4(0.f, fScale, 0.f, 0.f);
			pInstanceVertices[i].vLook = _float4(0.f, 0.f, fScale, 0.f);

			_float		fLifeTime = m_pGameInstance->Rand(pPointDesc->vLifeTime.x, pPointDesc->vLifeTime.y);
			pInstanceVertices[i].vLifeTime = _float2(0.f, fLifeTime);

 			pSRV[i].DefaultPos = pInstanceVertices[i].vTranslation;
			pSRV[i].fDelay = pPointDesc->fDelay.y * (i + 1);
		}
	}

	//초기화 전용 UAV버퍼 데이터, 클론끼리 공유해도 상관없음 초기값으로 되돌려주기 위한 값.
	D3D11_BUFFER_DESC Default_UAV_BufferDesc = {};
	Default_UAV_BufferDesc.StructureByteStride = sizeof(VTXINSTANCE_PARTICLE);
	Default_UAV_BufferDesc.ByteWidth = Default_UAV_BufferDesc.StructureByteStride * m_iNumInstance;
	Default_UAV_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	Default_UAV_BufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	Default_UAV_BufferDesc.CPUAccessFlags = 0;
	Default_UAV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA UAVInitialDesc = {};
	UAVInitialDesc.pSysMem = m_pVBInstanceVertices;

	if (FAILED(m_pDevice->CreateBuffer(&Default_UAV_BufferDesc, &UAVInitialDesc, &m_pDefaultUAVBufer)))
		return E_FAIL;
	
	D3D11_BUFFER_DESC SRV_BufferDesc = {};
	SRV_BufferDesc.StructureByteStride = sizeof(PARTICLE_SRV);
	SRV_BufferDesc.ByteWidth = SRV_BufferDesc.StructureByteStride * m_iNumInstance;
	SRV_BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;				
	SRV_BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;		
	SRV_BufferDesc.CPUAccessFlags = 0; 
	SRV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA SRVInitialData{};
	SRVInitialData.pSysMem = pSRV;

	if (FAILED(m_pDevice->CreateBuffer(&SRV_BufferDesc, &SRVInitialData, &m_pSRVBuffer)))
		return E_FAIL;

	Safe_Delete_Array(pSRV);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.FirstElement = 0;
	SRVDesc.Buffer.NumElements = SRV_BufferDesc.ByteWidth / SRV_BufferDesc.StructureByteStride;

	if (FAILED(m_pDevice->CreateShaderResourceView(m_pSRVBuffer, &SRVDesc, &m_pSRV)))
		return E_FAIL;

	PARTICLE_DefaultCB* pOptionCB = new PARTICLE_DefaultCB;
	pOptionCB->vPivot = m_vPivot;
 	pOptionCB->IsLoop = m_isLoop ? 1 : 0;
	pOptionCB->IsStretch = pPointDesc->IsStretch ? 1 : 0;
	pOptionCB->IsSprite = pPointDesc->IsSprite ? 1 : 0;
	pOptionCB->IsDelay = (pPointDesc->fDelay.y > 0.f) ? 1 : 0;

	D3D11_BUFFER_DESC CB_OptionBufferDesc = {};
	CB_OptionBufferDesc.StructureByteStride = 0;
	CB_OptionBufferDesc.ByteWidth = sizeof(PARTICLE_DefaultCB);				
	CB_OptionBufferDesc.Usage = D3D11_USAGE_DYNAMIC ;							
	CB_OptionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;				
	CB_OptionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	CB_OptionBufferDesc.MiscFlags = 0;


	D3D11_SUBRESOURCE_DATA CBOptionInitialData{};
	CBOptionInitialData.pSysMem = pOptionCB;

	if (FAILED(m_pDevice->CreateBuffer(&CB_OptionBufferDesc, &CBOptionInitialData, &m_pOptionCBBuffer)))
		return E_FAIL;

	Safe_Delete(pOptionCB);

	PARTICLE_SPEEDCB* pSpeedCB = new PARTICLE_SPEEDCB;
	pSpeedCB->fTimeDelta = 0.1f;
	pSpeedCB->fSpreadWeight = pPointDesc->fSpreadWeight;
	pSpeedCB->fDropWeight = pPointDesc->fDropWeight;
	pSpeedCB->fRotationWeight = pPointDesc->fRotationWeight;
	pSpeedCB->fGravity = pPointDesc->fGravity;
	pSpeedCB->fStretchWeight = pPointDesc->fStretchWeight;
	pSpeedCB->fStretchRange = pPointDesc->fStretchRange;
	pSpeedCB->fSpriteDefault = pPointDesc->fDefualtSpeed;
	pSpeedCB->fSpriteWeight = pPointDesc->fSpriteWeight;
	
	D3D11_BUFFER_DESC CB_SpeedBufferDesc = {};
	CB_SpeedBufferDesc.StructureByteStride = 0;
	CB_SpeedBufferDesc.ByteWidth = sizeof(PARTICLE_SPEEDCB);				
	CB_SpeedBufferDesc.Usage = D3D11_USAGE_DYNAMIC;							
	CB_SpeedBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;				
	CB_SpeedBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	CB_SpeedBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA CBSpeedInitialData{};
	CBSpeedInitialData.pSysMem = pSpeedCB;

	if (FAILED(m_pDevice->CreateBuffer(&CB_SpeedBufferDesc, &CBSpeedInitialData, &m_pSpeedCBBuffer)))
		return E_FAIL;

	Safe_Delete(pSpeedCB);

	//// ==디버깅 //
	//D3D11_BUFFER_DESC bufferDesc = {};
	//bufferDesc.StructureByteStride = sizeof(VTXINSTANCE_PARTICLE);
	//bufferDesc.ByteWidth = sizeof(VTXINSTANCE_PARTICLE) * m_iNumInstance;
	//bufferDesc.Usage = D3D11_USAGE_STAGING;
	//bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	//if (FAILED(m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pDebugBuffer)))
	//	return E_FAIL;

	////==디버깅==
	//m_pContext->CopyResource(m_pDebugBuffer, m_pDefaultUAVBufer);

	//D3D11_MAPPED_SUBRESOURCE mapped{};

	//m_pContext->Map(m_pDebugBuffer, 0, D3D11_MAP_READ, 0, &mapped);

	//VTXINSTANCE_PARTICLE* pData = static_cast<VTXINSTANCE_PARTICLE*>(mapped.pData);

	//for (UINT i = 0; i < m_iNumInstance; ++i)
	//{
	//	pData[i].vTranslation;
	//	pData[i];
	//}


	return S_OK;
}

//HRESULT CVIBuffer_Point_Instance::Initialize_Clone(void* pArg)
//{
//	if (FAILED(__super::Initialize_Clone(pArg)))
//		return E_FAIL;
//
//	D3D11_BUFFER_DESC UAV_BufferDesc = {};
//	UAV_BufferDesc.StructureByteStride = sizeof(VTXINSTANCE_PARTICLE);
//	UAV_BufferDesc.ByteWidth = UAV_BufferDesc.StructureByteStride * m_iNumInstance;
//	UAV_BufferDesc.Usage = D3D11_USAGE_DEFAULT;						
//	UAV_BufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;			
//	UAV_BufferDesc.CPUAccessFlags = 0;			
//	UAV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;	
//
//
//	D3D11_SUBRESOURCE_DATA UAVInitialDesc = {};
//	UAVInitialDesc.pSysMem = m_pVBInstanceVertices;
//
//	if (FAILED(m_pDevice->CreateBuffer(&UAV_BufferDesc, &UAVInitialDesc, &m_pUABuffer)))
//		return E_FAIL;
//
//	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
//	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
//	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//	UAVDesc.Buffer.FirstElement = 0;
//	UAVDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
//	UAVDesc.Buffer.NumElements = UAV_BufferDesc.ByteWidth / UAV_BufferDesc.StructureByteStride;
//
//	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pUABuffer, &UAVDesc, &m_pUAV)))
//		return E_FAIL;
//
//	// ==디버깅 //
//	D3D11_BUFFER_DESC bufferDesc = {};
//	bufferDesc.StructureByteStride = sizeof(VTXINSTANCE_PARTICLE);
//	bufferDesc.ByteWidth = sizeof(VTXINSTANCE_PARTICLE) * m_iNumInstance;
//	bufferDesc.Usage = D3D11_USAGE_STAGING;
//	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//
//	if (FAILED(m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pDebugBuffer)))
//		return E_FAIL;
//
//	return S_OK;
//}

HRESULT CVIBuffer_Point_Instance::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	D3D11_BUFFER_DESC UAV_BufferDesc = {};
	UAV_BufferDesc.StructureByteStride = sizeof(VTXINSTANCE_PARTICLE);
	UAV_BufferDesc.ByteWidth = UAV_BufferDesc.StructureByteStride * m_iNumInstance;
	UAV_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	UAV_BufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	UAV_BufferDesc.CPUAccessFlags = 0;
	UAV_BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA UAVInitialDesc = {};
	UAVInitialDesc.pSysMem = m_pVBInstanceVertices;
	if (FAILED(m_pDevice->CreateBuffer(&UAV_BufferDesc, nullptr, &m_pUABuffer)))
		return E_FAIL;
	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pUABuffer, nullptr, &m_pUAV)))
		return E_FAIL;

	// ==디버깅 //
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.StructureByteStride = sizeof(VTXINSTANCE_PARTICLE);
	bufferDesc.ByteWidth = sizeof(VTXINSTANCE_PARTICLE) * m_iNumInstance;
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	if (FAILED(m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_pDebugBuffer)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Point_Instance::Bind_Resources()
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
		0
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveType);

	return S_OK;
}

HRESULT CVIBuffer_Point_Instance::Render()
{
	m_pContext->DrawInstanced(1, m_iNumInstance, 0, 0);

	return S_OK;
}

void CVIBuffer_Point_Instance::Bind_CS_Pivot(_vector vRight, _vector vUp, _vector vLook)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	m_pContext->Map(m_pOptionCBBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	PARTICLE_DefaultCB* pCB = static_cast<PARTICLE_DefaultCB*>(SubResource.pData);

	_vector vWorldDir = (vRight * m_vPivot.x) + (vUp * m_vPivot.y) + (vLook * m_vPivot.z);

	_float3 WorldDir = {};
	XMStoreFloat3(&WorldDir, vWorldDir);

	pCB->vPivot.x = WorldDir.x;
	pCB->vPivot.y = WorldDir.y;
	pCB->vPivot.z = WorldDir.z;

	m_pContext->Unmap(m_pOptionCBBuffer, 0);
}

void CVIBuffer_Point_Instance::Bind_CS_Speed(_float fTimeDelta, PARTICLE_SPEEDCB* SpeedDesc)
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	m_pContext->Map(m_pSpeedCBBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	PARTICLE_SPEEDCB* pCB = static_cast<PARTICLE_SPEEDCB*>(SubResource.pData);

	pCB->fTimeDelta = fTimeDelta;

	if (SpeedDesc != nullptr)
	{
		pCB->fDropWeight = SpeedDesc->fDropWeight;
		pCB->fSpreadWeight = SpeedDesc->fSpreadWeight;
		pCB->fRotationWeight = SpeedDesc->fRotationWeight;
		pCB->fGravity = SpeedDesc->fGravity;
		pCB->fStretchWeight = SpeedDesc->fStretchWeight;
		pCB->fStretchRange = SpeedDesc->fStretchRange;
	}

	m_pContext->Unmap(m_pSpeedCBBuffer, 0);
}

void CVIBuffer_Point_Instance::Bind_CSResources(CComputeShader* pCShader)
{
	pCShader->Set_ConstantBuffer("OptionCB", m_pOptionCBBuffer);

	pCShader->Set_ConstantBuffer("SpeedCB", m_pSpeedCBBuffer);

	pCShader->Set_SRV("g_ParticleStatic", m_pSRV);

	pCShader->Set_UAV("g_ParticleState", m_pUAV);

	pCShader->Dispatch(128, 1, 1);

	m_pContext->CopyResource(m_pVBInstance, m_pUABuffer);
}

void CVIBuffer_Point_Instance::Reset_UAV(class CComputeShader* pCShader)
{
	pCShader->Clear_Resources();

	//UAV 내용을 기본값으로 롤백
	m_pContext->CopyResource(m_pUABuffer, m_pDefaultUAVBufer);

	// 카운터 0으로 초기화
	//const UINT zero[4] = { 0,0,0,0 };
	//m_pContext->ClearUnorderedAccessViewUint(m_pUAV, zero);

	//// m_pVBInstacne도 초기화 진행.
	//m_pContext->CopyResource(m_pVBInstance, m_pDefaultUAVBufer);

	////==디버깅==
	m_pContext->CopyResource(m_pDebugBuffer, m_pUABuffer);


	//D3D11_MAPPED_SUBRESOURCE mapped{};

	//m_pContext->Map(m_pDebugBuffer, 0, D3D11_MAP_READ, 0, &mapped);

	//VTXINSTANCE_PARTICLE* pData = static_cast<VTXINSTANCE_PARTICLE*>(mapped.pData);

	//for (size_t i = 0; i < m_iNumInstance; i++)
	//{
	//	pData[i];
	//}

}

void CVIBuffer_Point_Instance::Reset_CS_Option()
{
	D3D11_MAPPED_SUBRESOURCE	SubResource{};

	m_pContext->Map(m_pOptionCBBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	PARTICLE_DefaultCB* pCB = static_cast<PARTICLE_DefaultCB*>(SubResource.pData);

	pCB->vPivot = m_vPivot;

	m_pContext->Unmap(m_pOptionCBBuffer, 0);
}

CVIBuffer_Point_Instance* CVIBuffer_Point_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const INSTANCE_DESC* pDesc)
{
	CVIBuffer_Point_Instance* pInstance = new CVIBuffer_Point_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pDesc)))
	{
		MSG_BOX("Failed to Create : CVIBuffer_Point_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Point_Instance::Clone(void* pArg)
{
	CVIBuffer_Point_Instance* pClone = new CVIBuffer_Point_Instance(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Clone : CVIBuffer_Point_Instance");
		Safe_Release(pClone);
	}

	return pClone;
}

void CVIBuffer_Point_Instance::Free()
{
	__super::Free();

	//if (false == m_isClone)
	//{
	//	Safe_Delete_Array(m_pSpeeds);
	//}

	Safe_Release(m_pSRV);
	Safe_Release(m_pOptionCBBuffer);
	Safe_Release(m_pSpeedCBBuffer);
	Safe_Release(m_pSRVBuffer);
	Safe_Release(m_pUABuffer);
	Safe_Release(m_pUAV);
	Safe_Release(m_pDefaultUAVBufer);
	Safe_Release(m_pDebugBuffer);
}
