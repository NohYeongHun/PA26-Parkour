#pragma once
#include "Base.h"

NS_BEGIN(Client)

class CGameSystem : public CBase
{
	DECLARE_SINGLETON(CGameSystem)
	using TriggerCallback = function<void(void*)>;

private:
	explicit CGameSystem();
	virtual ~CGameSystem() = default;

public:
#pragma region GAMESYSTEM
	void		Ready_GameSystem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void		Update(_float fTimeDelta);
	void		Clear_Resource();
#pragma endregion

#pragma region MOUSECONTROLLER
	void  Register_Mouse(class CMouse* pMouse);
	void  Set_MouseFix(_bool isFix);
	_bool IsFix();
#pragma endregion

#pragma region PARSER
	void Ready_Prototype_Map(const _char* pDataFilePath, LEVEL eLevel, const _char* pModelFilePath);
	void Clone_MapObjects(LEVEL eLevel);
#pragma endregion

#pragma region SONORO_MANAGER

#pragma endregion


#pragma region TRIGGER
	//레벨 전환시 초기화 고려.
	void Clear_TriggerCallBack();
#pragma endregion


private:
	class CParser* m_pParser = { nullptr };
	class CMouseController* m_pMouseController = { nullptr };
	unordered_map<_uint, vector<TriggerCallback>> m_TriggerEvents;
	Mutex m_Mutex;




public:
	void Release_System();
	virtual	void Free() override;
};


NS_END