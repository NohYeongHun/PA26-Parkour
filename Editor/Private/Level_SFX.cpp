#include "EditorPch.h"
#include "Level_SFX.h"
#include "SFX_Interface.h"
#include "EditDummy_Map.h"

CLevel_SFX::CLevel_SFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_SFX::Initialize()
{
	if (FAILED(Ready_Light()))
		return E_FAIL;

	if (FAILED(Ready_Interface()))
		return E_FAIL;

	m_pGameInstance->SettingFog(false);

	CEditDummy_Map::DUMMY_MAP_DESC MapDesc = {};
	_matrix PreTransformationMatrix = XMMatrixScalingFromVector(XMVectorSet(0.001f, 0.001f, 0.001f, 1.f)) * XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(180.f), 0.f))
		* XMMatrixTranslationFromVector(XMVectorSet(0.f, -10.f, 0.f, 1.f));
	MapDesc.PreTransformMatrix = PreTransformationMatrix;

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Map"),
		ENUM_CLASS(LEVEL::SHADER), TEXT("Layer_Dummy"), &MapDesc)))
		CRASH("Failed Clone Dummy Wolf");

	return S_OK;
}

void CLevel_SFX::Update(_float fTimeDelta)
{
	m_pSFX_Interface->Priority_Update(fTimeDelta);
	m_pSFX_Interface->Update(fTimeDelta);
	m_pSFX_Interface->Late_Update(fTimeDelta);
}

void CLevel_SFX::Render()
{
	m_pSFX_Interface->Render();
}

HRESULT CLevel_SFX::Ready_Light()
{
	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;
	LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
	LightDesc.vDiffuse = _float4(0.8f, 0.7f, 0.12f, 1.f);
	LightDesc.vDirection = _float4(1.f, -0.5f, -1.f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_CameraNF();

	return S_OK;
}

HRESULT CLevel_SFX::Ready_Interface()
{
	m_pSFX_Interface = CSFX_Interface::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pSFX_Interface);

	return S_OK;
}

HRESULT CLevel_SFX::Ready_TestObjects()
{
	return S_OK;
}

CLevel_SFX* CLevel_SFX::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_SFX* pInstance = new CLevel_SFX(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : CLevel_SFX");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_SFX::Free()
{
	__super::Free();

	Safe_Release(m_pSFX_Interface);
}
