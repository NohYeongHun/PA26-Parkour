#pragma once
#include "GroundState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurGroundStand final : public CGroundState
{
public:
	explicit CTraceurGroundStand() = default;
	virtual ~CTraceurGroundStand() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Update_Animations(_float fTimeDelta) override;

private:
	virtual void SetUp_Animations() override;

public:
	static CTraceurGroundStand* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END
