#include "ClientPch.h"
#include "TraceurClimbExit.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurClimbExit::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurClimbExit::JumpFromWall);

	return S_OK;
}

void CTraceurClimbExit::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	if (!pArg || static_cast<STATE_ENTER_DESC*>(pArg)->iAnimIndex == UINT_MAX)
		m_iCurrentAnimIdx = ENUM_CLASS(ETraceurClimbExit::JumpFromWall);
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurClimbExit::OnExit()
{
	__super::OnExit();
}

void CTraceurClimbExit::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurClimbExit::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurClimbExit::JumpFromWall),
		{ &m_fTrackPosition, "JumpFromWall", 1.f, 0.05f, 0.f, false }, { 1.f, true, false, true });
}

void CTraceurClimbExit::SetUp_Transitions()
{
	// 애니 종료 + 미착지 → AirFall (FallingIdle)
	Add_Transition(
		[this] { return m_IsAnimationEnd && !m_pColliderCom->IsLand(); },
		{ ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ETraceurAirState::Fall) },
		ENUM_CLASS(ETraceurAirFall::FallALoop)
	);
	// 애니 종료 + 착지 → GroundLand
	Add_Transition(
		[this] { return m_IsAnimationEnd && m_pColliderCom->IsLand(); },
		{ ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Land) },
		ENUM_CLASS(ETraceurGroundLand::FallALandToStandingIdle)
	);
}

CTraceurClimbExit* CTraceurClimbExit::Create(CTraceur* pOwner)
{
	CTraceurClimbExit* pInstance = new CTraceurClimbExit();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurClimbExit");
		return nullptr;
	}

	return pInstance;
}

void CTraceurClimbExit::Free()
{
	__super::Free();
}
