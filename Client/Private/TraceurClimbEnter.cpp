#include "ClientPch.h"
#include "TraceurClimbEnter.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"


HRESULT CTraceurClimbEnter::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurClimbEnter::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);

	if (!Ready_Enter())
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
		return;
	}

	State_Reset();
}

void CTraceurClimbEnter::OnUpdate(_float fTimeDelta)
{
	__super::OnUpdate(fTimeDelta);

	// 1. 입력 확인
	Check_State();

	// 2. 애니메이션 업데이트
	Update_Animations(fTimeDelta);
	Move_AlongCurve(fTimeDelta);

#ifdef _DEBUG
	Draw_DebugCurve();
#endif // _DEBUG

	// 3. 물리 체크
	Check_Physics(fTimeDelta);

	// 4. 상태 전환
	Check_StateTransition(fTimeDelta);
	
	// 5. 상태 초기화
	State_Reset();
}

void CTraceurClimbEnter::OnExit()
{
	__super::OnExit();
	//m_pColliderCom->Set_Gravity(true);
}

void CTraceurClimbEnter::Check_State()
{
}

void CTraceurClimbEnter::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta * 1.5f);
}

void CTraceurClimbEnter::Check_Physics(_float fTimeDelta)
{
}

void CTraceurClimbEnter::Check_StateTransition(_float fTimeDelta)
{
	if (m_IsAnimationEnd)
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::CLIMB),
			ENUM_CLASS(ETraceurClimbState::Move));

		return;
	}
}



void CTraceurClimbEnter::State_Reset()
{
	
}

_bool CTraceurClimbEnter::Ready_Enter()
{
	m_EnvQueryResult = m_pEnvQueryCom->Get_QueryResult();
	if (!m_EnvQueryResult.isValid || !m_EnvQueryResult.Geometry.HeadHit.isHit)
		return false;

	if (!Select_Animation())
		return false;

	Build_Curve();
	

	return true;
}

_bool CTraceurClimbEnter::Select_Animation()
{
	// 임시
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurClimbEnter::IdleToBracedHang);
	return true;
}

void CTraceurClimbEnter::Build_Curve()
{
	const OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;
	_float fColliderRadius = m_pColliderCom->Get_Radius();

	_vector vP0 = m_pTransformCom->Get_State(Engine::STATE::POSITION);

	// SnapPoint
	_vector vWallNormal = XMVector3Normalize(XMLoadFloat3(&Geo.HeadHit.vHitNormal));
	_vector vP2 = XMVectorSetW(XMLoadFloat3(&Geo.HeadHit.vHitPosition), 1.f);
	vP2 = XMVectorSetY(vP2 + (vWallNormal * (fColliderRadius + 0.1f)), XMVectorGetY(vP0));
	
	_vector vP1 = (vP0 + vP2) * 0.5f;
	_float fApexY = XMVectorGetY(vP0) + 0.3f;
	vP1 = XMVectorSetY(vP1, fApexY);

	
	XMStoreFloat3(&m_vCurveP0, vP0);
	XMStoreFloat3(&m_vCurveP1, vP1);
	XMStoreFloat3(&m_vCurveP2, vP2);


	XMStoreFloat3(&m_vLookTarget, -vWallNormal);
	XMStoreFloat3(&m_vLookStart, m_pTransformCom->Get_State(Engine::STATE::LOOK));
	m_isValidCurve = true;


#ifdef _DEBUG
	XMStoreFloat3(&m_vDebugWallNormal, vWallNormal);
	m_vDebugWallHitPos = Geo.HeadHit.vHitPosition;
	XMStoreFloat3(&m_vDebugWallEndPos, XMLoadFloat3(&m_vDebugWallHitPos) + vWallNormal);
#endif // _DEBUG



}

#ifdef _DEBUG
void CTraceurClimbEnter::Draw_DebugCurve()
{
	if (!m_isValidCurve)
		return;

	CGameInstance* pGI = CGameInstance::GetInstance();

	// 1.Wall Noraml
	pGI->Add_DebugLine(XMVectorSetW(XMLoadFloat3(&m_vDebugWallHitPos), 1.f), XMVectorSetW(XMLoadFloat3(&m_vDebugWallEndPos), 1.f), JPH::Color(255.f, 0.f, 255.f, 1.f));

	// 2. 커브 라인
	_vector vPrev = QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, 0.f);
	for (_uint i = 1; i <= 20; ++i)
	{
		_vector vCur = QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, static_cast<_float>(i) / static_cast<_float>(20));
		pGI->Add_DebugLine(vPrev, vCur, JPH::Color(0.f, 255.f, 0.f, 1.f));
		vPrev = vCur;
	}

	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP0), 0.1f, JPH::Color(0.f, 255.f, 255.f, 1.f));   // 시안: P0 (시작)
	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP1), 0.1f, JPH::Color(255.f, 0.f, 255.f, 1.f));   // 마젠타: P1 (제어점 — 커브가 통과하지 않음)
	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP2), 0.1f, JPH::Color(255.f, 255.f, 255.f, 1.f)); // 흰색: P2 (착지)
}

#endif // _DEBUG

void CTraceurClimbEnter::Move_AlongCurve(_float fTimeDelta)
{
	if (!m_isValidCurve)
		return;

	m_fCurveT = min(m_fTrackPosition / m_pModelCom->Get_Duration(m_Animations[m_iCurrentAnimIdx].AnimPlayDesc.strAnimationName), 1.f);
	_float fSmoothT = m_fCurveT * m_fCurveT * (3.0f - 2.0f * m_fCurveT); // Smoothstep 공식
	_vector vPos = XMVectorSetW(QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, fSmoothT), 1.f);
	m_pTransformCom->Set_State(Engine::STATE::POSITION, vPos);
	// LookLerp
	_vector vLookLerp = XMVectorLerp(XMVector3Normalize(XMLoadFloat3(&m_vLookStart))
		, XMVector3Normalize(XMLoadFloat3(&m_vLookTarget)), fSmoothT);

	m_pTransformCom->LookDir(vLookLerp);
	
	m_pColliderCom->Set_Position(vPos);
}


void CTraceurClimbEnter::SetUp_Animations()
{
	CState::Add_ParkourAnimations(ENUM_CLASS(ETraceurClimbEnter::IdleToBracedHang),
		{ &m_fTrackPosition, "IdleToBracedHang", 1.f, 0.2f, 0.f, false }, { 1.f, false, true, true }, {});
}


CTraceurClimbEnter* CTraceurClimbEnter::Create(CTraceur* pOwner)
{
	CTraceurClimbEnter* pInstance = new CTraceurClimbEnter();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurClimbEnter");
		return nullptr;
	}

	return pInstance;
}


void CTraceurClimbEnter::Free()
{
	__super::Free();
}



