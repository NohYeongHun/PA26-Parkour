#include "ClientPch.h"
#include "TraceurGroundVault.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

static constexpr _float FHORIZON_MARGIN = 1.f;
static constexpr _float FHEIGHT_DIST = 0.1f;
static constexpr _uint  ICURVE_DEBUG_SEGMENTS = 20;   // 커브 디버그 렌더 분할 수

HRESULT CTraceurGroundVault::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	SetUp_Animations();
	m_iCurrentAnimIdx = 0;

	return S_OK;
}

void CTraceurGroundVault::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);

	if (!Ready_Enter())
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
		return;
	}
		

	m_pColliderCom->Set_Gravity(false);
	State_Reset();


}

void CTraceurGroundVault::OnUpdate(_float fTimeDelta)
{
	__super::OnUpdate(fTimeDelta);

	// 1. 입력 확인
	Check_State();

	// 2. 애니메이션 업데이트
	Update_Animations(fTimeDelta);
	Move_AlongCurve(fTimeDelta);

#ifdef _DEBUG
	Draw_DebugCurve();
#endif

	// 3. 물리 체크
	Check_Physics(fTimeDelta);

	// 4. 상태 전환
	Check_StateTransition(fTimeDelta);
	
	// 5. 상태 초기화
	State_Reset();
}

void CTraceurGroundVault::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurGroundVault::Check_State()
{
}

void CTraceurGroundVault::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurGroundVault::Check_Physics(_float fTimeDelta)
{
}

void CTraceurGroundVault::Check_StateTransition(_float fTimeDelta)
{
	if (!m_IsAnimationEnd) return;
	m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
}

void CTraceurGroundVault::SetUp_Animations()
{
	CState::Add_ParkourAnimations(ENUM_CLASS(ETraceurGroundVault::LowerVault),
		{ &m_fTrackPosition, "LowVault", 1.2f, 0.05f, 0.f, false }, { 1.f, false, true, true }, {});
}

void CTraceurGroundVault::State_Reset()
{
	
}

_bool CTraceurGroundVault::Ready_Enter()
{
	m_bValidCurve = false;
	m_EnvQueryResult = m_pEnvQueryCom->Get_QueryResult();
	if (!m_EnvQueryResult.isValid)
		return false;
	
	if (!Select_Animation())
		return false;


	Build_Curve();
	return true;
}

// P0 (시작점) -> P1(중간점 => 벽의 최대 높이 + 보정치) -> P2(끝점)
void CTraceurGroundVault::Build_Curve()
{
	const OBSTACLE_GEOMETRY& Geo = m_EnvQueryResult.Geometry;
	if (!Geo.isTopReachable)
		return;

	_float fColliderRadius = m_pColliderCom->Get_Radius();
	_float fObstacleHeight = Geo.fObstacleHeight;
	_float fHorizonDistance = Geo.fFrontDistance;
	_vector vFace = XMVector3Normalize(XMLoadFloat3(&Geo.vTraversalDir)); // 장애물 방향.
	_vector vP0 = m_pTransformCom->Get_State(Engine::STATE::POSITION);

	// P0이 너무 가깝다면? => Radius 길이 + 보정치 보다 길이가 짧다면? 
	if (Geo.fFrontDistance <= (fColliderRadius + FHORIZON_MARGIN))
	{
		_float fOffset = fColliderRadius + FHORIZON_MARGIN - Geo.fFrontDistance;
		vP0 -= vFace * fOffset; // 장애물 반대방향 이동.
		fHorizonDistance = fColliderRadius + FHORIZON_MARGIN;

		m_pTransformCom->Set_State(Engine::STATE::POSITION, vP0);
	}

	_vector vP2 = vP0 + (vFace * fHorizonDistance) * 2 + vFace * Geo.fDepth;
	_vector vP1 = (vP0 + vP2) * 0.5f;
	_float fApexY = XMVectorGetY(vP0) + fObstacleHeight;
	vP1 = XMVectorSetY(vP1, 2.f * fApexY - (XMVectorGetY(vP0) + XMVectorGetY(vP2)) * 0.5f);

	_float3 vGroundPos = {};
	m_pEnvQueryCom->Find_Ground(vP2, fObstacleHeight + FHEIGHT_DIST, 3.f, vGroundPos);

	XMStoreFloat3(&m_vCurveP0, vP0);
	XMStoreFloat3(&m_vCurveP1, vP1);
	m_vCurveP2 = vGroundPos;
	m_bValidCurve = true;
}

#ifdef _DEBUG

void CTraceurGroundVault::Draw_DebugCurve()
{
	if (!m_bValidCurve)
		return;

	CGameInstance* pGI = CGameInstance::GetInstance();

	// 커브 폴리라인 (초록)
	_vector vPrev = QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, 0.f);
	for (_uint i = 1; i <= ICURVE_DEBUG_SEGMENTS; ++i)
	{
		_vector vCur = QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, static_cast<_float>(i) / static_cast<_float>(ICURVE_DEBUG_SEGMENTS));
		pGI->Add_DebugLine(vPrev, vCur, JPH::Color(0.f, 255.f, 0.f, 1.f));
		vPrev = vCur;
	}

	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP0), 0.1f, JPH::Color(0.f, 255.f, 255.f, 1.f));   // 시안: P0 (시작)
	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP1), 0.1f, JPH::Color(255.f, 0.f, 255.f, 1.f));   // 마젠타: P1 (제어점 — 커브가 통과하지 않음)
	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP2), 0.1f, JPH::Color(255.f, 255.f, 255.f, 1.f)); // 흰색: P2 (착지)
}
#endif // _DEBSUG


void CTraceurGroundVault::Move_AlongCurve(_float fTimeDelta)
{
	if (!m_bValidCurve)
		return;
	
	m_fCurveT = min(m_fTrackPosition / m_pModelCom->Get_Duration(m_Animations[m_iCurrentAnimIdx].AnimPlayDesc.strAnimationName), 1.f);
	_float fSmoothT = m_fCurveT * m_fCurveT * (3.0f - 2.0f * m_fCurveT); // Smoothstep 공식
	_vector vPos = XMVectorSetW(QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, fSmoothT), 1.f);
	m_pTransformCom->Set_State(Engine::STATE::POSITION, vPos);
	m_pColliderCom->Set_Position(vPos);
}

_bool CTraceurGroundVault::Select_Animation()
{
	// 1. 현재 m_EnvQueryResult에서 파싱한 지형 정보와
	// 애니메이션 정보를 조합하여 가능한 Animation이 있는지 탐색합니다.
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurGroundVault::LowerVault);
	return true;
}






CTraceurGroundVault* CTraceurGroundVault::Create(CTraceur* pOwner)
{
	CTraceurGroundVault* pInstance = new CTraceurGroundVault();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundVault");
		return nullptr;
	}

	return pInstance;
}


void CTraceurGroundVault::Free()
{
	__super::Free();
}



