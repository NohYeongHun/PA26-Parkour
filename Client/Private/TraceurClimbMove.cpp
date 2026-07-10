#include "ClientPch.h"
#include "TraceurClimbMove.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurClimbMove::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurClimbMove::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurClimbMove::Move);
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::CLIMB);
	State_Reset();
}

void CTraceurClimbMove::OnUpdate(_float fTimeDelta)
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

void CTraceurClimbMove::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(false);
}

void CTraceurClimbMove::Check_State()
{
	// 임시
	//m_States[IDLE] = m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
	m_States[JUMP] = m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE));
}

void CTraceurClimbMove::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	const auto& Geo = m_pEnvQueryCom->Get_QueryResult().Geometry;

	// 벽에서 떨어지면(가슴 레이 miss) 입력이 있어도 END로 취급 -> 가중치가 Idle로 감쇠
	ACTORDIR eDir = Geo.ChestHit.isHit
		? CMovementComponent::Calculate_Direction(m_pInputControllerCom)
		: ACTORDIR::END;

	if (Geo.ChestHit.isHit)
	{
		_vector vNormal = XMLoadFloat3(& Geo.ChestHit.vHitNormal);
		_vector vClimbDir = CMovementComponent::Calc_ClimbDir(
			eDir,
			vNormal,
			XMVectorSet(0.f, 1.f, 0.f, 0.f)
		);
		m_pMoveCom->Move(vClimbDir, fTimeDelta, 1.f);
	}

	// 가중치 갱신은 ChestHit 여부와 무관하게 항상 실행 (그래야 벽 끝에서 얼어붙지 않고 Idle로 빠져나옴)
	m_pMoveCom->Update_ClimbBlendWeight(eDir, fTimeDelta);
}

void CTraceurClimbMove::Check_Physics(_float fTimeDelta)
{
}

void CTraceurClimbMove::Check_StateTransition(_float fTimeDelta)
{
	// 임시.
	/*if (m_States[IDLE])
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND),
			ENUM_CLASS(ETraceurGroundMove::Move));
	}*/
	if (m_States[JUMP])
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::AIR),
			ENUM_CLASS(ETraceurAirState::Fall));

		return;
	}
}

void CTraceurClimbMove::SetUp_Animations()
{
	BLENDSPACE_2D_DESC bs{};
	bs.pParam = m_pMoveCom->Get_LocomotionWeight2DPtr();
	bs.fBlendDuration = 0.2f;
	bs.Samples = { {"HangingIdle", 0.f, 0.f},
		{"LeftBracedHangShimmy", -1.f, 0.f},
		{"RightBracedHangShimmy", 1.f, 0.f},
		{"ClimbingUpWall", 0.f, 1.f},
		{"ClimbingDownWall", 0.f, -1.f}
	};

	ROOTMOTION_DESC root{};
	root.fRate = 1.f;
	CState::Add_BlendSpace(ENUM_CLASS(ETraceurClimbMove::Move), bs, root);

}

void CTraceurClimbMove::State_Reset()
{
	for (uint i = 0; i < STATE::END; ++i)
		m_States[i] = false;
}


CTraceurClimbMove* CTraceurClimbMove::Create(CTraceur* pOwner)
{
	CTraceurClimbMove* pInstance = new CTraceurClimbMove();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurClimbMove");
		return nullptr;
	}

	return pInstance;
}


void CTraceurClimbMove::Free()
{
	__super::Free();
}



