#include "ClientPch.h"
#include "TraceurAirJump.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurAirJump::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurAirJump::Jump);

	return S_OK;
}

void CTraceurAirJump::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurAirJump::Jump);
	m_pColliderCom->Set_Gravity(false);
	State_Reset();
}

void CTraceurAirJump::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(false);
}

void CTraceurAirJump::Check_State()
{
	m_States[LAND] = m_pColliderCom->IsLand();
	m_States[MOVE] = m_pInputControllerCom->Check_AnyInput(m_iMoveKey);
	m_States[RUN]  = m_States[MOVE] && m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT));
}

void CTraceurAirJump::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	_float fTargetWeight = 0.f;
	if (m_States[RUN])
		fTargetWeight = 0.5f;
	else if (m_States[MOVE])
		fTargetWeight = 0.2f;

	_vector vWorldDir = CMovementComponent::Calc_GroundDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());

	m_pMoveCom->Move(vWorldDir, fTimeDelta, fTargetWeight);
}

void CTraceurAirJump::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurAirJump::Jump),
		{ &m_fTrackPosition, "Jump", 1.f, 0.1f, 0.f, false }, { 3.f, true, true, true });
}

void CTraceurAirJump::SetUp_Transitions()
{
	// AnimEnd + 미착지 → AirFall
	Add_Transition(
		[this] { return m_IsAnimationEnd && !m_States[LAND]; },
		{ ENUM_CLASS(EStateCategory::AIR), ENUM_CLASS(ETraceurAirState::Fall) }
	);
	// 착지 → GroundLand (AnimEnd 없이도 착지하면 전환)
	Add_Transition(
		[this] { return m_States[LAND]; },
		{ ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Land) },
		ENUM_CLASS(ETraceurGroundLand::FallingToLanding)
	);
}

void CTraceurAirJump::State_Reset()
{
	for (_uint i = 0; i < STATE::END; ++i)
		m_States[i] = false;
}

CTraceurAirJump* CTraceurAirJump::Create(CTraceur* pOwner)
{
	CTraceurAirJump* pInstance = new CTraceurAirJump();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurAirJump");
		return nullptr;
	}

	return pInstance;
}

void CTraceurAirJump::Free()
{
	__super::Free();
}
