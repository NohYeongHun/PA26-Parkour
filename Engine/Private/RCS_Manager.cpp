#include "EnginePch.h"
#include "RCS_Manager.h"
#include "Shader.h"
#include "RendererCS.h"

CRCS_Manager::CRCS_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice { pDevice}
    , m_pContext { pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

ID3D11ShaderResourceView* CRCS_Manager::Get_RCS_SRV(const _wstring& strRCSTag, _uint iMipLevel)
{
    CRendererCS* pRCS = Find_RCS(strRCSTag);
    if (nullptr == pRCS)
        return nullptr;

    return pRCS->Get_SRV(iMipLevel);
}

HRESULT CRCS_Manager::Add_RCS(const _wstring& strRCSTag, void* pDesc)
{
    if (nullptr != Find_RCS(strRCSTag))
        return E_FAIL;

    CRendererCS* pRCS = CRendererCS::Create(m_pDevice, m_pContext, pDesc);
    ASSERT_CRASH(pRCS);

    pRCS->Setting_UAV_Data("OutputTexture");

    m_RCSs.emplace(strRCSTag, pRCS);

    return S_OK;
}

HRESULT CRCS_Manager::Add_BufferData(const _wstring& strRCSTag, const _char* pConstantName, void* pData, _uint iLength)
{
    CRendererCS* pRCS = Find_RCS(strRCSTag);
    if (nullptr == pRCS)
        return E_FAIL;

    pRCS->Add_BufferData(pConstantName, pData, iLength);

    return S_OK;
}

HRESULT CRCS_Manager::Add_SRVData(const _wstring& strRCSTag, const _char* pConstantName, ID3D11ShaderResourceView* pSRV)
{
    CRendererCS* pRCS = Find_RCS(strRCSTag);
    if (nullptr == pRCS)
        return E_FAIL;

    pRCS->Add_SRVData(pConstantName, pSRV);

    return S_OK;
}

HRESULT CRCS_Manager::Add_SamplerState(const _wstring& strRCSTag, _uint iSlotIndex, ID3D11SamplerState* pSampler)
{
	CRendererCS* pRCS = Find_RCS(strRCSTag);
	if (nullptr == pRCS)
		return E_FAIL;

	pRCS->Add_SamplerState(iSlotIndex, pSampler);

	return S_OK;
}

HRESULT CRCS_Manager::Setting_UAV_Data(const _wstring& strRCSTag, const _char* pConstantName)
{
    CRendererCS* pRCS = Find_RCS(strRCSTag);
    if (nullptr == pRCS)
        return E_FAIL;

    pRCS->Setting_UAV_Data(pConstantName);

    return S_OK;
}

HRESULT CRCS_Manager::Bind_RendererCS(const _wstring& strRCSTag, CShader* pShader, const _char* pConstantName, _uint iMipLevel)
{
    CRendererCS* pRCS = Find_RCS(strRCSTag);
    if (nullptr == pRCS)
        return E_FAIL;

    return pShader->Bind_Texture(pConstantName, pRCS->Get_SRV(iMipLevel));
}

HRESULT CRCS_Manager::Begin_RCS(const _wstring& strRCSTag, _uint iWidth, _uint iHeight, _uint iMipLevel)
{
    CRendererCS* pRCS = Find_RCS(strRCSTag);
    if (nullptr == pRCS)
        return E_FAIL;

    pRCS->Clear(iMipLevel);
    pRCS->Bind_Resources(iMipLevel);
    pRCS->Dispatch(iWidth, iHeight);

    ID3D11SamplerState* pNullSampler[D3D11_COMMONSHADER_SAMPLER_REGISTER_COUNT] = { nullptr };
    m_pContext->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_REGISTER_COUNT, pNullSampler);
    m_pContext->CSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_REGISTER_COUNT, pNullSampler);

    return S_OK;
}

void CRCS_Manager::Clear_RCS(const _wstring& strRCSTag, _uint iMipLevel)
{
    CRendererCS* pRCS = Find_RCS(strRCSTag);
    if (nullptr == pRCS)
        return;

    pRCS->Clear(iMipLevel);
}

HRESULT CRCS_Manager::Debug_Render()
{
    ImGui::Begin("RendererCS");

    if (ImGui::BeginCombo("RCS", "List"))
    {
        for (auto& Pair : m_RCSs)
        {
            if (ImGui::Selectable(WStringToString(Pair.first).c_str()))
            {
                AddRemoveRCS(Pair.first, Pair.second);
            }
        }

        ImGui::EndCombo();
    }
    ImGui::End();

    for (auto& Pair : m_RenderRCSs)
        Pair.second->Debug_Render(Pair.first);

    return S_OK;
}
void CRCS_Manager::AddRemoveRCS(const _wstring& strRCSTag, CRendererCS* pRCS)
{
    auto iter = m_RenderRCSs.find(strRCSTag);
    if (iter != m_RenderRCSs.end())
    {
        m_RenderRCSs.erase(iter);
        Safe_Release(pRCS);
        return;
    }

    m_RenderRCSs.emplace(strRCSTag, pRCS);
    Safe_AddRef(pRCS);
}

CRendererCS* CRCS_Manager::Find_RCS(const _wstring& strRCSTag)
{
    auto iter = m_RCSs.find(strRCSTag);
    if (iter == m_RCSs.end())
        return nullptr;
    return iter->second;
}



CRCS_Manager* CRCS_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CRCS_Manager(pDevice, pContext);
}

void CRCS_Manager::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);

    for (auto& Pair : m_RCSs)
        Safe_Release(Pair.second);
    m_RCSs.clear();

#ifdef _DEBUG
    for (auto& Pair : m_RenderRCSs)
        Safe_Release(Pair.second);
    m_RenderRCSs.clear();
#endif
}
