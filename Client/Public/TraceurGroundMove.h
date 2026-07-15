#pragma once
#include "GroundState.h"
#include "Client_Define.h"

NS_BEGIN(Client)

class CTraceurGroundMove final : public CGroundState
{
private:
	static constexpr _float FVAULT_WARP_MIN         = 0.8f;
	static constexpr _float FVAULT_WARP_MAX         = 1.3f;
	static constexpr _float FVAULT_LANDING_MARGIN   = 0.3f;
	static constexpr _float FVAULT_MAX_LANDING_DROP = 2.0f;
	static constexpr _float FWALLRUN_COOLDOWN       = 0.3f; // 상태 진입 직후 WallRun 재진입 금지 시간

public:
	explicit CTraceurGroundMove() = default;
	virtual ~CTraceurGroundMove() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Check_State() override;
	void Update_Animations(_float fTimeDelta) override;
	void Check_Physics(_float fTimeDelta) override;

private:
	virtual void SetUp_Animations() override;

private:
	_float m_fFallTime = {};
	_float m_fWallRunCooldown = 0.f; // ClimbRun에서 착지 복귀 직후 즉시 재진입(armed 리셋 루프) 방지

public:
	static CTraceurGroundMove* Create(class CTraceur* pOwner);
	virtual void Free() override;
};

NS_END
