#pragma once
#include "GroundState.h"
#include "Client_Define.h"

NS_BEGIN(Client)

// BlendSpace 파라미터 = 0~1 정규화 가중치 (0=Idle, 0.5=Walk, 1.0=Run).
class CTraceurGroundMove final : public CGroundState
{
private:
	static constexpr _float FVAULT_WARP_MIN        = 0.8f;
	static constexpr _float FVAULT_WARP_MAX        = 1.3f;
	static constexpr _float FVAULT_LANDING_MARGIN  = 0.3f;
	static constexpr _float FVAULT_MAX_LANDING_DROP = 2.0f;

	enum STATE
	{
		MOVE = 0,
		RUN,
		VAULT,
		END
	};

public:
	explicit CTraceurGroundMove() = default;
	virtual ~CTraceurGroundMove() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

private:
	void Check_State();
	void Update_Animations(_float fTimeDelta);
	void Check_Physics(_float fTimeDelta);

	virtual void Check_StateTransition(_float fTimeDelta) override;
	virtual void SetUp_Animations() override;

	void State_Reset();

private:
	_bool      m_States[STATE::END] = {};

public:
	static CTraceurGroundMove* Create(class CTraceur* pOwner);
	virtual void Free() override;
};

NS_END
