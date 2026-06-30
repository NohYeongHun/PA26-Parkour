#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CHdrTexture final : public CComponent
{
private:
	CHdrTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHdrTexture(const CHdrTexture& Prototype) = delete;
	virtual ~CHdrTexture() = default;

public:
	virtual HRESULT				Initialize_Prototype(const _tchar* pFilePath, _uint iNumTextures);
	virtual	HRESULT				Initialize_Clone(void* pArg) override;

	HRESULT						Bind_Shader_Resource(class CShader* pShader, const _char* pConstantName, _uint iTextureIndex);
	HRESULT						Bind_Shader_Resource(class CShader* pShader, const _char* pConstantName);
	ID3D11ShaderResourceView*	Get_SRV(_uint iTextureIndex);

	_float						Get_MaxFrame(_uint iTextureIndex);

private:
	_uint								m_iNumTextures = {};

	vector<_float>						m_MaxFrames;
	vector<ID3D11ShaderResourceView*>	m_HdrSRVs;

public:
	static CHdrTexture* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath, _uint iNumTextures);
	virtual CComponent* Clone(void* pArg) { return nullptr; }
	virtual void Free() override;
};

NS_END