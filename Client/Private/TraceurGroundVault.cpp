#include "ClientPch.h"
#include "TraceurGroundVault.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "MotionWarpingComponent.h"

HRESULT CTraceurGroundVault::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

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

}

void CTraceurGroundVault::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
	m_pMotionWarpCom->Abort_Warp();
}

void CTraceurGroundVault::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
	//m_pColliderCom->Set_Position(m_pTransformCom->Get_State(Engine::STATE::POSITION));
}

void CTraceurGroundVault::Late_Anim_Update(_float fTimeDelta)
{
#ifdef _DEBUG
	Draw_Debug();
#endif // _DEBUG

	
}

_bool CTraceurGroundVault::Ready_Enter()
{
	if (!m_Decision.isValid)
		return false;

	if (!Select_Animation())
		return false;

	const OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;
	m_pMotionWarpCom->Clear_WarpTargets();
	if (Geo.isTopReachable)
		m_pMotionWarpCom->Set_WarpTarget("VaultTop", Geo.vTopEdgePos);
	if (Geo.hasLandingSpace)
		m_pMotionWarpCom->Set_WarpTarget("VaultLand", Geo.vLandingPos);

#ifdef _DEBUG
	m_pMotionWarpCom->Reset_DebugTrail();   // 새 Vault 시작 → 이전 궤적 리셋
#endif

	return true;
}

_bool CTraceurGroundVault::Select_Animation()
{
	Request_Anim(ENUM_CLASS(ETraceurGroundVault::LowerVault));
	return true;
}

#ifdef _DEBUG
void CTraceurGroundVault::Draw_Debug()
{
	const OBSTACLE_GEOMETRY& Geo = m_Perception.Geometry;
	CGameInstance* pGI = CGameInstance::GetInstance();

	pGI->Add_DebugSphere(XMLoadFloat3(&Geo.vTopEdgePos), 0.3f, JPH::Color(0.f, 255.f, 255.f, 1.f));
	//pGI->Add_DebugSphere(XMLoadFloat3(&Geo.vLandingPos), 0.3f, JPH::Color(255.f, 255.f, 255.f, 1.f));
}
#endif // _DEBUG



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
