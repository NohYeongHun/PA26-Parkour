#pragma once
#include "GroundState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurGroundVault final : public CGroundState
{
private:
	enum STATE
	{
		END
	};

public:
	explicit CTraceurGroundVault() = default;
	virtual ~CTraceurGroundVault() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

private:
	// Update
	void Check_State();
	void Update_Animations(_float fTimeDelta);
	void Check_Physics(_float fTimeDelta);

private:
	virtual void Check_StateTransition(_float fTimeDelta) override;
	virtual void SetUp_Animations() override;

private:
	void State_Reset();

private:
	// 진입 시점의 장애물 모서리(vTopEdgePos)에 맞춰 캐릭터 위치를 보정합니다. (모션워핑 정렬)
	void Align_ToObstacle();

private:
	VAULT_PLAN m_Plan{};
	_bool      m_bValidPlan = false;

public:
	static CTraceurGroundVault* Create(class CTraceur* pOwner);
	virtual void Free() override;

};
NS_END
