#include "EnginePch.h"
#include "ComputeShader.h"

CComputeShader::CComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext }
{
}

CComputeShader::CComputeShader(CComputeShader& Prototype)
    : CComponent(Prototype)
    , m_pComputeShader{ Prototype.m_pComputeShader }
    , m_SRV_BindPoints{ Prototype.m_SRV_BindPoints }
    , m_UAV_BindPoints{ Prototype.m_UAV_BindPoints }
    , m_CB_BindPoints{ Prototype.m_CB_BindPoints }
    , m_ThreadInfo{ Prototype.m_ThreadInfo }
{
    Safe_AddRef(m_pComputeShader);
}

HRESULT CComputeShader::Initialize_Prototype(const _tchar* pFilePath, const SHADER_MACRO& eShaderMacro, const _string& strEntryPoint)
{
#pragma region
    _uint iHlslFlag = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG

    iHlslFlag |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else

    iHlslFlag |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif

    ID3DBlob* pCSBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;

    D3D_SHADER_MACRO shaderMacros[] =
    {
        eShaderMacro.tagX,
        eShaderMacro.tagY,
        eShaderMacro.tagZ,
        eShaderMacro.tagEnd,
    };

    m_ThreadInfo = { stoul(eShaderMacro.tagX.Definition)
        , stoul(eShaderMacro.tagY.Definition)
        , stoul(eShaderMacro.tagZ.Definition) };


    HRESULT hr = D3DCompileFromFile(pFilePath
        , shaderMacros
        , D3D_COMPILE_STANDARD_FILE_INCLUDE
        , strEntryPoint.c_str() 
        , "cs_5_0" 
        , iHlslFlag
        , 0
        , &pCSBlob 
        , &pErrorBlob 
    );

 
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
          
            const char* errorMessage = (char*)pErrorBlob->GetBufferPointer();
            MessageBoxA(nullptr, errorMessage, "HLSL Compile Error", MB_OK);
            Safe_Release(pErrorBlob);
        }
        return E_FAIL;
    }

#pragma endregion

    if (FAILED(m_pDevice->CreateComputeShader(pCSBlob->GetBufferPointer()
        , pCSBlob->GetBufferSize(), nullptr, &m_pComputeShader)))
    {
        Safe_Release(pCSBlob);
        return E_FAIL;
    }

 
    if (FAILED(Ready_Reflection(pCSBlob)))
    {
        MSG_BOX("Create Compute Shader File Read Failed");
        Safe_Release(pCSBlob);
        return E_FAIL;
    }

    Safe_Release(pCSBlob);

    return S_OK;
}

HRESULT CComputeShader::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CComputeShader::Set_SRV(const string& strName, ID3D11ShaderResourceView* pSRV)
{
    // 1. 문자열을 통해 슬롯 번호를 탐색
    auto iter = m_SRV_BindPoints.find(strName);
    if (iter != m_SRV_BindPoints.end())
        m_SRVs_To_Bind[iter->second] = pSRV;
}

void CComputeShader::Set_UAV(const string& strName, ID3D11UnorderedAccessView* pUAV)
{
    auto iter = m_UAV_BindPoints.find(strName);
    if (iter != m_UAV_BindPoints.end())
        m_UAVs_To_Bind[iter->second] = pUAV;
}

void CComputeShader::Set_ConstantBuffer(const string& strName, ID3D11Buffer* pCB)
{
    auto iter = m_CB_BindPoints.find(strName);
    if (iter != m_CB_BindPoints.end())
        m_CBs_To_Bind[iter->second] = pCB;
}

void CComputeShader::Set_Sampler(_uint iSlotIndex, ID3D11SamplerState* pSampler)
{
	auto iter = m_SAMPLERs_To_Bind.find(iSlotIndex);
	if (iter != m_SAMPLERs_To_Bind.end())
	{
		MSG_BOX("Duplicate Sampler");
		return;
	}
	m_SAMPLERs_To_Bind.emplace(iSlotIndex, pSampler);
}

void CComputeShader::Dispatch(_uint iThreadGroupCountX, _uint iThreadGroupCountY, _uint iThreadGroupCountZ)
{
    m_pContext->CSSetShader(m_pComputeShader, nullptr, 0);

    for (auto& Pair : m_SRVs_To_Bind)
        m_pContext->CSSetShaderResources(Pair.first, 1, &Pair.second);
    for (auto& Pair : m_UAVs_To_Bind)
        m_pContext->CSSetUnorderedAccessViews(Pair.first, 1, &Pair.second, nullptr);
    for (auto& Pair : m_CBs_To_Bind)
        m_pContext->CSSetConstantBuffers(Pair.first, 1, &Pair.second);
	for (auto& Pair : m_SAMPLERs_To_Bind)
		m_pContext->CSSetSamplers(Pair.first, 1, &Pair.second);

    m_pContext->Dispatch(iThreadGroupCountX, iThreadGroupCountY, iThreadGroupCountZ);

    Clear_Resources();
}


// HLSL 리플렉션 기능을 통해 컴파일 시점에 모든 레지스터 번호를 미리 찾아 캐싱해둡니다.
HRESULT CComputeShader::Ready_Reflection(ID3DBlob* pCSBlob)
{

	// 1. 셰이더 데이터 분석을 위한 리플렉션 인터페이스 생성.
    ID3D11ShaderReflection* pReflection = nullptr;
    if (FAILED(D3DReflect(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize()
        , IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&pReflection)))
    )
        return E_FAIL;

	// 2. Shader Description을 통한 바인딩된 전체 리소스 개수 파악.
    D3D11_SHADER_DESC shaderDesc;
    pReflection->GetDesc(&shaderDesc);


	// 3. 모든 바인딩 리소스를 순회 정보 추출.
    for (_uint i = 0; i < shaderDesc.BoundResources; ++i)
    {
        D3D11_SHADER_INPUT_BIND_DESC bindDesc;
        pReflection->GetResourceBindingDesc(i, &bindDesc);

        string strName = bindDesc.Name;

		// 4. 리소스 타입별로 구분하여 해당 레지스터 번호를 맵에 캐싱해둡니다.
        switch (bindDesc.Type)
        {
        case D3D_SIT_CBUFFER:
            m_CB_BindPoints[strName] = bindDesc.BindPoint;
            break;
        case D3D_SIT_TBUFFER:
        case D3D_SIT_TEXTURE:
        case D3D_SIT_STRUCTURED:
            m_SRV_BindPoints[strName] = bindDesc.BindPoint;
            break;
        case D3D_SIT_UAV_RWTYPED:
        case D3D_SIT_UAV_RWSTRUCTURED:
            m_UAV_BindPoints[strName] = bindDesc.BindPoint;
            break;
        }
    }

	// 5. 사용이 끝난 리플랙션 객체는 해제합니다.
    Safe_Release(pReflection);
    return S_OK;
}


void CComputeShader::Clear_Resources()
{
	ID3D11ShaderResourceView* pNullSRV = nullptr;
	ID3D11Buffer* pNullCB = nullptr;
	ID3D11UnorderedAccessView* pNullUAV = nullptr;
	ID3D11SamplerState* pSampler = nullptr;
    for(auto& Pair : m_SRVs_To_Bind)
        m_pContext->CSSetShaderResources(Pair.first, 1, &pNullSRV);
    for(auto& Pair : m_CBs_To_Bind)
        m_pContext->CSSetConstantBuffers(Pair.first, 1, &pNullCB);
    for(auto& Pair : m_UAVs_To_Bind)
        m_pContext->CSSetUnorderedAccessViews(Pair.first, 1, &pNullUAV, nullptr);
	for (auto& Pair : m_SAMPLERs_To_Bind)
		m_pContext->CSSetSamplers(Pair.first, 1, &pSampler);

    m_SRVs_To_Bind.clear();
    m_UAVs_To_Bind.clear();
    m_CBs_To_Bind.clear();
	m_SAMPLERs_To_Bind.clear();
}

CComputeShader* CComputeShader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath, const SHADER_MACRO& eShaderMacro, _string strEntryPoint)
{
    CComputeShader* pInstance = new CComputeShader(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype(pFilePath, eShaderMacro, strEntryPoint)))
    {
        MSG_BOX("Failed to Create : CComputeShader");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CComputeShader::Clone(void* pArg)
{
    CComputeShader* pClone = new CComputeShader(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Clone : ComputeShader");
        Safe_Release(pClone);
    }

    return pClone;
}

void CComputeShader::Free()
{
    CComponent::Free();

    m_SRVs_To_Bind.clear();
    m_UAVs_To_Bind.clear();
    m_CBs_To_Bind.clear();

	Clear_Resources();

    Safe_Release(m_pComputeShader);
}