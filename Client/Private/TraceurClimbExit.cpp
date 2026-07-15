#include "ClientPch.h"
#include "TraceurClimbExit.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "MotionWarpingComponent.h"

HRESULT CTraceurClimbExit::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	Register_Flag("Land");
	Register_Flag("Mantle");
	Register_Flag("Fall");
	Register_Flag("WallDrop");
	Register_Flag("HangDropEnd");
	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurClimbExit::Climbing);

	return S_OK;
}

void CTraceurClimbExit::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);
	m_EnvQueryResult = m_pEnvQueryCom->Get_QueryResult();

	ETraceurClimbExit eType = static_cast<ETraceurClimbExit>(m_iCurrentAnimIdx);
	Set_Flag("Mantle", eType == ETraceurClimbExit::ClimbingToTop || eType == ETraceurClimbExit::Climbing
		|| eType == ETraceurClimbExit::BracedHangToCrouch);
	Set_Flag("WallDrop", eType == ETraceurClimbExit::BracedHangDrop);
	
	if (Get_Flag("Mantle"))
	{
		_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

		const OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;
		m_pMotionWarpCom->Clear_WarpTargets();
#ifdef _DEBUG
		m_pMotionWarpCom->Reset_DebugTrail(); 
#endif

		if (Geo.isTopReachable)
		{
			_vector vFacing = -XMVectorSetY(XMLoadFloat3(&Geo.vFrontNormal), 0.f);
			if (XMVectorGetX(XMVector3LengthSq(vFacing)) < 1e-4f)
				vFacing = vLook;

			vFacing = XMVector3Normalize(vFacing);

			_float fYaw = atan2f(XMVectorGetX(vFacing), XMVectorGetZ(vFacing));
			_vector vQuat = XMQuaternionRotationRollPitchYaw(0.f, fYaw, 0.f);
			_float4 qRot{};
			XMStoreFloat4(&qRot, vQuat);


			m_pMotionWarpCom->Set_WarpTarget("VaultLedge", Geo.vTopEdgePos, qRot);
			m_pMotionWarpCom->Set_WarpTarget("VaultStand", Geo.vTopStandPos);
			//m_pMotionWarpCom->Set_WarpTarget("VaultStand", Geo.vTopStandPos);

#ifdef _DEBUG
			cout << "현재 Edge, Stand Y: " << Geo.vTopEdgePos.y << ", " << Geo.vTopStandPos.y << endl;
			m_pModelCom->Dump_RootMotionCurve(m_Animations[m_iCurrentAnimIdx].AnimPlayDesc.strAnimationName);
#endif // _DEBUG
		}
			
		if (Geo.hasLandingSpace)
			m_pMotionWarpCom->Set_WarpTarget("VaultLand", Geo.vLandingPos);
		m_isMantle = true;
		m_pColliderCom->Set_Gravity(false);
	};
}

void CTraceurClimbExit::OnExit()
{
	__super::OnExit();
	m_pMotionWarpCom->Abort_Warp();
}

void CTraceurClimbExit::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
	m_pColliderCom->Set_Position(m_pTransformCom->Get_State(STATE::POSITION));

}

void CTraceurClimbExit::Late_Anim_Update(_float fTimeDelta)
{
	if (m_isMantle)
	{
#ifdef _DEBUG
		Draw_DebugCurve();
		
#endif // _DEBUG
	}
}

void CTraceurClimbExit::Check_State()
{
	_float3 vGroundN{};
	_bool isSupported = m_pColliderCom->IsLand(&vGroundN);
	_bool isLand = isSupported && vGroundN.y >= cosf(XMConvertToRadians(50.f));

	Set_Flag("Land", isLand);
	Set_Flag("Fall", !isLand);
	ETraceurClimbExit eType = static_cast<ETraceurClimbExit>(m_iCurrentAnimIdx);
	Set_Flag("Mantle", m_isMantle);
	
	if (Get_Flag("HangDropEnd"))
	{
		m_pColliderCom->Set_Gravity(true);
	}
	

#ifdef _DEBUG
	//Debug_PrintFlag();

	if (Get_Flag("Land"))
	{
		cout << "Collider is Land" << endl;
	}
#endif // _DEBUG
}

void CTraceurClimbExit::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurClimbExit::JumpFromWall),
		{ &m_fTrackPosition, "JumpFromWall", 1.f, 0.05f, 0.2f, 0.f, false }, { 1.f, true, true, true });

	CState::Add_Animations(ENUM_CLASS(ETraceurClimbExit::Climbing),
		{ &m_fTrackPosition, "Climbing", 1.f, 0.05f, 0.2f, 0.f, false }, { 1.f, true, true, true });

	CState::Add_Animations(ENUM_CLASS(ETraceurClimbExit::ClimbingToTop),
		{ &m_fTrackPosition, "ClimbingToTop", 1.f, 0.05f, 0.2f, 0.f, false }, { 1.f, true, true, true });
	
	CState::Add_Animations(ENUM_CLASS(ETraceurClimbExit::BracedHangToCrouch),
		{ &m_fTrackPosition, "BracedHangToCrouch", 1.f, 0.05f, 0.2f, 0.f, false }, { 1.f, true, true, true });

	CState::Add_Animations(ENUM_CLASS(ETraceurClimbExit::BracedHangDrop),
		{ &m_fTrackPosition, "BracedHangDrop", 1.f, 0.05f, 0.2f, 0.f, false }, { 1.f, true, true, true });
	
}

#ifdef _DEBUG
void CTraceurClimbExit::Draw_DebugCurve()
{
	const OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;
	CGameInstance* pGI = CGameInstance::GetInstance();

	pGI->Add_DebugSphere(XMLoadFloat3(&Geo.vTopEdgePos), 0.3f, JPH::Color(255.f, 255.f, 255.f, 1.f));
	pGI->Add_DebugSphere(XMLoadFloat3(&Geo.vTopStandPos), 0.3f, JPH::Color(0.f, 255.f, 255.f, 1.f));
	//pGI->Add_DebugSphere(XMLoadFloat3(&Geo.vLandingPos), 0.3f, JPH::Color(255.f, 255.f, 255.f, 1.f));
	return;

	if (!m_bValidCurve)
		return;
}
#endif // _DEBUG



void CTraceurClimbExit::Move_AlongCurve(_float fTimeDelta)
{
	if (!m_bValidCurve)
		return;

	m_fCurveT = min(m_fTrackPosition / m_pModelCom->Get_Duration(
		m_Animations[m_iCurrentAnimIdx].AnimPlayDesc.strAnimationName), 1.f);
	_vector vPos = XMVectorSetW(QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, m_fCurveT), 1.f);
	m_pTransformCom->Set_State(Engine::STATE::POSITION, vPos);
}

void CTraceurClimbExit::Build_Curve()
{
	// 1. 상단에 닿을 수 없다면?
	const OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;
	if (!Geo.isTopReachable)
		return;

	_float fColliderRadius = m_pColliderCom->Get_Radius();
	_float fObstacleHeight = Geo.fObstacleHeight;
	_float fHorizonDistance = Geo.fFrontDistance;
	_vector vFace = XMVector3Normalize(XMLoadFloat3(&Geo.vTraversalDir)); // 접촉면 방향 벡터
	_vector vP0 = m_pTransformCom->Get_State(Engine::STATE::POSITION); 

	_vector vP2 = vP0 + (vFace * (m_pColliderCom->Get_Radius() + 0.2f));
	_vector vP1 = (vP0 + vP2) * 0.5f;
	_float  fApexY = XMVectorGetY(vP0);
	

	_float3 vGroundPos = {};
	m_pEnvQueryCom->Find_Ground(vP2, fObstacleHeight, 3.f, vGroundPos);

	vP1 = XMVectorSetY(vP1, vGroundPos.y + m_pColliderCom->Get_Radius());

	XMStoreFloat3(&m_vCurveP0, vP0);
	XMStoreFloat3(&m_vCurveP1, vP1);
	m_vCurveP2 = vGroundPos;
	m_bValidCurve = true;
}

void CTraceurClimbExit::End_Traversal()
{
	const OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;
	_vector vStandPos = XMVectorSetW(XMLoadFloat3(&Geo.vTopStandPos), 1.f);
	m_pColliderCom->Set_Position(vStandPos);
	m_pTransformCom->Set_State(STATE::POSITION, vStandPos);
	m_pTransformCom->Save_PreviousPosition();
	m_pColliderCom->Set_Gravity(true);
	m_isMantle = false;
	m_isWarpBegun = false;
}

CTraceurClimbExit* CTraceurClimbExit::Create(CTraceur* pOwner)
{
	CTraceurClimbExit* pInstance = new CTraceurClimbExit();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurClimbExit");
		return nullptr;
	}

	return pInstance;
}

void CTraceurClimbExit::Free()
{
	__super::Free();
}
