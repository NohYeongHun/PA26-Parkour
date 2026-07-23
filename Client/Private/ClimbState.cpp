#include "ClientPch.h"
#include "ClimbState.h"
#include "Traceur.h"
#include "EnvironmentQueryComponent.h"

HRESULT CClimbState::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CClimbState::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
}

void CClimbState::OnUpdate(_float fTimeDelta)
{
	__super::OnUpdate(fTimeDelta);
}

void CClimbState::OnExit()
{
	__super::OnExit();
}

void CClimbState::Set_HangContext(_fvector vEdgePos, _fvector vWallNormal, const BodyID& GrabBodyID)
{
	HANG_CONTEXT& Ctx = m_pOwner->Get_HangContext();

	_vector vN = XMVector3Normalize(vWallNormal);

	Ctx.isValid = true;
	XMStoreFloat3(&Ctx.vGrabEdgePos, vEdgePos);
	XMStoreFloat3(&Ctx.vWallNormal, vN);
	Ctx.GrabBodyID = GrabBodyID;

	_vector vTrav = XMVectorNegate(vN);
	_vector vTopN = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_vector vL{}, vR{}, vOutN{};
	m_pEnvQueryCom->Compute_EdgeAnchor("TOP_LEFT_EDGE",  vEdgePos, vTrav, vTopN, vL, vOutN);
	m_pEnvQueryCom->Compute_EdgeAnchor("TOP_RIGHT_EDGE", vEdgePos, vTrav, vTopN, vR, vOutN);
	XMStoreFloat3(&Ctx.vGrabL, vL);
	XMStoreFloat3(&Ctx.vGrabR, vR);
	XMStoreFloat3(&Ctx.vGrabN, vOutN);

	// 클립 IK 노티가 이름으로 참조할 수 있게 래치 좌표를 공표 (좌표 소유는 여전히 컨텍스트)
	m_pEnvQueryCom->Publish_Anchor("GRAB_LEFT",  vL, vOutN);
	m_pEnvQueryCom->Publish_Anchor("GRAB_RIGHT", vR, vOutN);
	m_pEnvQueryCom->Publish_Anchor("GRAB_WALL",  vEdgePos, vN);	// 발 프로브 방향용 (노멀 = 벽)
}


void CClimbState::Free()
{
	__super::Free();
}
