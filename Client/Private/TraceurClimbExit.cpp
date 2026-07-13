#include "ClientPch.h"
#include "TraceurClimbExit.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurClimbExit::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	Register_Flag("Land");
	Register_Flag("Mantle");

	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurClimbExit::Climbing);

	return S_OK;
}

void CTraceurClimbExit::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);
	m_EnvQueryResult = m_pEnvQueryCom->Get_QueryResult();
	// Mantle -> Land -> Move
	Set_Flag("Mantle", true);
	Set_Flag("Land", m_pColliderCom->IsLand());
	if (Get_Flag("Mantle"))
	{
		Build_Curve();
	};
	
}

void CTraceurClimbExit::OnExit()
{
	__super::OnExit();
}

void CTraceurClimbExit::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurClimbExit::Late_Anim_Update(_float fTimeDelta)
{
	Move_AlongCurve(fTimeDelta);
#ifdef _DEBUG
	Draw_DebugCurve();
#endif // _DEBUG

}

void CTraceurClimbExit::Check_State()
{
	Set_Flag("Land", m_pColliderCom->IsLand());

#ifdef _DEBUG

#endif // _DEBUG

}

void CTraceurClimbExit::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurClimbExit::Climbing),
		{ &m_fTrackPosition, "Climbing", 1.f, 0.05f, 0.2f, 0.f, false }, { 1.f, false, true, true });

	CState::Add_Animations(ENUM_CLASS(ETraceurClimbExit::JumpFromWall),
		{ &m_fTrackPosition, "JumpFromWall", 1.f, 0.05f, 0.2f, 0.f, false }, { 1.f, true, true, true });

	
}

#ifdef _DEBUG
void CTraceurClimbExit::Draw_DebugCurve()
{
	if (!m_bValidCurve)
		return;

	CGameInstance* pGI = CGameInstance::GetInstance();

	_vector vPrev = QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, 0.f);
	for (_uint i = 1; i <= 20; ++i)
	{
		_vector vCur = QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2,
			static_cast<_float>(i) / static_cast<_float>(20));
		pGI->Add_DebugLine(vPrev, vCur, JPH::Color(0.f, 255.f, 0.f, 1.f));
		vPrev = vCur;
	}

	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP0), 0.1f, JPH::Color(0.f, 255.f, 255.f, 1.f));
	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP1), 0.1f, JPH::Color(255.f, 0.f, 255.f, 1.f));
	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP2), 0.1f, JPH::Color(255.f, 255.f, 255.f, 1.f));
}
#endif // _DEBUG



void CTraceurClimbExit::Move_AlongCurve(_float fTimeDelta)
{
	if (!m_bValidCurve)
		return;

	m_fCurveT = min(m_fTrackPosition / m_pModelCom->Get_Duration(
		m_Animations[m_iCurrentAnimIdx].AnimPlayDesc.strAnimationName), 1.f);
	/*_float  fSmoothT = m_fCurveT * m_fCurveT * (3.0f - 2.0f * m_fCurveT);
	_vector vPos = XMVectorSetW(QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, fSmoothT), 1.f);*/
	_vector vPos = XMVectorSetW(QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, m_fCurveT), 1.f);
	m_pTransformCom->Set_State(Engine::STATE::POSITION, vPos);
	//m_pColliderCom->Set_Position(vPos); // 강제 이동.
}

void CTraceurClimbExit::Build_Curve()
{
	// 1. 상단에 닿을 수 없다면?
	const OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;
	if (!Geo.isTopReachable)
		return;

	// 2. 

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
