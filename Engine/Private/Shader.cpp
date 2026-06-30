#include "EnginePch.h"
#include "Shader.h"

CShader::CShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent { pDevice, pContext }
{
}

CShader::CShader(const CShader& Prototype)
    : CComponent { Prototype },
	m_pEffect { Prototype.m_pEffect },
    m_InputLayouts { Prototype.m_InputLayouts },
    m_iNumPasses { Prototype.m_iNumPasses }
{
	for (auto& pInputLayOut : m_InputLayouts)
	    Safe_AddRef(pInputLayOut);
}

HRESULT CShader::Initialize_Prototype(const _tchar* pFilePath, const D3D11_INPUT_ELEMENT_DESC* Elements, _uint iNumElements)
{
    _uint iHlslFlag = {};

#ifdef _DEBUG
    iHlslFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    iHlslFlag = D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif
	// 1. 임시 변수가 필요 없습니다. ComPtr이 & 연산자를 오버로딩합니다.
	// m_pEffect.GetAddressOf()는 &pRawEffect와 동일한 역할을 합니다.
	if (FAILED(D3DX11CompileEffectFromFile(pFilePath, nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, iHlslFlag, 0,
		m_pDevice, m_pEffect.GetAddressOf(), nullptr))) // <-- ComPtr 사용
		return E_FAIL;

	// 3. nullptr 체크
	if (nullptr == m_pEffect) // m_pEffect.Get()을 쓸 필요 없이 바로 비교 가능
		return E_FAIL;

    ID3DX11EffectTechnique* pTechnique = m_pEffect->GetTechniqueByIndex(0);
    if (nullptr == pTechnique)
        return E_FAIL;
    D3DX11_TECHNIQUE_DESC TechniqueDesc = {};
    pTechnique->GetDesc(&TechniqueDesc);
    m_iNumPasses = TechniqueDesc.Passes;
    
    for (_uint i = 0; i < m_iNumPasses; ++i)
    {
        ID3DX11EffectPass* pPass = pTechnique->GetPassByIndex(i);
        if (nullptr == pPass)
            return E_FAIL;
         D3DX11_PASS_DESC PassDesc = {};
        pPass->GetDesc(&PassDesc);

        ID3D11InputLayout* pInputLayout = { nullptr };
        if (FAILED(m_pDevice->CreateInputLayout(Elements, iNumElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &pInputLayout)))
            return E_FAIL;
        m_InputLayouts.push_back(pInputLayout);
    }

    return S_OK;
}

HRESULT CShader::Initialize_Clone(void* pArg)
{
    return S_OK;
}

HRESULT CShader::Begin(_uint iPassIndex)
{
    if (FAILED(m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(iPassIndex)->Apply(0, m_pContext)))
        return E_FAIL;

    m_pContext->IASetInputLayout(m_InputLayouts[iPassIndex]);

    return S_OK;
}

HRESULT CShader::Begin(_uint iPassIndex, ID3D11DeviceContext* pDC)
{
	if (FAILED(m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(iPassIndex)->Apply(0, pDC)))
		return E_FAIL;

	pDC->IASetInputLayout(m_InputLayouts[iPassIndex]);

	return S_OK;
}

HRESULT CShader::Bind_Matrix(const _char* pConstantName, const _float4x4* pMatrix)
{
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    ID3DX11EffectMatrixVariable* pMatrixVariable = pVariable->AsMatrix();
    if (nullptr == pMatrixVariable)
        return E_FAIL;

    return pMatrixVariable->SetMatrix(reinterpret_cast<const _float*>(pMatrix));
}

HRESULT CShader::Bind_Matrices(const _char* pConstantName, const _float4x4* Matrices, _uint iNumMatirces)
{
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    ID3DX11EffectMatrixVariable* pMatrixVariable = pVariable->AsMatrix();
    if (nullptr == pMatrixVariable)
        return E_FAIL;

    return pMatrixVariable->SetMatrixArray(reinterpret_cast<const _float*>(Matrices), 0, iNumMatirces);
}

HRESULT CShader::Bind_Texture(const _char* pConstantName, ID3D11ShaderResourceView* pSRV)
{
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
    if (nullptr == pSRVariable)
        return E_FAIL;

    return pSRVariable->SetResource(pSRV);
}

HRESULT CShader::Bind_Textures(const _char* pConstantName, ID3D11ShaderResourceView** ppSRV, _uint iNumTextures)
{
	ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVariable)
		return E_FAIL;

	return pSRVariable->SetResourceArray(ppSRV, 0, iNumTextures);
}

HRESULT CShader::Bind_Value(const _char* pConstantName, const void* pValue, _uint iLength)
{
    ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;
    return pVariable->SetRawValue(pValue, 0, iLength);
}

HRESULT CShader::Bind_SRV(const _char* pConstantName, ID3D11ShaderResourceView* pSRV)
{
	ID3DX11EffectVariable* pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVariable)
		return E_FAIL;

	return pSRVariable->SetResource(pSRV);
}

void CShader::UndBind_All_VS_SRV()
{
	// VS 최대 SRV 슬롯 개수가 128개.
	ID3D11ShaderResourceView* nullSRV[128] = {nullptr,};
	m_pContext->VSSetShaderResources(0, 128, nullSRV);
}

#ifdef _DEBUG
const char* CShader::Get_PassName(_uint iNumPass)
{
    ID3DX11EffectTechnique* pTechnique = m_pEffect->GetTechniqueByIndex(0);

    if (nullptr == pTechnique)
        return nullptr;

    D3DX11_TECHNIQUE_DESC TechniqueDesc = {};
    pTechnique->GetDesc(&TechniqueDesc);
    m_iNumPasses = TechniqueDesc.Passes;

    ID3DX11EffectPass* pPass = pTechnique->GetPassByIndex(iNumPass);
    if (nullptr == pPass)
        return nullptr;
    D3DX11_PASS_DESC PassDesc = {};
    pPass->GetDesc(&PassDesc);

    return PassDesc.Name;
}
#endif

CShader* CShader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, const D3D11_INPUT_ELEMENT_DESC* Elements, _uint iNumElements)
{
    CShader* pInstance = new CShader(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(strFilePath, Elements, iNumElements)))
    {
        MSG_BOX("Failed to Create : Shader");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CShader::Clone(void* pArg)
{
	lock_guard<mutex> lock(m_Mutex);
    CShader* pClone = new CShader(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : Shader (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CShader::Free()
{
    __super::Free();

    for (auto& pInputLayout : m_InputLayouts)
        Safe_Release(pInputLayout);
    m_InputLayouts.clear();

	m_pEffect = nullptr;
}
