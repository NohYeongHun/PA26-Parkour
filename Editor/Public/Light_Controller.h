#pragma once
#include "Base.h"
#include "Effect_Light.h"

NS_BEGIN(Editor)

class CLight_Controller final : public CBase
{
public:
	typedef struct FXLightData {
		_wstring wstrLightTag = {};
		_float4	vColor;
	}LIGHT_DATADESC;


private:
	explicit CLight_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLight_Controller() = default;

#pragma region
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	//void Texture_Loading(const char* TextureName, const _tchar* pFilePath);

	void Load_AllJsonLightDataFromFolder(const _string& strFolderPath);
	void Load_LightData_FromJson(const _string& strFilePath);

public:
	void Light_Tab();

	void Light_Base_Tab(CEffect_Light::LIGHT_DESC& tEffectLightDesc, _bool& IsCreate);
	void LightData_Base_Tab();

public:
	void UpdateSelected_LightFormTag(_wstring LightTag);
	
	CEffect_Light::LIGHT_DESC* Get_LightDesc(_wstring& LightTag);
	void	Set_LightDesc(_wstring& LightTag, CEffect_Light::LIGHT_DESC& LightDesc);

	void Set_LightTag(const _char* szLightTag);

	void Remove_Desc(const _wstring& DescTag);

public:
	void LightData_To_Json(LIGHT_DATADESC* pDesc);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	

	//매니저에 추가할 데칼 정보
	_char														m_LightDataTag[MAX_PATH] = {};
	_bool														m_bDataTagFlag = false;

	vector<_string>												m_LightData;
	_int														m_iSelectedDacalData = -1;

	//클래스로 만들 데칼이 가질 정보.
	//vector<Light_TEXTURE>										m_Textures = {};
	//_int														m_iSelectedTexture = -1;
	//_bool														m_TexturPopOpend = false;

	_bool														m_IsLightDataFlag = false;

	_char														m_LightTag[MAX_PATH] = {};
	_bool														m_bTagFlag = false;
	_bool														m_bDataFlag = false;

	map<const _wstring, CEffect_Light::LIGHT_DESC>				m_tLightDesc = {};

	_int														m_iSelectedLight = 0;
	_bool														m_bSelectedLight = false;
	CEffect_Light::LIGHT_DESC*									m_pSelectedLightDesc = { nullptr };

	_float														m_fColor[4] = {1.f, 1.f, 1.f, 1.f};		

	//Light 베이스 정보
	_float														m_vBaseColor[4] = { 1.f, 1.f, 1.f, 1.f };

public:
	static CLight_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END