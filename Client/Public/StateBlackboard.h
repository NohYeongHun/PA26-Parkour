#pragma once
#include "Component.h"

NS_BEGIN(Client)
class CStateBlackboard final : public Engine::CComponent
{
protected:
	explicit CStateBlackboard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CStateBlackboard(const CStateBlackboard& Prototype);
	virtual ~CStateBlackboard() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize_Clone(void* pArg) override;

public:
	_uint Register_Bool(const _string& strName);
	_uint Find_Slot(const _string& strName) const;

	vector<_uint> Collect_Slots(const _string& strPrefix) const;

	void  Set(_uint iSlot, _bool isOn);
	_bool Get(_uint iSlot) const;

	void  Set(const _string& strName, _bool isOn);
	_bool Get(const _string& strName) const;

	void  Clear_Bools();

	void  Set_Notify(const _string& strName, _bool isOn);
	void  Clear_Notifies();

private:
	map<_string, _uint> m_BoolSlots;
	vector<_bool>       m_BoolValues;
	map<_string, _bool> m_NotifyLatch;

public:
	static CStateBlackboard* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
