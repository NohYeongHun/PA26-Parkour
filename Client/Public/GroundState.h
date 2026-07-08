#pragma once
#include "TraceurState.h"

NS_BEGIN(Client)
class CGroundState abstract : public CTraceurState
{
protected:
	explicit CGroundState() = default;
	virtual ~CGroundState() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;


public:
	virtual void Free() override;
};
NS_END

