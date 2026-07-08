#include "ClientPch.h"
#include "TraceurGroundVault.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "Transform.h"

HRESULT CTraceurGroundVault::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurGroundVault::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);

	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurGroundVault::LowerVault);

	m_pColliderCom->Set_Gravity(false);
	State_Reset();
}

void CTraceurGroundVault::OnUpdate(_float fTimeDelta)
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

void CTraceurGroundVault::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurGroundVault::Check_State()
{
}

void CTraceurGroundVault::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurGroundVault::Check_Physics(_float fTimeDelta)
{
}

void CTraceurGroundVault::Check_StateTransition(_float fTimeDelta)
{
	if (!m_IsAnimationEnd) return;
	m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
}

void CTraceurGroundVault::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurGroundVault::LowerVault),
		{&m_fTrackPosition, "LowVault", 1.f, 0.05f, 0.f, false}, { 1.f, true, true, true});
}

void CTraceurGroundVault::State_Reset()
{
	
}



void CTraceurGroundVault::Align_ToObstacle()
{
}

CTraceurGroundVault* CTraceurGroundVault::Create(CTraceur* pOwner)
{
	CTraceurGroundVault* pInstance = new CTraceurGroundVault();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundVault");
		return nullptr;
	}

	return pInstance;
}


void CTraceurGroundVault::Free()
{
	__super::Free();
}



