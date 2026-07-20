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

	if (!Ready_Enter(pArg))
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

_bool CTraceurGroundVault::Ready_Enter(void* pArg)
{
	if (!Enter_Decision(pArg).isValid)
		return false;

	if (!Select_Animation())
		return false;

	m_pColliderCom->Set_Gravity(false);

	const OBSTACLE_GEOMETRY& Geo = Enter_Perception(pArg).Geometry; // 판정 당시의 상태를 가져오기 위함.
	m_pMotionWarpCom->Clear_WarpTargets();
	if (Geo.Top.isReachable)
	{
		_float3 vPos = Geo.Top.vStandPos;
		/*_float3 vOffset{};
		XMStoreFloat3(&vOffset, m_pColliderCom->Get_Offset());
		vPos.y += vOffset.y;*/
		m_pMotionWarpCom->Set_WarpTarget("VaultTop", vPos);
	}
		
	if (Geo.Landing.hasSpace)
		m_pMotionWarpCom->Set_WarpTarget("VaultLand", Geo.Landing.vPos);

	return true;
}

_bool CTraceurGroundVault::Select_Animation()
{
	return true;
}

#ifdef _DEBUG
void CTraceurGroundVault::Draw_Debug()
{
	const OBSTACLE_GEOMETRY& Geo = m_pEnvQueryCom->Get_Perception().Geometry;
	CGameInstance* pGI = CGameInstance::GetInstance();

	_float3 vPos = Geo.Top.vStandPos;
	//_float3 vOffset{};
	//XMStoreFloat3(&vOffset, m_pColliderCom->Get_Offset());
	//vPos.y += vOffset.y;

	//pGI->Add_DebugSphere(XMLoadFloat3(&Geo.Top.vEdgePos), 0.3f, JPH::Color(0.f, 255.f, 255.f, 1.f));
	pGI->Add_DebugSphere(XMLoadFloat3(&vPos), 0.3f, JPH::Color(0.f, 255.f, 255.f, 1.f));
	pGI->Add_DebugSphere(XMLoadFloat3(&Geo.Landing.vPos), 0.3f, JPH::Color(255.f, 255.f, 255.f, 1.f));
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
