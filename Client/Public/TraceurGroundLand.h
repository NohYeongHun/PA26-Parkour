#pragma once
#include "GroundState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurGroundLand final : public CGroundState
{
public:
	explicit CTraceurGroundLand() = default;
	virtual ~CTraceurGroundLand() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Update_Animations(_float fTimeDelta) override;
	virtual void Check_State() override;

private:
	virtual void SetUp_Animations() override;
	

public:
	static CTraceurGroundLand* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END
