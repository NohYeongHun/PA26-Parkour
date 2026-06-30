#pragma once
#include "Base.h"
#include "Effect_Radial.h"

NS_BEGIN(Editor)

class CRadial_Controller final : public CBase
{
private:
	explicit CRadial_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRadial_Controller() = default;

#pragma region
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion


public:
	void Radial_Tab();

	void Radial_Base_Tab(CEffect_Radial::RADIAL_DESC& tEffectRadialDesc, _bool& IsCreate);

public:
	void UpdateSelected_RadialFormTag(_wstring RadialTag);
	
	CEffect_Radial::RADIAL_DESC* Get_RadialDesc(_wstring& RadialTag);
	void	Set_RadialDesc(_wstring& RadialTag, CEffect_Radial::RADIAL_DESC& tEffectRadialDesc);

	void Set_RadialTag(const _char* szRadialTag);

	void Remove_Desc(const _wstring& DescTag);

//public:
//	void DecalData_To_Json(DECAL_DATADESC* pDesc);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	


	_char														m_RadialTag[MAX_PATH] = {};
	_bool														m_bTagFlag = false;

	map<const _wstring, CEffect_Radial::RADIAL_DESC>			m_tRadialDesc = {};

	_int														m_iSelectedRadial = 0;
	_bool														m_bSelectedRadial = false;
	CEffect_Radial::RADIAL_DESC*								m_pSelectedRadialDesc = { nullptr };



public:
	static CRadial_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END