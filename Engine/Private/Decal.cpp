#include "EnginePch.h"
#include "Decal.h"
#include "Texture.h"
#include "VIBuffer_Decal_Cube.h"
#include "Shader.h"
#include "GameInstance.h"

CDecal::CDecal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CDecal::Initialize(const _tchar* pFilePath[ENUM_CLASS(TEXTURETYPE::END)], const _float3& vEmissiveLuminance)
{
	m_pVIBuffer_Decal = CVIBuffer_Decal_Cube::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pVIBuffer_Decal);

	if (FAILED(Add_DecalTexture(pFilePath, vEmissiveLuminance)))
		return E_FAIL;

	return S_OK;
}

void CDecal::Update(_float fTimeDelta)
{
	for (auto iter = m_DecalDatas.begin(); iter != m_DecalDatas.end();)
	{
		if (iter->first == DECAL_DATA::STATIC)                                
		{
			iter++;
		}
		else
		{
			if (Update_InstanceData(iter->second, fTimeDelta))
			{
				++iter;
			}
			else
			{
				iter = m_DecalDatas.erase(iter);
			}
		}
		
	}

	m_iNumDecals = static_cast<_uint>(m_DecalDatas.size());

	vector<VTXINSTANCE_DECAL> TempDatas;
	TempDatas.reserve(m_iNumDecals);
	for (auto& Data : m_DecalDatas)
		TempDatas.push_back(Data.second.second);

	if (m_iNumDecals > 0)
		m_pVIBuffer_Decal->Update_Buffer(TempDatas);
	else
		m_pVIBuffer_Decal->Clear();
}

void CDecal::Render(CShader* pShader)
{
	if (m_iNumDecals <= 0 || m_iNumDecals >= g_iMaxDecal)
		return;

	Bind_Resources(pShader);

	pShader->Begin(0);

	m_pVIBuffer_Decal->Bind_Resources();
	m_pVIBuffer_Decal->Render();
}

HRESULT CDecal::Add_DecalTexture(const _tchar* pFilePath[ENUM_CLASS(TEXTURETYPE::END)], _float3 vEmissiveLuminance)
{
	for (_uint i = 0; i < ENUM_CLASS(TEXTURETYPE::END); ++i)
	{
		if (nullptr == pFilePath[i])
			continue;

		if (m_pDecalTexture[i] != nullptr)
			ASSERT_CRASH("TEST");

		CTexture* pTexture = CTexture::Create(m_pDevice, m_pContext, pFilePath[i], 1);
		ASSERT_CRASH(pTexture);

		m_pDecalTexture[i] = pTexture;
	}

	m_vEmiisiveLuminance = vEmissiveLuminance;

    return S_OK;
}

HRESULT CDecal::Add_DecalData(const DECAL_DATA& Decal)
{
	if (m_iNumDecals >= g_iMaxDecal)
		return E_FAIL;

	VTXINSTANCE_DECAL Data = {};

	_matrix WorldInv = XMMatrixInverse(nullptr, Decal.WorldMatrix);

	memcpy(&Data.vRight, &Decal.WorldMatrix, sizeof(_matrix));
	memcpy(&Data.vRightInv, &WorldInv, sizeof(_matrix));

	Data.fAlpha = 0.f;

	Data.vColor = Decal.vColor;
	Data.fEmissiveIntensity = Decal.fEmissiveIntensity == 0.f ? 1.f : Decal.fEmissiveIntensity;

	DECAL_UPDATE_DATA UpdateData = {};

	UpdateData.fCurrentTime = 0.f;
	UpdateData.fBlendTime = Decal.fBlendTime;
	UpdateData.fLifeTime = Decal.fLifeTime;

	XMMatrixDecompose(&UpdateData.vStartScale, &UpdateData.vStartRotation, &UpdateData.vStartPosition, Decal.WorldMatrix);
	XMMatrixDecompose(&UpdateData.vEndScale, &UpdateData.vEndRotation, &UpdateData.vEndPosition, Decal.EndWorldMatrix);

	if (XMVector4Equal(UpdateData.vStartScale, UpdateData.vEndScale)
		&& XMQuaternionEqual(UpdateData.vStartRotation, UpdateData.vEndRotation) 
		&& XMVector4Equal(UpdateData.vStartPosition, UpdateData.vEndPosition))
		UpdateData.IsEqual = true;
	else
		UpdateData.IsEqual = false;

	DECAL_INSTANCE_DATA DataPair = make_pair(UpdateData, Data);

	DECAL_INSTANCE Pair = make_pair(Decal.eType, DataPair);

	m_DecalDatas.push_back(Pair);

	return S_OK;
}

ID3D11ShaderResourceView* CDecal::Get_DecalSRV(TEXTURETYPE eTextureType)
{
	if(nullptr == m_pDecalTexture[ENUM_CLASS(eTextureType)])
		return nullptr;

	return m_pDecalTexture[ENUM_CLASS(eTextureType)]->Get_SRV(0);
}

_bool CDecal::Update_InstanceData(DECAL_INSTANCE_DATA& Data, _float fTimeDelta)
{
	Data.first.fCurrentTime += fTimeDelta;

	if (Data.first.fCurrentTime >= Data.first.fLifeTime)
		return false;

	if (Data.first.fCurrentTime >= Data.first.fBlendTime)
	{
		_float fDenom = max(Data.first.fLifeTime - Data.first.fBlendTime, 1e-5);
		_float fNum = Data.first.fCurrentTime - Data.first.fBlendTime;

		Data.second.fAlpha = Saturate(fNum / fDenom);
	}

	if (Data.first.IsEqual)
		return true;

	_matrix CurrentWorld= {};

	_vector vScale, vRotation, vPosition;

	vScale = XMVectorLerp(Data.first.vStartScale, Data.first.vEndScale, Data.second.fAlpha);
	vRotation = XMQuaternionSlerp(Data.first.vStartRotation, Data.first.vEndRotation, Data.second.fAlpha);
	vPosition = XMVectorLerp(Data.first.vStartPosition, Data.first.vEndPosition, Data.second.fAlpha);

	CurrentWorld = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vPosition);

	_matrix CurrentWorldInv= XMMatrixInverse(nullptr, CurrentWorld);

	memcpy(&Data.second.vRight, &CurrentWorld, sizeof(_matrix));
	memcpy(&Data.second.vRightInv, &CurrentWorldInv, sizeof(_matrix));

	return true;
}

HRESULT CDecal::Bind_Resources(CShader* pShader)
{
	if (nullptr == pShader)
		return E_FAIL;
	
	for (_uint i = 0; i < ENUM_CLASS(TEXTURETYPE::END); ++i)
	{
		string strTextureConstantName = {};
		string strBoolConstantName = {};
		switch (i)
		{
		case ENUM_CLASS(TEXTURETYPE::DIFFUSE):
			strTextureConstantName = "g_DiffuseTexture";
			strBoolConstantName = "g_HasDiffuse";
			break;
		case ENUM_CLASS(TEXTURETYPE::NORMAL):
			strTextureConstantName = "g_NormalTexture";
			strBoolConstantName = "g_HasNormal";
			break;
		case ENUM_CLASS(TEXTURETYPE::MASK):
			strTextureConstantName = "g_MaskTexture";
			strBoolConstantName = "g_HasMask";
			break;
		case ENUM_CLASS(TEXTURETYPE::EMISSIVE):
			strTextureConstantName = "g_EmissiveTexture";
			strBoolConstantName = "g_HasEmissive";
			break;
		}

		if (nullptr == m_pDecalTexture[i])
		{
			if (FAILED(pShader->Bind_Texture(strTextureConstantName.c_str(), nullptr)))
				return S_OK;

			_bool HasTexture = false;

			if (FAILED(pShader->Bind_Value(strBoolConstantName.c_str(), &HasTexture, sizeof(_bool))))
				CRASH("Failed Bind Value");

			continue;
		}

		_bool HasTexture = false;
		if (SUCCEEDED(m_pDecalTexture[i]->Bind_Shader_Resource(pShader, strTextureConstantName.c_str())))
			HasTexture = true;

		if (FAILED(pShader->Bind_Value(strBoolConstantName.c_str(), &HasTexture, sizeof(_bool))))
			CRASH("Failed Bind Value");
	}

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Depth"), pShader, "g_DepthTexture")))
		CRASH("Failed Bind RT_Depth");

	if (FAILED(pShader->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed Bind ViewMatrixInv");
	if (FAILED(pShader->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed Bind ProjMatrixInv");
	if (FAILED(pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW))))
		CRASH("Failed Bind ViewMatrixInv");
	if (FAILED(pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::PROJ))))
		CRASH("Failed Bind ProjMatrixInv");
	if (FAILED(pShader->Bind_Value("g_EmissiveLuminance", &m_vEmiisiveLuminance, sizeof(_float3))))
		CRASH("Failed Bind EmissiveLuminance");

	return S_OK;
}

CDecal* CDecal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath[ENUM_CLASS(TEXTURETYPE::END)], const _float3& vEmissiveLuminance)
{
	CDecal* pInstance = new CDecal(pDevice, pContext);
	if (FAILED(pInstance->Initialize(pFilePath, vEmissiveLuminance)))
	{
		MSG_BOX("Failed to Created : CDecal");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CDecal::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pVIBuffer_Decal);

	for (_uint i = 0; i < ENUM_CLASS(TEXTURETYPE::END); ++i)
		Safe_Release(m_pDecalTexture[i]);
}
