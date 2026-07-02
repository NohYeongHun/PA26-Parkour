#pragma once
#include "Base.h"
#include "Client_Enum.h"
#include <variant>

NS_BEGIN(Client)
class CSonoro_Manager final: public CBase
{
private:
	explicit CSonoro_Manager();
	virtual ~CSonoro_Manager() = default;

public:
	void	Set_Sonora_LightDesc(SONORA eType, const LIGHT_DESC& Desc) { m_LightDesc[ENUM_CLASS(eType)] = Desc; }

public:
	HRESULT Initialize();
	_bool* Add_To_Management(OBJECTTYPE eType, class CMapObject_Sonoro* pObjects,_bool** SonoroMode);
	_bool* Add_To_Management(OBJECTTYPE eType, class CMapObject_NonSonoro* pObjects, _bool** SonoroMode);
	_bool* Add_To_Management(INSTANCETYPE eType, class CMapObject_Instance* pObjects,_bool** SonoroMode);
	void Update(_float fTimeDelta);
	_bool IsSonoro() { return m_SonoroRender; }
	const _tchar* Get_SonoroText();
	void  Clear_Resource();

private:
	vector<class CMapObject_Sonoro*> m_SonoroObjects;
	vector<class CMapObject_NonSonoro*> m_NonSonoroObjects;
	vector<class CMapObject_Instance*> m_Instance;
	_bool m_LastSonoroMode = { false };
	class CGameInstance* m_pGameInstance = { nullptr };
	_float m_fTriggerdTime = {};
	_float4 m_vUpSpeed = {};

	_bool m_SonoroRender = { false };
	_bool m_IsUpdate = { false };
	_bool m_SonoroRigidActive = { false };
	mutex m_Mutex;

	_wstring m_EnterSonoro;
	_wstring m_ExitSonoro;
	LIGHT_DESC		m_LightDesc[ENUM_CLASS(SONORA::END)] = {};

public:
	static CSonoro_Manager* Create();
	virtual void Free()override;
};

NS_END