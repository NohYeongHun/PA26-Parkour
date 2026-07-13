#include "ClientPch.h"
#include "TraceurGroundMove.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "Collider.h"
#include "Model.h"

HRESULT CTraceurGroundMove::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	Register_Flag("Move");
	Register_Flag("Run");
	Register_Flag("Vault");
	Register_Flag("Land");
	Register_Flag("Climb");
	Register_Flag("Jump");
	Register_Flag("Fall");

	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurGroundMove::Move);

	return S_OK;
}

void CTraceurGroundMove::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);
	Clear_Flags();
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::GROUND);
	m_fFallTime = 0.f;
}

void CTraceurGroundMove::OnExit()
{
	__super::OnExit();
	m_fFallTime = 0.f;
}

void CTraceurGroundMove::Check_State()
{
	Set_Flag("Move", m_pInputControllerCom->Check_AnyInput(m_iMoveKey));
	Set_Flag("Run",  Get_Flag("Move") && m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT)));
	Set_Flag("Land", m_pColliderCom->IsLand());
	Set_Flag("Jump", m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE)));
}

void CTraceurGroundMove::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	_float fTargetWeight = 0.f;
	if (Get_Flag("Run"))
		fTargetWeight = 1.f;
	else if (Get_Flag("Move"))
		fTargetWeight = 0.5f;

	_vector vWorldDir = CMovementComponent::Calc_GroundDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());

	m_pMoveCom->Move(vWorldDir, fTimeDelta, fTargetWeight);
}

void CTraceurGroundMove::Check_Physics(_float fTimeDelta)
{
	const _bool isLand = m_pColliderCom->IsLand();
	Set_Flag("Land", isLand);
	if (isLand)
	{
		m_fFallTime = 0.f;
	}
	else
	{
		m_fFallTime += fTimeDelta;
		if (m_fFallTime >= 0.3f)
			Set_Flag("Fall", true);
	}

	const ENV_QUERY_RESULT& EnvResult = m_pEnvQueryCom->Get_QueryResult();
	if (!Get_Flag("Move") || !Get_Flag("Run"))
		return;

	if (EnvResult.Decision.isValid)
	{
		if (EnvResult.Decision.eBestAction == PARKOUR_ACTION::LOW_VAULT)
		{
			Set_Flag("Vault", true);
			return;
		}

		if (EnvResult.Decision.eBestAction == PARKOUR_ACTION::CLIMB)
		{
			Set_Flag("Climb", true);
			return;
		}
	}
}

void CTraceurGroundMove::SetUp_Animations()
{
	BLENDSPACE_1D_DESC bs{};
	bs.pParam         = m_pMoveCom->Get_LocomotionWeightPtr();
	bs.fBlendIn = 0.2f;
	bs.fBlendOut = 0.2f;
	bs.Samples        = { {"StandingIdle01", 0.f}, {"StandingWalkForward", 0.5f}, {"StandingRunForward", 1.f} };

	ROOTMOTION_DESC root{};
	root.fRate = 1.f;

	Add_BlendSpace(ENUM_CLASS(ETraceurGroundMove::Move), bs, root);
}

CTraceurGroundMove* CTraceurGroundMove::Create(CTraceur* pOwner)
{
	CTraceurGroundMove* pInstance = new CTraceurGroundMove();
	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundMove");
		return nullptr;
	}
	return pInstance;
}

void CTraceurGroundMove::Free()
{
	__super::Free();
}
