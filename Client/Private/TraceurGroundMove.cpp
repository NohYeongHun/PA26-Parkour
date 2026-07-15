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
	Register_Flag("Front");
	Register_Flag("WallRun");

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
	m_fWallRunCooldown = FWALLRUN_COOLDOWN;
}

void CTraceurGroundMove::OnExit()
{
	__super::OnExit();
	m_fFallTime = 0.f;
}

void CTraceurGroundMove::Check_State()
{
	_float3 vGround;
	_bool isSupported = m_pColliderCom->IsLand(&vGround);
	_bool isLand = isSupported && vGround.y >= cosf(XMConvertToRadians(50.f));
	Set_Flag("Move", m_pInputControllerCom->Check_AnyInput(m_iMoveKey));
	Set_Flag("Run",  Get_Flag("Move") && m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT)));
	Set_Flag("Land", isLand);
	Set_Flag("Jump", m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE)));
	Set_Flag("Front", m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::W)));

	const ENV_QUERY_RESULT& EnvResult = m_pEnvQueryCom->Get_QueryResult();
	if (EnvResult.Decision.isValid)
	{
		if (EnvResult.Decision.eBestAction == PARKOUR_ACTION::LOW_VAULT && Get_Flag("Run") && Get_Flag("Land"))
		{
			Set_Flag("Vault", true);
			return;
		}

		if (EnvResult.Decision.Verdicts[ENUM_CLASS(PARKOUR_ACTION::WALL_RUN)].isPossible && Get_Flag("Run")
			&& m_fWallRunCooldown <= 0.f)
		{
			Set_Flag("WallRun", true);
			return;
		}

		if (EnvResult.Decision.eBestAction == PARKOUR_ACTION::CLIMB && Get_Flag("Front") && !Get_Flag("Run"))
		{
			Set_Flag("Climb", true);
			return;
		}
	}
}

void CTraceurGroundMove::Update_Animations(_float fTimeDelta)
{
	m_fWallRunCooldown = max(m_fWallRunCooldown - fTimeDelta, 0.f);

	CTraceurState::Play_Animation(fTimeDelta);

	_float fTargetWeight = 0.f;
	if (Get_Flag("Run"))
		fTargetWeight = 1.5f;
	else if (Get_Flag("Move"))
		fTargetWeight = 1.f;

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
