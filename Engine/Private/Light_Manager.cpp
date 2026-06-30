#include "EnginePch.h"
#include "Light_Manager.h"

#include "Shader.h"

CLight_Manager::CLight_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CLight_Manager::Initialize()
{
	m_iMaxLights = 256;

	if (FAILED(Ready_LightBuffer()))
		return E_FAIL;

	return S_OK;
}

const LIGHT_DESC* CLight_Manager::Get_LightDesc(const _wstring& strLightTag)
{
	auto iter = m_Lights.find(strLightTag);

	if (iter == m_Lights.end())
		return nullptr;

	return iter->second->Get_LightDesc();
}

HRESULT CLight_Manager::Update_LightDesc(const _wstring& strLightTag, const LIGHT_DESC& LightDesc)
{
	auto iter = m_Lights.find(strLightTag);

	if (iter == m_Lights.end())
		return E_FAIL;

	return iter->second->Update_LightDesc(LightDesc);
}

void CLight_Manager::Set_Active(const _wstring& strLightTag, _bool isActive)
{
	auto iter = m_Lights.find(strLightTag);

	if (iter == m_Lights.end())
		return;

	iter->second->Set_Active(isActive);
}

#ifdef _DEBUG
LIGHT_DESC* CLight_Manager::Get_LightDesc_For_Map(const _wstring& strLightTag)
{
	auto iter = m_Lights.find(strLightTag);

	if (iter == m_Lights.end())
		return nullptr;

	return iter->second->Get_LightDesc_For_Map();
}
#endif

void CLight_Manager::Update_Light()
{
	m_LightDatas.clear();

	for (auto& Pair : m_Lights)
	{
		if (Pair.second->IsInFrustrum())
		{
			const LIGHT_DESC* pLightDesc = (Pair.second)->Get_LightDesc();

			LIGHT_DATA Data = {};

			Data.iType = static_cast<_uint>(pLightDesc->eType);
			Data.fRange = pLightDesc->fRange;
			Data.vPosition = pLightDesc->vPosition;
			Data.vDirection = pLightDesc->vDirection;
			Data.vDiffuse = pLightDesc->vDiffuse;
			Data.vSpecular = pLightDesc->vSpecular;
			Data.vAmbient = pLightDesc->vAmbient;

			m_LightDatas.push_back(Data);
		}
	}

	Update_Buffer();
}

HRESULT CLight_Manager::Add_Light(const _wstring& strLightTag, const LIGHT_DESC& LightDesc)
{
	auto iter = m_Lights.find(strLightTag);

	if (iter != m_Lights.end())
	{
		Safe_Release(iter->second);
		m_Lights.erase(iter);
	}

	CLight* pLight = CLight::Create(LightDesc);

	m_Lights.emplace(strLightTag, pLight);

	return S_OK;
}

//HRESULT CLight_Manager::SetUp_Light(CShader* pShader, const _wstring& strLightTag, LIGHT_DESC::TYPE eType)
//{
//	auto iter = m_Lights.find(strLightTag);
//	if (iter == m_Lights.end())
//		return E_FAIL;
//
//	const LIGHT_DESC* pLight = iter->second->Get_LightDesc();
//
//	if (eType == LIGHT_DESC::DIRECTION)
//	{
//		pShader->Bind_Value("g_vLightDiffuse", &pLight->vDiffuse, sizeof(_float4));
//		pShader->Bind_Value("g_vLightAmbient", &pLight->vAmbient, sizeof(_float4));
//		pShader->Bind_Value("g_vLightSpecular", &pLight->vSpecular, sizeof(_float4));
//		pShader->Bind_Value("g_vLightDir", &pLight->vDirection, sizeof(_float4));
//	}
//
//	return S_OK;
//}

HRESULT CLight_Manager::Clear_Light()
{
	for (auto& Pair : m_Lights)
		Safe_Release(Pair.second);
	m_Lights.clear();

	return S_OK;
}

HRESULT CLight_Manager::Bind_LightDatas(CShader* pShader)
{
	if (FAILED(pShader->Bind_Value("g_iNumLight", &m_iNumLights, sizeof(_uint))))
		CRASH("Failed to Bind NumLights");

	if (FAILED(pShader->Bind_SRV("g_LightDatas", m_pLightSRV)))
		CRASH("Failed to Bind LightData");

	return S_OK;
}

HRESULT CLight_Manager::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	for (auto& Pair : m_Lights)
		Pair.second->Render(pShader, pVIBuffer);

	return S_OK;
}

HRESULT CLight_Manager::Render_EnvMap(CShader* pShader, CVIBuffer_Rect* pVIBuffer, BoundingBox* pBounding)
{
	for (auto& Pair : m_Lights)
		Pair.second->Render_EnvMap(pShader, pVIBuffer, pBounding);

	return S_OK;
}

HRESULT CLight_Manager::Ready_LightBuffer()
{
	//STRUCTURED BUFFER
	D3D11_BUFFER_DESC StructureBufferDesc = {};
	StructureBufferDesc.ByteWidth = sizeof(LIGHT_DATA) * m_iMaxLights;
	StructureBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	StructureBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	StructureBufferDesc.StructureByteStride = sizeof(LIGHT_DATA);
	StructureBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	StructureBufferDesc.CPUAccessFlags = 0;

	if (FAILED(m_pDevice->CreateBuffer(&StructureBufferDesc, nullptr, &m_pLightBuffer)))
		CRASH("Failed to Created VF_LightBuffer");

	//SBUFFER SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC SBuffer_SRVDesc = {};
	SBuffer_SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SBuffer_SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SBuffer_SRVDesc.Buffer.FirstElement = 0;
	SBuffer_SRVDesc.Buffer.NumElements = m_iMaxLights;

	if (FAILED(m_pDevice->CreateShaderResourceView(m_pLightBuffer, &SBuffer_SRVDesc, &m_pLightSRV)))
		CRASH("Failed to Created Buffer_SRV");

	return S_OK;
}

void CLight_Manager::Update_Buffer()
{
	m_iNumLights = m_LightDatas.size();

	if (m_iNumLights <= 0)
		return;

	D3D11_BOX Box = { 0, 0, 0, max(sizeof(LIGHT_DATA) * m_iNumLights, 1), 1, 1 };
	m_pContext->UpdateSubresource(m_pLightBuffer, 0, &Box, m_LightDatas.data(), 0, 0);
}

CLight_Manager* CLight_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLight_Manager* pInstance = new CLight_Manager(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLight_Manager");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLight_Manager::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	Safe_Release(m_pLightSRV);
	Safe_Release(m_pLightBuffer);

	for (auto& Pair : m_Lights)
		Safe_Release(Pair.second);
	m_Lights.clear();
}
