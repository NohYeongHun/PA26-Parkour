#include "EnginePch.h"
#include "Target_Manager.h"

#include "Shader.h"
#include "RenderTarget.h"

CTarget_Manager::CTarget_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

ID3D11Texture2D* CTarget_Manager::Get_RT_Resource(const _wstring& strTargetTag)
{
	CRenderTarget* pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return nullptr;

	return pRenderTarget->Get_Resource();
}

ID3D11ShaderResourceView* CTarget_Manager::Get_RT_SRV(const _wstring& strTargetTag)
{
	CRenderTarget* pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return nullptr;

	return pRenderTarget->Get_SRV();
}

#ifdef _DEBUG
ID3D11ShaderResourceView* CTarget_Manager::Get_Debug_RT_Resource(const _wstring& strTargetTag)
{
	CRenderTarget* pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return nullptr;

	return pRenderTarget->Get_SRV();
}
void CTarget_Manager::Change_BackBufferColor(const _wstring& strTargetTag, const _float4& vClearColor)
{
	CRenderTarget* pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return;

	pRenderTarget->Change_BackBufferColor(vClearColor);
}
#endif

HRESULT CTarget_Manager::Bind_OpenRT(OPEN_RT eRT, CShader* pShader, const _char* pConstantName)
{
	_wstring strRTTag = {};

	switch(eRT)
	{
		case OPEN_RT::DEPTH:
			strRTTag = TEXT("RT_Depth");
			break;

		default:
			return E_FAIL;
			break;
	}

	return pShader->Bind_SRV(pConstantName, Get_RT_SRV(strRTTag));
}
HRESULT CTarget_Manager::Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT eFormat, const _float4& vClearColor)
{
	if (nullptr != Find_RenderTarget(strTargetTag))
		return E_FAIL;

	CRenderTarget* pRenderTarget = CRenderTarget::Create(m_pDevice, m_pContext, iWidth, iHeight, eFormat, vClearColor);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	m_RenderTargets.emplace(strTargetTag, pRenderTarget);

	return S_OK;
}

HRESULT CTarget_Manager::Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag)
{
	CRenderTarget* pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	list<CRenderTarget*>* pMRTs = Find_MRT(strMRTTag);
	if (nullptr == pMRTs)
	{
		list<CRenderTarget*> MRTs;
		m_MRTs.emplace(strMRTTag, MRTs);
	}

	m_MRTs[strMRTTag].push_back(pRenderTarget);
	Safe_AddRef(pRenderTarget);

	return S_OK;
}

HRESULT CTarget_Manager::Bind_Shader_Resource(const _wstring& strTargetTag, CShader* pShader, const _char* pConstantName)
{
	CRenderTarget* pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Bind_Shader_Resource(pShader, pConstantName);
}

HRESULT CTarget_Manager::Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV, _bool isClear)
{
	ID3D11ShaderResourceView* pSRV[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {
		nullptr
	};
	m_pContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pSRV);
	m_pContext->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pSRV);
	
	list<CRenderTarget*>* pMRTs = Find_MRT(strMRTTag);
	if (nullptr == pMRTs)
		return E_FAIL;

	m_pContext->OMGetRenderTargets(1, &m_pBackBuffer, &m_pOriginalDSV);

	ID3D11RenderTargetView* RTVs[8] = { nullptr };
	_uint iNumRTV = {};

	for (auto& pRenderTarget : *pMRTs)
	{
		if(true == isClear)
			pRenderTarget->Clear();
		RTVs[iNumRTV++] = pRenderTarget->Get_RTV();
	}

	if (nullptr != pDSV)
		m_pContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	m_pContext->OMSetRenderTargets(iNumRTV, RTVs, nullptr == pDSV ? m_pOriginalDSV : pDSV);

	return S_OK;
}

HRESULT CTarget_Manager::SetUp_MRT(ID3D11DeviceContext* pContext, const _wstring& strMRTTag)
{
	ID3D11ShaderResourceView* pSRV[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {
	nullptr
	};
	pContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pSRV);

	list<CRenderTarget*>* pMRTs = Find_MRT(strMRTTag);
	if (nullptr == pMRTs)
		return E_FAIL;

	ID3D11RenderTargetView* RTVs[8] = { nullptr };
	_uint iNumRTV = {};

	for (auto& pRenderTarget : *pMRTs)
	{
		RTVs[iNumRTV++] = pRenderTarget->Get_RTV();
	}

	pContext->OMSetRenderTargets(iNumRTV, RTVs, m_pOriginalDSV);

    return S_OK;
}

void CTarget_Manager::End_MRT()
{
	m_pContext->OMSetRenderTargets(1, &m_pBackBuffer, m_pOriginalDSV);

	Safe_Release(m_pBackBuffer);
	Safe_Release(m_pOriginalDSV);
}

HRESULT CTarget_Manager::Clear_RT(const _wstring& strTargetTag)
{
	CRenderTarget* pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	pRenderTarget->Clear();

    return S_OK;
}

HRESULT CTarget_Manager::Render()
{
	ImGui::Begin("RenderTarget");

	if (ImGui::BeginCombo("RT", "List"))
	{
		for (auto& Pair : m_RenderTargets)
		{
			if (ImGui::Selectable(WStringToString(Pair.first).c_str()))
			{
				AddRemoveRT(Pair.first, Pair.second);
			}
		}

		ImGui::EndCombo();
	}
	ImGui::End();

	for (auto& Pair : m_DebugRenderRT)
		Pair.second->Render(Pair.first);

	return S_OK;
}

void CTarget_Manager::AddRemoveRT(const _wstring& strTargetTag, CRenderTarget* pRT)
{
	auto iter = m_DebugRenderRT.find(strTargetTag);
	if (iter != m_DebugRenderRT.end())
	{
		m_DebugRenderRT.erase(iter);
		Safe_Release(pRT);
		return;
	}

	m_DebugRenderRT.emplace(strTargetTag, pRT);
	Safe_AddRef(pRT);
}
#ifdef _DEBUG

HRESULT CTarget_Manager::Ready_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	CRenderTarget* pRenderTarget = Find_RenderTarget(strTargetTag);
	if (nullptr == pRenderTarget)
		return E_FAIL;

	return pRenderTarget->Ready_Debug(fX, fY, fSizeX, fSizeY);
}

HRESULT CTarget_Manager::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	for (auto& Pair : m_RenderTargets)
	{
		if(nullptr != Pair.second)
			Pair.second->Render(pShader, pVIBuffer);
	}

	return S_OK;
}



#endif

CRenderTarget* CTarget_Manager::Find_RenderTarget(const _wstring& strTargetTag)
{
	auto iter = m_RenderTargets.find(strTargetTag);
	if (iter == m_RenderTargets.end())
		return nullptr;

	return iter->second;
}

list<class CRenderTarget*>* CTarget_Manager::Find_MRT(const _wstring& strMRTTag)
{
	auto iter = m_MRTs.find(strMRTTag);
	if (iter == m_MRTs.end())
		return nullptr;

	return &(iter->second);
}

CTarget_Manager* CTarget_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return new CTarget_Manager(pDevice, pContext);
}

void CTarget_Manager::Free()
{
	__super::Free();

	for (auto& Pair : m_MRTs)
	{
		for (auto& pRenderTarget : Pair.second)
			Safe_Release(pRenderTarget);
		Pair.second.clear();
	}
	m_MRTs.clear();

	for (auto& Pair : m_RenderTargets)
		Safe_Release(Pair.second);
	m_RenderTargets.clear();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

#ifdef _DEBUG
	for (auto& Pair : m_DebugRenderRT)
		Safe_Release(Pair.second);
	m_DebugRenderRT.clear();
#endif
}
