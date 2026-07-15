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

	Register_Flag("Jump");
	Register_Flag("Land");
	Register_Flag("Fall");
	Register_Flag("KneeHit");
	Register_Flag("InputDown");
	Register_Flag("Mantle");

	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurClimbMove::Move);

	return S_OK;
}

void CTraceurClimbMove::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurClimbMove::Move);
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::CLIMB);
	Clear_Flags();
}

void CTraceurClimbMove::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(false);
}

void CTraceurClimbMove::Check_State()
{
	m_EnvQueryResult = m_pEnvQueryCom->Get_QueryResult();
#ifdef _DEBUG
	//m_pEnvQueryCom->Print_Debug();
#endif // _DEBUG

	PARKOUR_ACTION eAction = m_EnvQueryResult.Decision.eBestAction;
	Set_Flag("InputDown", m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::S)));
	Set_Flag("Jump", m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE)));

	Set_Flag("KneeHit", m_EnvQueryResult.Scan.KneeHit.isHit);
	Set_Flag("Land", m_pColliderCom->IsLand());
	Set_Flag("Fall", !Get_Flag("Land"));
	Set_Flag("Mantle", m_EnvQueryResult.Geometry.isTopReachable && !m_EnvQueryResult.Scan.HeadHit.isHit);
	
	
}

void CTraceurClimbMove::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	const OBSTACLE_SCAN& Scan = m_EnvQueryResult.Scan;

	ACTORDIR eDir = Scan.ChestHit.isHit
		? CMovementComponent::Calculate_Direction(m_pInputControllerCom)
		: ACTORDIR::END;

	if (Scan.ChestHit.isHit)
	{
		_vector vNormal   = XMLoadFloat3(&Scan.ChestHit.vHitNormal);
		_vector vClimbDir = CMovementComponent::Calc_ClimbDir(
			eDir, vNormal, XMVectorSet(0.f, 1.f, 0.f, 0.f));
		m_pMoveCom->Move(vClimbDir, fTimeDelta, 1.f);
	}

	m_pMoveCom->Update_ClimbBlendWeight(eDir, fTimeDelta);
}

void CTraceurClimbMove::SetUp_Animations()
{
	BLENDSPACE_2D_DESC bs{};
	bs.pParam         = m_pMoveCom->Get_LocomotionWeight2DPtr();
	bs.fBlendIn = 0.2f;
	bs.fBlendOut = 0.2f;
	bs.Samples        = {
		{"HangingIdle",            0.f,  0.f},
		{"LeftBracedHangShimmy",  -1.f,  0.f},
		{"RightBracedHangShimmy",  1.f,  0.f},
		{"ClimbingUpWall",         0.f,  1.f},
		{"ClimbingDownWall",       0.f, -1.f}
	};

	ROOTMOTION_DESC root{};
	root.fRate = 1.f;
	CState::Add_BlendSpace(ENUM_CLASS(ETraceurClimbMove::Move), bs, root);
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
