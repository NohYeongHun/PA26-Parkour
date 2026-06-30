#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTexture;
class CShader;
class CGameInstance;
class CDecal;
class CGameObject;

class CDecal_Manager final : public CBase
{
private:
	CDecal_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDecal_Manager() = default;

public:
	HRESULT					Initialize();
	void					Update(_float fTimeDelta);
	
	HRESULT					Add_CustomDecal(CGameObject* pCustomDecalObject);
	HRESULT					Add_Decal(const _wstring& strDecalTag, 
							const _tchar* pFilePath[ENUM_CLASS(TEXTURETYPE::END)], 
							const _float3& vEmissiveLuminance);
	
	HRESULT					Add_DecalData(const _wstring& strDecalTag, const DECAL_DATA& Decal);
	
	HRESULT					Render();
	void					Clear();

private:
	HRESULT					Ready_Components();
	CDecal*					Find_Decal(const _wstring& strDecalTag);

public:
	static CDecal_Manager*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void			Free() override;


private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };
	CShader*				m_pShader = { nullptr };

	typedef unordered_map<_wstring, CDecal*> DECALS;
	DECALS					m_Decals;
	list<CGameObject*>		m_CustomDecals;


};


NS_END