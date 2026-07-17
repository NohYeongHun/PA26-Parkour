#include "ClientPch.h"
#include "TraceurState.h"
#include "AnimationController.h"
#include "Traceur.h"
#include "Model.h"
#include "Collider.h"
#include "MovementComponent.h"
#include "MeshAlignComponent.h"
#include "EnvironmentQueryComponent.h"
#include "ParkourDeciderComponent.h"
#include "MotionWarpingComponent.h"
#include "TraceurState_Enum.h"
#include "GameSystem.h"
#include "TransitionTable.h"
#include "TraceurStateNames.h"
#include "StateBlackboard.h"
#include "ClimbEvaluator.h"


HRESULT CTraceurState::Initialize(CTraceur* pOwner)
{
	m_pOwner = pOwner;
	if (nullptr == pOwner)
		return E_FAIL;

	m_pModelCom = dynamic_cast<CModel*>(pOwner->Get_Component(TEXT("Com_Model")));
	if (nullptr == m_pModelCom)
		return E_FAIL;

	m_pTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(TEXT("Com_Transform")));
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	m_pMeshAlignCom = dynamic_cast<CMeshAlignComponent*>(m_pOwner->Get_Component(TEXT("Com_MeshAlign")));
	if (nullptr == m_pMeshAlignCom)
		return E_FAIL;

	m_pColliderCom = dynamic_cast<CCollider*>(m_pOwner->Get_Component(TEXT("Com_Collider")));
	if (nullptr == m_pColliderCom)
		return E_FAIL;

	m_pStateMachinCom = dynamic_cast<CStateMachine*>(m_pOwner->Get_Component(TEXT("Com_StateMachine")));
	if (nullptr == m_pStateMachinCom)
		return E_FAIL;

	m_pInputControllerCom = dynamic_cast<CInputController*>(m_pOwner->Get_Component(TEXT("Com_InputController")));
	if (nullptr == m_pInputControllerCom)
		return E_FAIL;

	m_pEnvQueryCom = dynamic_cast<CEnvironmentQueryComponent*>(m_pOwner->Get_Component(TEXT("Com_EnvQuery")));
	if (nullptr == m_pEnvQueryCom)
		return E_FAIL;

	m_pDeciderCom = dynamic_cast<CParkourDeciderComponent*>(m_pOwner->Get_Component(TEXT("Com_ParkourDecider")));
	if (nullptr == m_pDeciderCom)
		return E_FAIL;

	m_pMotionWarpCom = dynamic_cast<CMotionWarpingComponent*>(m_pOwner->Get_Component(TEXT("Com_MotionWarp")));
	if (nullptr == m_pMotionWarpCom)
		return E_FAIL;

	m_pMoveCom = dynamic_cast<CMovementComponent*>(m_pOwner->Get_Component(TEXT("Com_Move")));
	if (nullptr == m_pMoveCom)
		return E_FAIL;

	m_pAnimCtrlCom = dynamic_cast<CAnimationController*>(m_pOwner->Get_Component(TEXT("Com_AnimController")));
	if (nullptr == m_pAnimCtrlCom)
		return E_FAIL;

	m_pBlackboardCom = dynamic_cast<CStateBlackboard*>(m_pOwner->Get_Component(TEXT("Com_StateBlackboard")));
	if (nullptr == m_pBlackboardCom)
		return E_FAIL;

	m_pClimbEvalCom = dynamic_cast<CClimbEvaluator*>(m_pOwner->Get_Component(TEXT("Com_ClimbEvaluator")));
	if (nullptr == m_pClimbEvalCom)
		return E_FAIL;

	m_iMoveKey |= static_cast<_uint>(KEYINPUT::W);
	m_iMoveKey |= static_cast<_uint>(KEYINPUT::A);
	m_iMoveKey |= static_cast<_uint>(KEYINPUT::S);
	m_iMoveKey |= static_cast<_uint>(KEYINPUT::D);

	return S_OK;
}

void CTraceurState::OnEnter(void* pArg)
{
	CState::OnEnter(pArg);

	_uint iReqAnim = 0;
	if (pArg)
	{
		auto* pDesc = static_cast<STATE_ENTER_DESC*>(pArg);
		if (pDesc->iAnimIndex != UINT_MAX)
			iReqAnim = pDesc->iAnimIndex;
	}

	Request_Anim(iReqAnim); // 항상 Request — 파생 OnEnter의 Select_Animation이 이후 덮어씀

	Clear_Flags();
	m_pBlackboardCom->Clear_Notifies();

#ifdef _DEBUG
	// 시작 위치 저장
	XMStoreFloat4x4(&m_StartMatrix, m_pTransformCom->Get_WorldMatrix());
#endif // _DEBUG
}

void CTraceurState::OnUpdate(_float fTimeDelta)
{
	CState::OnUpdate(fTimeDelta);
#ifdef _DEBUG
	if (m_pOwner->Show_DebugTrajectory())
	{
		const CState::ANIM_DATA* pData = m_pAnimCtrlCom->Get_CurrentAnimData();
		if (pData)
			m_pModelCom->Debug_RootMotionDraw(pData->AnimPlayDesc.strAnimationName,
				XMLoadFloat4x4(&m_StartMatrix), 0.1f, pData->RootMotionDesc);
	}
#endif // _DEBUG

	

	Check_State();
	Update_Animations(fTimeDelta);
	Late_Anim_Update(fTimeDelta);
	Check_Physics(fTimeDelta);
	Set_Flag("Anim.End", m_pAnimCtrlCom->IsAnimEnd());
}

void CTraceurState::OnExit()
{
	CState::OnExit();
}

_bool CTraceurState::IsVault() const
{
	const auto& stateKey = m_pStateMachinCom->Get_CurrentStateKey();
	return (EStateCategory::GROUND == static_cast<EStateCategory>(stateKey.iCategory) &&
		ETraceurGroundState::Vault == static_cast<ETraceurGroundState>(stateKey.iSubState));
}

_bool CTraceurState::Play_Animation(_float fTimeDelta)
{
	m_pAnimCtrlCom->Tick(fTimeDelta);
	return true;
}

void CTraceurState::Set_Flag(const _string& strName, _bool isOn)
{
	m_pBlackboardCom->Set(strName, isOn);
}

_bool CTraceurState::Get_Flag(const _string& strName) const
{
	return m_pBlackboardCom->Get(strName);
}

void CTraceurState::Clear_Flags()
{
	m_pBlackboardCom->Clear_Bools();
}

#ifdef _DEBUG
void CTraceurState::Debug_PrintFlag()
{
	// Blackboard에서 값을 읽어 출력
	cout << "[Blackboard Flags]" << endl;
}
#endif // _DEBUG

const ENV_PERCEPTION& CTraceurState::Enter_Perception(void* pArg) const
{
	auto* pDesc = static_cast<STATE_ENTER_DESC*>(pArg);
	if (pDesc && pDesc->hasEnvSnapshot)
		return pDesc->Perception;
	return m_pEnvQueryCom->Get_Perception();
}

const PARKOUR_DECISION& CTraceurState::Enter_Decision(void* pArg) const
{
	auto* pDesc = static_cast<STATE_ENTER_DESC*>(pArg);
	if (pDesc && pDesc->hasEnvSnapshot)
		return pDesc->Decision;
	return m_pDeciderCom->Get_Decision();
}

void CTraceurState::Request_Anim(_uint iAnimId)
{
	m_pAnimCtrlCom->Request(m_SelfKey, iAnimId);
}

_uint CTraceurState::Get_CurrentAnim() const
{
	return m_pAnimCtrlCom->Get_CurrentAnimId();
}

_uint CTraceurState::Get_CurrentAnimIndex()
{
	return m_pAnimCtrlCom->Get_CurrentAnimId();
}

void CTraceurState::Free()
{
	__super::Free();
}
