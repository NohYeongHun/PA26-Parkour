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

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurAirFall::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);
	State_Reset();


}

void CTraceurAirFall::OnUpdate(_float fTimeDelta)
{
	__super::OnUpdate(fTimeDelta);

	// 1. 입력 확인
	Check_State();

	// 2. 애니메이션 업데이트
	Update_Animations(fTimeDelta);

	// 3. 물리 체크
	Check_Physics(fTimeDelta);

	// 4. 상태 전환
	Check_StateTransition(fTimeDelta);
	
	// 5. 상태 초기화
	State_Reset();
}

void CTraceurAirFall::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurAirFall::Check_State()
{
}

void CTraceurAirFall::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurAirFall::Check_Physics(_float fTimeDelta)
{
}

void CTraceurAirFall::Check_StateTransition(_float fTimeDelta)
{
	if (!m_IsAnimationEnd) 
		return;
}

void CTraceurAirFall::SetUp_Animations()
{
	/*CState::Add_ParkourAnimations(ENUM_CLASS(ETraceurGroundVault::LowerVault),
		{ &m_fTrackPosition, "LowVault", 1.f, 0.05f, 0.f, false }, { 1.f, false, true, true }, {});*/
}

void CTraceurAirFall::State_Reset()
{
	
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



