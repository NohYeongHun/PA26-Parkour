#include "ClientPch.h"
#include "TraceurGroundLand.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurGroundLand::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurGroundLand::FallingToLanding);

	return S_OK;
}

void CTraceurGroundLand::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurGroundLand::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurGroundLand::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurGroundLand::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurGroundLand::FallingToLanding),
		{ &m_fTrackPosition, "FallingToLanding", 1.f, 0.1f, 0.2f, 0.f, false }, { 1.f, true, true, true });

	CState::Add_Animations(ENUM_CLASS(ETraceurGroundLand::FallALandToStandingIdle),
		{ &m_fTrackPosition, "FallALandToStandingIdle", 1.f, 0.1f, 0.2f, 0.f, false }, { 1.f, true, true, true });
}


CTraceurGroundLand* CTraceurGroundLand::Create(CTraceur* pOwner)
{
	CTraceurGroundLand* pInstance = new CTraceurGroundLand();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundLand");
		return nullptr;
	}

	return pInstance;
}

void CTraceurGroundLand::Free()
{
	__super::Free();
}
