#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CGameInstance;
class CLevel;

class CLevel_Manager final : public CBase
{
private:
	explicit CLevel_Manager();
	virtual ~CLevel_Manager() = default;

public:
	_uint					Get_CurrentLevel() { return m_iCurrentLevel; }

public:
	HRESULT				Open_Level(_uint iCurrentLevel, CLevel* pCurrentLevel);
	void					Update_Level(_float fTimeDelta);
	HRESULT				Render();
	HRESULT				Clear_CurrentLevel_Resources(_uint iNextLevel);

private:
	_uint					m_iCurrentLevel		= {};

	CGameInstance*	m_pGameInstance		= { nullptr };
	CLevel*				m_pCurrentLevel		= { nullptr };

public:
	static		CLevel_Manager*	Create();
	virtual		void					Free() override;
};

NS_END