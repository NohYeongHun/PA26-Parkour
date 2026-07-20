#include "ClientPch.h"
#include "TraceurGroundMantle.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "MotionWarpingComponent.h"
#include "ParkourDeciderComponent.h"

HRESULT CTraceurGroundMantle::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CTraceurGroundMantle::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);
	if (!Ready_Enter(pArg))
	{
		m_pStateMachinCom->Change_State(ENUM_CLASS(EStateCategory::GROUND), ENUM_CLASS(ETraceurGroundState::Move));
		return;
	}
}

void CTraceurGroundMantle::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
	m_pMotionWarpCom->Abort_Warp();
}

void CTraceurGroundMantle::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

void CTraceurGroundMantle::Late_Anim_Update(_float fTimeDelta)
{
#ifdef _DEBUG
	Draw_Debug();
#endif // _DEBUG
}

_bool CTraceurGroundMantle::Ready_Enter(void* pArg)
{
	const PARKOUR_DECISION& Snap = Enter_Decision(pArg);
	if (!Snap.isValid)
		return false;

	const OBSTACLE_GEOMETRY& Geo = Enter_Perception(pArg).Geometry; // 판정 당시의 스냅샷
	if (!Geo.Top.isReachable)
		return false;

	m_pColliderCom->Set_Gravity(false);
	m_pMotionWarpCom->Clear_WarpTargets();
	m_pMotionWarpCom->Set_WarpTarget("MantleTop", Geo.Top.vEdgePos);

	return true;
}

#ifdef _DEBUG
void CTraceurGroundMantle::Draw_Debug()
{
	const OBSTACLE_GEOMETRY& Geo = m_pEnvQueryCom->Get_Perception().Geometry;
	CGameInstance::GetInstance()->Add_DebugSphere(XMLoadFloat3(&Geo.Top.vEdgePos), 0.3f, JPH::Color(0.f, 255.f, 255.f, 1.f));
}
#endif // _DEBUG

CTraceurGroundMantle* CTraceurGroundMantle::Create(CTraceur* pOwner)
{
	CTraceurGroundMantle* pInstance = new CTraceurGroundMantle();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundMantle");
		return nullptr;
	}

	return pInstance;
}

void CTraceurGroundMantle::Free()
{
	__super::Free();
}
