#include "ClientPch.h"
#include "TraceurClimbRun.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurClimbRun::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	Register_Flag("Jump");
	Register_Flag("Fall");
	Register_Flag("Land");
	Register_Flag("Mantle");
	
	SetUp_Animations();

	return S_OK;
}

void CTraceurClimbRun::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::CLIMB_RUN);

	_matrix mat = XMMatrixIdentity();
	m_pMeshTransformCom->Set_WorldMatrix(mat); // 회전을 더해줄 Transform;

	if (!Ready_WallRun())
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
		return;
	}

	m_pMeshTransformCom->Set_WorldMatrix(XMMatrixRotationAxis(m_pTransformCom->Get_State(STATE::LOOK), 70.f));


}

void CTraceurClimbRun::OnExit()
{
	// 초기화
	_matrix mat = XMMatrixIdentity();
	m_pMeshTransformCom->Set_WorldMatrix(mat);
	__super::OnExit();
}

_bool CTraceurClimbRun::Ready_WallRun()
{
	m_EnvQueryResult = m_pEnvQueryCom->Get_QueryResult();
	const OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;
	if (!Geo.hasFront)
		return false;

	return true;
}

void CTraceurClimbRun::Check_State()
{
	m_EnvQueryResult = m_pEnvQueryCom->Get_QueryResult();
	_float3 vGroundN{};
	_bool isSupported = m_pColliderCom->IsLand(&vGroundN);
	_bool isLand = isSupported && vGroundN.y >= cosf(XMConvertToRadians(50.f));

	Set_Flag("Jump", m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE)));
	Set_Flag("Land", isLand);
	Set_Flag("Mantle", m_EnvQueryResult.Geometry.isTopReachable && !m_EnvQueryResult.Scan.HeadHit.isHit);
	
}

void CTraceurClimbRun::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	const OBSTACLE_SCAN& Scan = m_EnvQueryResult.Scan;
	ACTORDIR eDir = Scan.ChestHit.isHit
		? CMovementComponent::Calculate_Direction(m_pInputControllerCom)
		: ACTORDIR::END;

	if (Scan.ChestHit.isHit)
	{
		_vector vNormal = XMLoadFloat3(&Scan.ChestHit.vHitNormal);
		_vector vClimbDir = CMovementComponent::Calc_ClimbDir(
			eDir, vNormal, XMVectorSet(0.f, 1.f, 0.f, 0.f));
		
		m_pMoveCom->Move(vClimbDir, fTimeDelta, 2.f);
	}

}

void CTraceurClimbRun::SetUp_Animations()
{
	BLENDSPACE_1D_DESC bs{};
	bs.pParam = m_pMoveCom->Get_LocomotionWeightPtr();
	bs.fBlendIn = 0.2f;
	bs.fBlendOut = 0.2f;
	bs.Samples = { {"StandingIdle01", 0.f}, {"StandingWalkForward", 0.5f}, {"StandingRunForward", 1.f} };

	ROOTMOTION_DESC root{};
	root.fRate = 1.f;

	Add_BlendSpace(ENUM_CLASS(ETraceurClimbRun::Move), bs, root);
}

CTraceurClimbRun* CTraceurClimbRun::Create(CTraceur* pOwner)
{
	CTraceurClimbRun* pInstance = new CTraceurClimbRun();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurClimbRun");
		return nullptr;
	}

	return pInstance;
}

void CTraceurClimbRun::Free()
{
	__super::Free();
}
