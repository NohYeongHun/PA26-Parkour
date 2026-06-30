#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CLayer final : public CBase
{
private:
	explicit CLayer();
	virtual ~CLayer() = default;

public:
	HRESULT					Add_GameObject(class CGameObject* pObject);
	class CComponent*	Get_Component(_uint iGameObjectIndex, const _wstring& strComponentTag);
	void						Change_TimeRate(_float fTimeRate, _bool isTimeStop) {
		m_fTimeRate = fTimeRate; 
		m_isTimeStop = isTimeStop;
	}
	void						Change_TimeRate(_float fTimeRate, _float fDuration) { 
		m_fTimeRate = fTimeRate;
		m_fDuration = fDuration;
	}

public:
	void	Priority_Update(_float fTimeDelta);
	void	Update(_float fTimeDelta);
	void	Late_Update(_float fTimeDelta);

private:
	vector<class CGameObject*>	m_Objects;

	_float									m_fTimeRate = {};
	_float									m_fDuration = {};
	_bool									m_isTimeStop = { false };

public:
	static		CLayer*	Create();
	virtual		void		Free() override;
};

NS_END