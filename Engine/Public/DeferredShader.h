#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CDeferredShader : public CComponent
{
private:
	explicit CDeferredShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CDeferredShader(const CDeferredShader& Prototype);
	virtual ~CDeferredShader() = default;

public:
	virtual HRESULT		Initialize_Prototype(const _tchar* pFilePath, const D3D11_INPUT_ELEMENT_DESC* Elements, _uint iNumElements, const _wstring& strEffectTag);
	virtual HRESULT		Initialize_Clone(void* pArg);

public:
	HRESULT					Begin(_uint iPassIndex, ID3D11DeviceContext* pDC, ID3DX11Effect* pEffect);
	HRESULT					Bind_Matrix(const _char* pConstantName, const _float4x4* pMatrix, ID3DX11Effect* pEffect);
	HRESULT					Bind_Texture(const _char* pConstantName, ID3D11ShaderResourceView* pSRV, ID3DX11Effect* pEffect);
	HRESULT					Bind_Textures(const _char* pConstantName, ID3D11ShaderResourceView** ppSRV, _uint iNumTextures, ID3DX11Effect* pEffect);
	HRESULT					Bind_Value(const _char* pConstantName, const void* pValue, _uint iLength, ID3DX11Effect* pEffect);

	HRESULT					Clear_Textures(const _char* pConstantName, _uint iNumTexture, ID3DX11Effect* pEffect);

private:
	vector<ID3D11InputLayout*>	m_InputLayouts;
	mutex m_Mutex;
public:
	static		CDeferredShader*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, const D3D11_INPUT_ELEMENT_DESC* Elements, _uint iNumElements, const _wstring& strEffectTag);
	virtual		CComponent*	Clone(void* pArg);
	virtual		void				Free() override;
};

NS_END