#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CShader : public CComponent
{
private:
	explicit CShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CShader(const CShader& Prototype);
	virtual ~CShader() = default;

public:
	virtual HRESULT		Initialize_Prototype(const _tchar* pFilePath, const D3D11_INPUT_ELEMENT_DESC* Elements, _uint iNumElements);
	virtual HRESULT		Initialize_Clone(void* pArg);

public:
	HRESULT					Begin(_uint iPassIndex);
	HRESULT					Begin(_uint iPassIndex, ID3D11DeviceContext* pDC);
	HRESULT					Bind_Matrix(const _char* pConstantName, const _float4x4* pMatrix);
	HRESULT					Bind_Matrices(const _char* pConstantName, const _float4x4* Matrices, _uint iNumMatirces);
	HRESULT					Bind_Texture(const _char* pConstantName, ID3D11ShaderResourceView* pSRV);
	HRESULT					Bind_Textures(const _char* pConstantName, ID3D11ShaderResourceView** ppSRV, _uint iNumTextures);
	HRESULT					Bind_Value(const _char* pConstantName, const void* pValue, _uint iLength);
	HRESULT					Bind_SRV(const _char* pConstantName, ID3D11ShaderResourceView* pSRV);


public:
	void					UndBind_All_VS_SRV();
#ifdef _DEBUG
	//
	_uint						Get_PassCount() { return m_iNumPasses; }
	const char*				Get_PassName(_uint iNumPass);
#endif

private:
	ComPtr<ID3DX11Effect>			m_pEffect = { nullptr };
	vector<ID3D11InputLayout*>				m_InputLayouts;
	_uint									m_iNumPasses = {};

	mutex									m_Mutex;

public:
	static		CShader*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* strFilePath, const D3D11_INPUT_ELEMENT_DESC* Elements, _uint iNumElements);
	virtual		CComponent*	Clone(void* pArg);
	virtual		void				Free() override;
};

NS_END