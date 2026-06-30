#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CTexture final : public CComponent
{
private:
	explicit CTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CTexture(const CTexture& Prototype);
	virtual ~CTexture() = default;

public:
	virtual		HRESULT			Initialize_Prototype(const _tchar* pFilePath, _uint iNumTexture);
	virtual		HRESULT			Initialize_Clone(void* pArg);

	HRESULT						Bind_Shader_Resource(class CShader* pShader, const _char* pConstantName, _uint iTextureIndex);
	HRESULT						Bind_Shader_Resource(class CShader* pShader, const _char* pConstantName);
	ID3D11ShaderResourceView*	Get_SRV(_uint iIndex);

private:
	vector<ID3D11ShaderResourceView*>		m_SRVs;
	_uint									m_iNumTextures = {};

public:
	static		CTexture*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath, _uint iNumTexture);
	virtual		CComponent*		Clone(void* pArg);
	virtual		void			Free() override;
};

NS_END