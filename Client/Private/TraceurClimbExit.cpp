#include "ClientPch.h"
#include "TraceurClimbExit.h"
#include "AnimationController.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"
#include "MotionWarpingComponent.h"

HRESULT CTraceurClimbExit::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CTraceurClimbExit::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);

	const _bool isMantle = Is_MantleAnim();

	if (isMantle)
	{
		_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));

		const OBSTACLE_GEOMETRY& Geo = Enter_Perception(pArg).Geometry;
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

#ifdef _DEBUG
			cout << "현재 Edge, Stand Y: " << Geo.vTopEdgePos.y << ", " << Geo.vTopStandPos.y << endl;
#endif
		}

		if (Geo.hasLandingSpace)
			m_pMotionWarpCom->Set_WarpTarget("VaultLand", Geo.vLandingPos);
		m_pColliderCom->Set_Gravity(false);
	}
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
#ifdef _DEBUG
	if (Is_MantleAnim())
		Draw_DebugCurve();
#endif
}

void CTraceurClimbExit::Check_State()
{
	if (Get_Flag("Notify.HangDropEnd"))
	{
		m_pColliderCom->Set_Gravity(true);
	}
}

_bool CTraceurClimbExit::Is_MantleAnim() const
{
	ETraceurClimbExit eType = static_cast<ETraceurClimbExit>(Get_CurrentAnim());
	return eType == ETraceurClimbExit::ClimbingToTop || eType == ETraceurClimbExit::Climbing
		|| eType == ETraceurClimbExit::BracedHangToCrouch;
}

#ifdef _DEBUG
void CTraceurClimbExit::Draw_DebugCurve()
{
	const OBSTACLE_GEOMETRY& Geo = m_pEnvQueryCom->Get_Perception().Geometry;
	CGameInstance* pGI = CGameInstance::GetInstance();

	pGI->Add_DebugSphere(XMLoadFloat3(&Geo.vTopEdgePos), 0.3f, JPH::Color(255.f, 255.f, 255.f, 1.f));
	pGI->Add_DebugSphere(XMLoadFloat3(&Geo.vTopStandPos), 0.3f, JPH::Color(0.f, 255.f, 255.f, 1.f));
}
#endif

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
