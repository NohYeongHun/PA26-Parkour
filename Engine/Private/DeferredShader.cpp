#include "EnginePch.h"
#include "DeferredShader.h"

#include "GameInstance.h"

CDeferredShader::CDeferredShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent { pDevice, pContext }
{
}

CDeferredShader::CDeferredShader(const CDeferredShader& Prototype)
    : CComponent { Prototype },
    m_InputLayouts { Prototype.m_InputLayouts }
{
    for (auto& pInputLayOut : m_InputLayouts)
        Safe_AddRef(pInputLayOut);
}

HRESULT CDeferredShader::Initialize_Prototype(const _tchar* pFilePath, const D3D11_INPUT_ELEMENT_DESC* Elements, _uint iNumElements, const _wstring& strEffectTag)
{
    _uint iHlslFlag = {};

#ifdef _DEBUG
    iHlslFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    iHlslFlag = D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif

	_uint iNumThread = m_pGameInstance->Get_NumThread();

	vector<ID3DX11Effect*> Effects;
	for (_uint i = 0; i < iNumThread; ++i)
	{
		ID3DX11Effect* pEffect = { nullptr };
		if (FAILED(D3DX11CompileEffectFromFile(pFilePath, nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE, iHlslFlag, 0,
			m_pDevice, &pEffect, nullptr)))
			CRASH("Deferred Shader");
		Effects.push_back(pEffect);
	}
	m_pGameInstance->Add_Effects(strEffectTag, Effects);

    ID3DX11EffectTechnique* pTechnique = Effects[0]->GetTechniqueByIndex(0);
    if (nullptr == pTechnique)
        return E_FAIL;
    D3DX11_TECHNIQUE_DESC TechniqueDesc = {};
    pTechnique->GetDesc(&TechniqueDesc);
    _uint iNumPasses = TechniqueDesc.Passes;
    
    for (_uint i = 0; i < iNumPasses; ++i)
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

HRESULT CDeferredShader::Initialize_Clone(void* pArg)
{
    return S_OK;
}

HRESULT CDeferredShader::Begin(_uint iPassIndex, ID3D11DeviceContext* pDC, ID3DX11Effect* pEffect)
{
	{
		lock_guard<mutex> lock(m_Mutex);
		if (FAILED(pEffect->GetTechniqueByIndex(0)->GetPassByIndex(iPassIndex)->Apply(0, pDC)))
			return E_FAIL;

		pDC->IASetInputLayout(m_InputLayouts[iPassIndex]);
	}
	return S_OK;
}

HRESULT CDeferredShader::Bind_Matrix(const _char* pConstantName, const _float4x4* pMatrix, ID3DX11Effect* pEffect)
{
    ID3DX11EffectVariable* pVariable = pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    ID3DX11EffectMatrixVariable* pMatrixVariable = pVariable->AsMatrix();
    if (nullptr == pMatrixVariable)
        return E_FAIL;

    return pMatrixVariable->SetMatrix(reinterpret_cast<const _float*>(pMatrix));
}

HRESULT CDeferredShader::Bind_Texture(const _char* pConstantName, ID3D11ShaderResourceView* pSRV, ID3DX11Effect* pEffect)
{
    ID3DX11EffectVariable* pVariable = pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;

    ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
    if (nullptr == pSRVariable)
        return E_FAIL;

    return pSRVariable->SetResource(pSRV);
}

HRESULT CDeferredShader::Bind_Textures(const _char* pConstantName, ID3D11ShaderResourceView** ppSRV, _uint iNumTextures, ID3DX11Effect* pEffect)
{
	ID3DX11EffectVariable* pVariable = pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVariable)
		return E_FAIL;

	return pSRVariable->SetResourceArray(ppSRV, 0, iNumTextures);
}

HRESULT CDeferredShader::Bind_Value(const _char* pConstantName, const void* pValue, _uint iLength, ID3DX11Effect* pEffect)
{
    ID3DX11EffectVariable* pVariable = pEffect->GetVariableByName(pConstantName);
    if (nullptr == pVariable)
        return E_FAIL;
    return pVariable->SetRawValue(pValue, 0, iLength);

}

HRESULT CDeferredShader::Clear_Textures(const _char* pConstantName, _uint iNumTexture, ID3DX11Effect* pEffect)
{
	ID3DX11EffectVariable* pVariable = pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ID3DX11EffectShaderResourceVariable* pSRVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVariable)
		return E_FAIL;

	ID3D11ShaderResourceView** nullSRV = new ID3D11ShaderResourceView * [iNumTexture] {nullptr};
	pSRVariable->SetResourceArray(nullSRV, 0, iNumTexture);
	Safe_Delete_Array(nullSRV);
	return S_OK;
}

CDeferredShader* CDeferredShader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, const D3D11_INPUT_ELEMENT_DESC* Elements, _uint iNumElements, const _wstring& strEffectTag)
{
    CDeferredShader* pInstance = new CDeferredShader(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype(strFilePath, Elements, iNumElements, strEffectTag)))
    {
        MSG_BOX("Failed to Create : Shader");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CComponent* CDeferredShader::Clone(void* pArg)
{
    CDeferredShader* pClone = new CDeferredShader(*this);

    if (FAILED(pClone->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : Shader (Clone)");
        Safe_Release(pClone);
    }

    return pClone;
}

void CDeferredShader::Free()
{
    __super::Free();

    for (auto& pInputLayout : m_InputLayouts)
        Safe_Release(pInputLayout);
    m_InputLayouts.clear();
}
