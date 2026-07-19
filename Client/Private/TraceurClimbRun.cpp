#include "ClientPch.h"
#include "TraceurClimbRun.h"
#include "ClimbEvaluator.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "MeshAlignComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurClimbRun::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CTraceurClimbRun::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::CLIMB_RUN);

	if (!Ready_WallRun(pArg))
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
		return;
	}

	const OBSTACLE_SCAN& Scan = Enter_Perception(pArg).Scan;

	// 1. 벽의 Normal을 구합니다.
	_vector vN = XMVector3Normalize(XMLoadFloat3(&Scan.ChestHit.vHitNormal));
	m_pClimbEvalCom->Begin_Climb(vN);

	// 2. 회전용 축을 구합니다.
	_vector vAxisWorld = XMVector3Normalize(XMVector3Cross(-vN, XMVectorSet(0.f, 1.f, 0.f, 0.f)));
	_matrix WorldInv = XMMatrixInverse(nullptr, m_pTransformCom->Get_WorldMatrix());
	_vector vAxisLocal = XMVector3Normalize(XMVector3TransformNormal(vAxisWorld, WorldInv));

	// 3. MeshSpace의 축으로 캐릭터를 회전시킬 Quaternion을 구합니다.
	_vector qTilt = XMQuaternionRotationAxis(vAxisLocal, XMConvertToRadians(FTILT_ANGLE_DEG));

	// 4. Offset을 지정합니다. (발 위치를 보정하기 위해)
	_float3 vOffsetLocal{};
	XMStoreFloat3(&vOffsetLocal,
		XMVector3Normalize(XMVector3TransformNormal(XMVectorNegate(vN), WorldInv)) * m_pColliderCom->Get_Radius());

	m_pMeshAlignCom->Request_Pose(qTilt, vOffsetLocal, FBLEND_IN_TIME);
	m_pMoveCom->Set_ClimbNormal(vN);

#ifdef _DEBUG
	XMStoreFloat3(&m_vDebugWallNormal, vN);
	XMStoreFloat3(&m_vDebugAxisWorld, vAxisWorld);
	XMStoreFloat3(&m_vDebugAxisLocal, vAxisLocal);
	m_hasDebugSteer = true;
#endif
}

void CTraceurClimbRun::OnExit()
{
	m_pMeshAlignCom->Clear_Pose(FBLEND_OUT_TIME);
	__super::OnExit();
	m_pMoveCom->Set_MovementType(MOVEMENT_TYPE::GROUND);
}

_bool CTraceurClimbRun::Ready_WallRun(void* pArg)
{
	const OBSTACLE_GEOMETRY& Geo = Enter_Perception(pArg).Geometry;
	if (!Geo.Front.hasHit)
		return false;

	return true;
}

void CTraceurClimbRun::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	const OBSTACLE_SCAN& Scan = m_pEnvQueryCom->Get_Perception().Scan;
	ACTORDIR eDir = CMovementComponent::Calculate_Direction(m_pInputControllerCom);

	_vector vNormal = XMLoadFloat3(&m_pClimbEvalCom->Get_Eval().vClimbNormal);
	_vector vClimbDir = CMovementComponent::Calc_ClimbDir(
		eDir, vNormal, XMVectorSet(0.f, 1.f, 0.f, 0.f));
	m_pMoveCom->Move(vClimbDir, fTimeDelta, 2.f);

#ifdef _DEBUG
	Draw_DebugSteer();
#endif
}

#ifdef _DEBUG
void CTraceurClimbRun::Draw_DebugSteer()
{
	if (!m_hasDebugSteer)
		return;

	CGameInstance* pGI = CGameInstance::GetInstance();
	_vector vPos = XMVectorSetW(
		m_pTransformCom->Get_State(STATE::POSITION) + XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f);

	pGI->Add_DebugLine(vPos, vPos + XMLoadFloat3(&m_vDebugWallNormal), JPH::Color(255.f, 0.f, 255.f, 1.f));
	pGI->Add_DebugLine(vPos, vPos + XMLoadFloat3(&m_vDebugAxisWorld), JPH::Color(255.f, 255.f, 0.f, 1.f));

	_vector vAxisLocalInWorld = XMVector3Normalize(XMVector3TransformNormal(
		XMLoadFloat3(&m_vDebugAxisLocal), m_pTransformCom->Get_WorldMatrix()));
}
#endif

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
