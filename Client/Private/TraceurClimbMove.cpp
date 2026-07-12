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
	Set_Flag("Jump", m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE)));

	const auto& Geo = m_pEnvQueryCom->Get_QueryResult().Geometry;
	Set_Flag("KneeHit", Geo.KneeHit.isHit);
	Set_Flag("Land", m_pColliderCom->IsLand());
	Set_Flag("Fall", !Get_Flag("Land"));
}

void CTraceurClimbMove::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	const auto& Geo = m_pEnvQueryCom->Get_QueryResult().Geometry;

	ACTORDIR eDir = Geo.ChestHit.isHit
		? CMovementComponent::Calculate_Direction(m_pInputControllerCom)
		: ACTORDIR::END;

	if (Geo.ChestHit.isHit)
	{
		_vector vNormal   = XMLoadFloat3(&Geo.ChestHit.vHitNormal);
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
	bs.fBlendDuration = 0.2f;
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
