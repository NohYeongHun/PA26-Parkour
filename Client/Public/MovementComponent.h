#pragma once
#include "Component.h"

NS_BEGIN(Client)
class CMovementComponent final : public Engine::CComponent
{
protected:
	explicit CMovementComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMovementComponent(const CMovementComponent& Prototype);
	virtual ~CMovementComponent() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;

public:
	void Set_MovementType(MOVEMENT_TYPE eMovementType);

public:
	// 정적 유틸 함수.
	static ACTORDIR Calculate_Direction(const class CInputController* pInputController);
	static _vector  Calc_GroundDir(ACTORDIR eDir, _fvector vCamForward, _fvector vCamRight);
	static _vector	Calc_ClimbDir(ACTORDIR eDir, _fvector vClimbNormal, _fvector vWorldUp);

public:
	void Move(_fvector vWorldDir, _float fTimeDelta, _float fTargetWeight);

	void Update_ClimbBlendWeight(ACTORDIR eDir, _float fTimeDelta);

	const _float* Get_LocomotionWeightPtr() const { return &m_fLocomotionWeight; }
	_float        Get_LocomotionWeight()    const { return m_fLocomotionWeight; }

	const _float2* Get_LocomotionWeight2DPtr() const { return &m_fLocomotionWeight2D; }
	_float2        Get_LocomotionWeight2D()    const { return m_fLocomotionWeight2D; }

private:
	class CTransform* m_pTransformCom = { nullptr }; // 약한 참조.

	_float  m_fLocomotionWeight = 0.f;   
	_float2 m_fLocomotionWeight2D = {};
	_float3 m_vLastMoveDir      = {};
	_float3 m_vLastClimbDir = {};
	_float  m_fAccelTime        = 0.25f;
	_float  m_fDecelTime        = 0.15f;
	_float  m_fMaxMoveSpeed     = 0.5f;  // 가중치 1.0일 때의 월드 이동 속도
	_float  m_fMaxClimbSpeed    = 0.2f;  // 가중치 1.0일 때의 월드 이동 속도


private:
	MOVEMENT_TYPE m_eMoventType = {};

private:
	// Type에 맞는 Movement 진행.
	void GroundMove(_fvector vWorldDir, _float fTimeDelta, _float fTargetWeight);
	void ClimbMove(_fvector vWorldDir, _float fTimeDelta, _float fTargetWeight);
	void ClimbRun(_fvector vWorldDir, _float fTimeDelta, _float fTargetWeight);

public:
	static	CMovementComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

