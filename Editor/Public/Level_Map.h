#pragma once
#include "Level.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Editor)

class CLevel_Map final : public CLevel
{
	enum Menu { MENU_OBJECT, MENU_RANDSCAPE, MENU_LIGHT, MENU_MAPSAVELOAD, MENU_OBJECTLOAD,MENU_OBJECTTYPE, END };
private:
	explicit CLevel_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Map() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;

	void Menu_Select();
	void Menu_Object();
	void Menu_RandSacpe();
	void Menu_Light();
	void Menu_Save_Load();
	void Menu_Model_Load();
	void Menu_Object_Type();
	void Load_Objects();


	void Ready_Map_Load_Prototype();
	void Ready_Debris_Prototype(const _char* pModelName);
	_bool NameCheck(const _string& ModelName, const _string& Name);
	void ShaderChange(const _string& ModelName, _uint* pShaderIndex);
private:
	HRESULT Ready_Static_Component();
	void Ready_Event();
	void Make_MousePos();
	void Container_Info();
	void Create_TriggerBox();
	void Create_SlideBox();

	void Load_Foliage();
public:
	static _float3 m_vWorldPos;
	static _float3 m_vWorldDir;
	static _float4 m_vPickedPos;
	
private:
	Menu m_eMenu = { END };
	_uint m_iLevel = ENUM_CLASS(LEVEL::MAP);
	class CEdit_MapObject* m_pChildObject = { nullptr };
	class CEdit_MapObject* m_pPickedObject = { nullptr };
	class CEdit_MapObject_Instance* m_pPickedInstanceObject = { nullptr };
	class CEdit_MapObject_Destruction* m_pPickedDestructObject = {nullptr};
	class CEdit_TriggerBox* m_pPickedTriggerBox = { nullptr };
	class CEdit_MonsterSpawnor* m_pPickedSpawnor = { nullptr };
	class CEdit_Meteo* m_pPickedMeteo = { nullptr };
	class CEdit_MapObject_Water* m_pPickedWater = { nullptr };
	class CEdit_MapObject_Collaps* m_pPickedCollaps = { nullptr };
	class CEdit_LightManager* m_pLightManager = { nullptr };
	class CEdit_MapEffectCollector* m_pEffectCollector = { nullptr };
	class CEdit_FireFly_Manager* m_pFlyManager = { nullptr };
	class CEdit_SlideZone* m_pPickedSlideBox = { nullptr };
	unordered_set< _string> m_szPrototypeName;

	class CEdit_LightObject* m_pPickedLightObject = { nullptr };
	class CEdit_PreViewModel* m_pPreViewObject = { nullptr };

	class CEdit_Brush* m_pBrush = { nullptr };

	class CShader_Interface* pShaderInterface = { nullptr };
	class CAnimationTool* m_pAnimationTool = { nullptr };

	unordered_map<string, vector<CGameObject*>> m_SaveObjects;
	unordered_map<_uint , vector<CGameObject*>> m_SaveInstanceObjects;
	unordered_map<string, class CEdit_MapObject*> m_ContainerObjects;
	_float m_fNearDistance= { FLT_MAX };
	_float m_fNearDistance_Instance = {FLT_MAX};

	vector<_string> m_ModelPaths;
	vector<_string> m_FoliagePaths;
	_wstring m_szPreViewModelName;
	_bool m_LoadMenu = { false };

	_string m_CurrentObjectMode;
	_uint m_eObjectType = {};
	mutex m_Mutex;
	_float m_TriggerBoxExtends[3] = { 10.f,10.f,10.f };
	string m_FolderPath;

	_bool m_Effect = { false };
	_bool m_FireFly = { false };
	_bool m_ManageTrigger = { true };
public:
	static		CLevel_Map*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END