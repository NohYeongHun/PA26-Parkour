#include "ClientPch.h"
#include "TraceurAirFall.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurAirFall::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	Register_Flag("Land");

	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurAirFall::FallingIdle);

	return S_OK;
}

void CTraceurAirFall::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurAirFall::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurAirFall::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurAirFall::Check_State()
{
	Set_Flag("Land", m_pColliderCom->IsLand());
}

void CTraceurAirFall::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurAirFall::FallingIdle),
		{ &m_fTrackPosition, "FallingIdle", 1.f, 0.05f, 0.2f, 0.f, false }, { 1.f, true, true, true });

	CState::Add_Animations(ENUM_CLASS(ETraceurAirFall::FallALoop),
		{ &m_fTrackPosition, "FallALoop", 1.f, 0.1f, 0.2f, 0.f, false }, { 1.f, true, true, true });

	CState::Add_Animations(ENUM_CLASS(ETraceurAirFall::JumpFromWall),
		{ &m_fTrackPosition, "JumpFromWall", 1.f, 0.05f, 0.2f, 0.f, false }, { 1.f, true, true, true });
}

CTraceurAirFall* CTraceurAirFall::Create(CTraceur* pOwner)
{
	CTraceurAirFall* pInstance = new CTraceurAirFall();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurAirFall");
		return nullptr;
	}

	return pInstance;
}

void CTraceurAirFall::Free()
{
	__super::Free();
}
