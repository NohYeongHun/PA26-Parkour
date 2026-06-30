#include "EnginePch.h"
#include "Decal_Manager.h"
#include "Shader.h"
#include "GameInstance.h"
#include "Decal.h"

CDecal_Manager::CDecal_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CDecal_Manager::Initialize()
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

HRESULT CDecal_Manager::Ready_Components()
{
	m_pShader = CShader::Create(m_pDevice, m_pContext,
		TEXT("../Bin/ShaderFiles/Engine_Shader_Decal.hlsl"),
		VTX_DECAL::Elements, VTX_DECAL::iNumElements);

	ASSERT_CRASH(m_pShader);

	return S_OK;
}

void CDecal_Manager::Update(_float fTimeDelta)
{
	for (auto& Pair : m_Decals)
		Pair.second->Update(fTimeDelta);
}

HRESULT CDecal_Manager::Add_CustomDecal(CGameObject* pCustomDecalObject)
{
	if (nullptr == pCustomDecalObject)
		return E_FAIL;

	m_CustomDecals.push_back(pCustomDecalObject);
	Safe_AddRef(pCustomDecalObject);

	return S_OK;
}

HRESULT CDecal_Manager::Add_Decal(const _wstring& strDecalTag, 
	const _tchar* pFilePath[ENUM_CLASS(TEXTURETYPE::END)], const _float3& vEmissiveLuminance)
{
	if (nullptr != Find_Decal(strDecalTag))
		CRASH("Failed to Add Decal Duplication");

	CDecal* pDecal = CDecal::Create(m_pDevice, m_pContext, pFilePath, vEmissiveLuminance);
	ASSERT_CRASH(pDecal);

	m_Decals.emplace(strDecalTag, pDecal);

	return S_OK;
}


HRESULT CDecal_Manager::Add_DecalData(const _wstring& strDecalTag, const DECAL_DATA& Decal)
{
	CDecal* pDecal = Find_Decal(strDecalTag);
	ASSERT_CRASH(pDecal);

	pDecal->Add_DecalData(Decal);

	return S_OK;
}

HRESULT CDecal_Manager::Render()
{
	for (auto& Pair : m_Decals)
		Pair.second->Render(m_pShader);

	for (auto& pCustomDecal : m_CustomDecals)
	{
		pCustomDecal->Render();
		Safe_Release(pCustomDecal);
	}

	m_CustomDecals.clear();

	return S_OK;
}

void CDecal_Manager::Clear()
{
	for (auto& Pair : m_Decals)
		Safe_Release(Pair.second);
	m_Decals.clear();

	for (auto& pCustomDecal : m_CustomDecals)
		Safe_Release(pCustomDecal);
	m_CustomDecals.clear();
}



CDecal* CDecal_Manager::Find_Decal(const _wstring& strDecalTag)
{
	auto iter = m_Decals.find(strDecalTag);
	if(iter == m_Decals.end())
		return nullptr;
	return iter->second;
}

CDecal_Manager* CDecal_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDecal_Manager* pInstance = new CDecal_Manager(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CDecal_Manager");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CDecal_Manager::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pShader);

	for (auto& Pair : m_Decals)
		Safe_Release(Pair.second);
	m_Decals.clear();
	
	for (auto& pCustomDecal : m_CustomDecals)
		Safe_Release(pCustomDecal);
	m_CustomDecals.clear();
}
