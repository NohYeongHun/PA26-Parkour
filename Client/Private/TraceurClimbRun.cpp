#include "ClientPch.h"
#include "TraceurClimbRun.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "MeshAlignComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurClimbRun::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	Register_Flag("Jump");
	Register_Flag("Fall");
	Register_Flag("Land");
	Register_Flag("Mantle");
	Register_Flag("Arrive");
	
	SetUp_Animations();

	return S_OK;
}

void CTraceurClimbRun::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::CLIMB_RUN);
	m_hasLeftGround = false;
	m_fGroundedTime = 0.f;
	m_hasTopStandCache = false;

	if (!Ready_WallRun())
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
		return;
	}

	const OBSTACLE_SCAN& Scan = m_EnvQueryResult.Scan;
	m_vClimbNormal = Scan.ChestHit.vHitNormal;

	// 1. 벽의 Normal을 구합니다.
	_vector vN = XMVector3Normalize(XMLoadFloat3(&m_vClimbNormal));

	// 2. 회전용 축을 구합니다.
	_vector vAxisWorld = XMVector3Normalize(XMVector3Cross(-vN, XMVectorSet(0.f, 1.f, 0.f, 0.f)));

	// 3. MeshSpace로 좌표계를 변환합니다.
	_matrix WorldInv = XMMatrixInverse(nullptr, m_pTransformCom->Get_WorldMatrix());
	_vector vAxisLocal = XMVector3Normalize(XMVector3TransformNormal(vAxisWorld, WorldInv));


	// 4. MeshSpace의 축으로 캐릭터를 회전시킬 Quaternion을 구합니다.
	_vector qTilt = XMQuaternionRotationAxis(vAxisLocal, XMConvertToRadians(FTILT_ANGLE_DEG)); // 부호는 플레이 검증으로 확정된 값

	// 5. Offset을 지정합니다. (발 위치를 보정하기 위해)
	_float3 vOffsetLocal{};
	XMStoreFloat3(&vOffsetLocal,
		XMVector3Normalize(XMVector3TransformNormal(XMVectorNegate(vN), WorldInv)) * m_pColliderCom->Get_Radius());

	m_pMeshAlignCom->Request_Pose(qTilt, vOffsetLocal, FBLEND_IN_TIME);

}

void CTraceurClimbRun::OnExit()
{
	m_pMeshAlignCom->Clear_Pose(FBLEND_OUT_TIME);
	__super::OnExit();
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::GROUND);
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

	if (!isLand)
		m_hasLeftGround = true; 

	Set_Flag("Jump", m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::SPACE)));
	Set_Flag("Fall", m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::S)));
	Set_Flag("Land", m_hasLeftGround && isLand);
	Set_Flag("Mantle", m_EnvQueryResult.Geometry.isTopReachable && !m_EnvQueryResult.Scan.HeadHit.isHit);

	if (m_EnvQueryResult.Geometry.isTopReachable)
	{
		m_vTopStandCache = m_EnvQueryResult.Geometry.vTopStandPos;
		m_hasTopStandCache = true;
	}
	


}

// Look 제한 걸어야함.
void CTraceurClimbRun::Update_Animations(_float fTimeDelta)
{
	// 이미 벽 위이고, 상단 레이가 착지점을 찾았고, 현재 y값이 착지점y값보다 크거나 같다면?
	if (m_hasLeftGround && m_hasTopStandCache
		&& XMVectorGetY(m_pTransformCom->Get_State(STATE::POSITION)) >= m_vTopStandCache.y)
	{
		Set_Flag("Arrive", true);
		return;
	}

	CTraceurState::Play_Animation(fTimeDelta);

	const OBSTACLE_SCAN& Scan = m_EnvQueryResult.Scan;
	ACTORDIR eDir = CMovementComponent::Calculate_Direction(m_pInputControllerCom);

	// 한번 들어가면 
	_vector vNormal = XMLoadFloat3(&m_vClimbNormal);
	_vector vClimbDir = CMovementComponent::Calc_ClimbDir(
		eDir, vNormal, XMVectorSet(0.f, 1.f, 0.f, 0.f));
	m_pMoveCom->Move(vClimbDir, fTimeDelta, 2.f);
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
