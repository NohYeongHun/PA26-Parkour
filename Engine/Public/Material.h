#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CMaterial final : public CBase
{
private:
	explicit CMaterial(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMaterial() = default;

public:
	HRESULT						Initialize(const json& MaterialData);

	HRESULT						Bind_Resource(class CDeferredShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect);
	HRESULT						Bind_Resource(class CDeferredShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect);
	HRESULT						Bind_Resource(class CShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, _uint iTextureIndex);
	HRESULT						Bind_Resource(class CShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType);
	HRESULT						Clear_Resource(class CDeferredShader* pShader, const _char* pConstantName, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect);

private:
	class CGameInstance*	m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	vector<ID3D11ShaderResourceView*> m_SRVs[ENUM_CLASS(TEXTURETYPE::END)];

private:
	void							Store_Texture(const json& MaterialData, const _char* pTextureTag, TEXTURETYPE eTextureType);

public:
	static		CMaterial*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const json& MaterialData);
	virtual		void				Free() override;
};

NS_END