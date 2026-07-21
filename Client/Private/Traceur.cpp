#include "ClientPch.h"
#include "Traceur.h"
#include "GameSystem.h"

#include "SpringCamera.h"

#include "Rigidbody.h"
#include "Collider.h"
#include "MovementComponent.h"
#include "MeshAlignComponent.h"
#include "EnvironmentQueryComponent.h"
#include "ParkourDeciderComponent.h"
#include "MotionWarpingComponent.h"
#include "TraceurFactory.h"
#include "TraceurState_Enum.h"
#include "TraceurState.h"
#include "TraceurStateNames.h"
#include "AnimationController.h"
#include "IKComponent.h"
#include "StateBlackboard.h"
#include "TagRegistry.h"
#include "TransitionEvaluator.h"
#include "ClimbEvaluator.h"

#include "Engine_Profile.h"


CTraceur::CTraceur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter{ pDevice, pContext }
	, m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

CTraceur::CTraceur(const CTraceur& Prototype)
	: CCharacter(Prototype)
	, m_pGameSystem{ CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CTraceur::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CTraceur::Initialize_Clone(void* pArg)
{
	CHARACTER_DESC* pDesc = static_cast<CHARACTER_DESC*>(pArg);
	if (FAILED(__super::Initialize_Clone(pDesc)))
		return E_FAIL;

	CMeshAlignComponent::COMPONENT_DESC MeshAlignDesc{};
	MeshAlignDesc.pOwner = this;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_MeshAlign"),
		TEXT("Com_MeshAlign"), reinterpret_cast<CComponent**>(&m_pMeshAlignCom), &MeshAlignDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	if (FAILED(CTagRegistry::Load("../../Client/Bin/Data/TraceurTags.json", m_pStateBlackboardCom)))
		return E_FAIL;
	Bind_CollectSlots();
	
	auto StateFlagCallBack = [this](const _string& strFlag, _bool isOn) { Notify_StateFlag(strFlag, isOn); };

	auto MotionWarpCallBack = [this](const _string& strName, _bool isStart, _float fEndPos, _bool bTrans, _bool bRot) {
		if (m_pMotionWarpCom && m_pColliderCom)
		{
			m_pMotionWarpCom->On_WarpNotify(strName, isStart, fEndPos, bTrans, bRot);
			m_pColliderCom->Set_Gravity(false);
		};
	};

	auto IKCallBack = [this](const vector<IK_BINDING>& Bindings, _float fBlendSec, _bool isBegin) {
		On_IK_Notify(Bindings, fBlendSec, isBegin);
	};



	m_pModelCom->Register_AllNotifies(
		pDesc->strNotfiyFolderPath,
		nullptr,   
		nullptr,   
		nullptr,   
		StateFlagCallBack,
		MotionWarpCallBack,
		IKCallBack
	);

	

	if (FAILED(Ready_Variables(pDesc)))
		return E_FAIL;

	m_pAnimControllerCom->Bind_Parameter("Locomotion1D",   m_pMoveCom->Get_LocomotionWeightPtr());
	m_pAnimControllerCom->Bind_Parameter2D("Locomotion2D", m_pMoveCom->Get_LocomotionWeight2DPtr());
	if (FAILED(m_pAnimControllerCom->Load("../../Client/Bin/Data/TraceurAnimations.json",
		[](const _string& strPath, Engine::StateKey& OutKey)
		{ return CTraceurStateNames::Resolve_StateKey(strPath, OutKey); })))
		return E_FAIL;

	CTraceurFactory::Register_Camera(LEVEL::STATIC, m_eCurLevel, this, m_pGameInstance, &m_pSpringCamera);
	CTraceurFactory::Register_KeyInputs(m_pInputControllerCom, this);
	CTraceurFactory::Register_States(m_pStateMachineCom, this);

	return S_OK;
}

void CTraceur::Priority_Update(_float fTimeDelta)
{
	PROFILE_ZONE();
	__super::Priority_Update(fTimeDelta);
	PreUpdate_Input(fTimeDelta);
	Save_PreviousPosition();
	Handle_Input(fTimeDelta);
}

void CTraceur::Update(_float fTimeDelta)
{
	PROFILE_ZONE();
	__super::Update(fTimeDelta);

	// 0. 수집 — 지난 프레임 Decider 결정을 Blackboard 플래그로 (舊 각 State의 Apply_DecisionFlags와 동일 시점/데이터)
	Collect_StateFlags();

	// 1. StateMachine => 이동량, 회전량 생성 (State는 플래그를 쓰기만 하고 전환하지 않음)
	m_pStateMachineCom->Update(fTimeDelta);

	// 2. Transition Rule 체크 => 조건에 맞는다면 상태 전환
	if (!m_pTransitionEvalCom->Evaluate())
		m_pStateBlackboardCom->Clear_Bools();

	// 3. 특수한 동작에서 캐릭터의 회전이나 이동값에 대한 행렬을 담당함.
	m_pMeshAlignCom->Update(fTimeDelta);

	// 4. 물리 업데이트
	Update_Physics(fTimeDelta);

	// 5. 환경 평가
	Update_EnvQuery(fTimeDelta);

	// 6. 카메라가 플레이어 Transform을 따라다니게 Update Target을 설정.
	Sync_Camera(fTimeDelta);

	
}

void CTraceur::Bind_CollectSlots()
{
	CStateBlackboard* pBB = m_pStateBlackboardCom;
	auto Bind = [pBB](const _string& strName)
	{
		const _uint iSlot = pBB->Find_Slot(strName);
		ASSERT_CRASH(iSlot != UINT_MAX);
		return iSlot;
	};

	m_CollectSlots.Grounded     = Bind("Phys.Grounded");
	m_CollectSlots.Supported    = Bind("Phys.Supported");
	m_CollectSlots.Unsupported  = Bind("Phys.Unsupported");
	m_CollectSlots.Falling      = Bind("Phys.Falling");
	m_CollectSlots.Airborne     = Bind("Phys.Airborne");
	m_CollectSlots.Move         = Bind("Intent.Move");
	m_CollectSlots.Run          = Bind("Intent.Run");
	m_CollectSlots.Jump         = Bind("Intent.Jump");
	m_CollectSlots.Forward      = Bind("Intent.Forward");
	m_CollectSlots.Down         = Bind("Intent.Down");
	m_CollectSlots.Left         = Bind("Intent.Left");
	m_CollectSlots.Right        = Bind("Intent.Right");
	m_CollectSlots.JumpPress    = Bind("Intent.JumpPress");
	m_CollectSlots.CmdLowVault  = Bind("Cmd.LowVault");
	m_CollectSlots.CmdHighVault = Bind("Cmd.HighVault");
	m_CollectSlots.CmdHighMantle= Bind("Cmd.HighMantle");
	m_CollectSlots.CmdLowMantle = Bind("Cmd.LowMantle");
	m_CollectSlots.CmdClimb     = Bind("Cmd.Climb");
	m_CollectSlots.CmdHang      = Bind("Cmd.Hang");
	m_CollectSlots.CmdWallRun   = Bind("Cmd.WallRun");
	m_CollectSlots.EvalFall     = Bind("Eval.Climb.Fall");
	m_CollectSlots.EvalLand     = Bind("Eval.Climb.Land");
	m_CollectSlots.EvalArrive   = Bind("Eval.Climb.Arrive");
	m_CollectSlots.EvalClimbMantle = Bind("Eval.Climb.Mantle");
	m_CollectSlots.EvalKneeHit  = Bind("Eval.Climb.KneeHit");
}

void CTraceur::Collect_StateFlags()
{
	const PARKOUR_DECISION& D = m_pParkourDeciderCom->Get_Decision();
	CStateBlackboard* pBB = m_pStateBlackboardCom;
	pBB->Set(m_CollectSlots.Grounded,     D.isGrounded);
	pBB->Set(m_CollectSlots.Supported,    D.isSupported);
	pBB->Set(m_CollectSlots.Unsupported,  !D.isSupported);
	pBB->Set(m_CollectSlots.Falling,      D.isFalling);
	pBB->Set(m_CollectSlots.Airborne,     !D.isGrounded);
	pBB->Set(m_CollectSlots.Move,         D.hasMoveInput);
	pBB->Set(m_CollectSlots.Run,          D.wantsRun);
	pBB->Set(m_CollectSlots.Jump,         D.wantsJump);
	pBB->Set(m_CollectSlots.Forward,      D.wantsForward);
	pBB->Set(m_CollectSlots.Down,         D.wantsDown);
	pBB->Set(m_CollectSlots.Left,         D.wantsLeft);
	pBB->Set(m_CollectSlots.Right,        D.wantsRight);
	pBB->Set(m_CollectSlots.JumpPress,    D.wantsJumpPress);
	pBB->Set(m_CollectSlots.CmdLowVault,  D.eCommand == PARKOUR_ACTION::LOW_VAULT);
	pBB->Set(m_CollectSlots.CmdHighVault, D.eCommand == PARKOUR_ACTION::HIGH_VAULT);
	pBB->Set(m_CollectSlots.CmdHighMantle,D.eCommand == PARKOUR_ACTION::HIGH_MANTLE);
	pBB->Set(m_CollectSlots.CmdLowMantle, D.eCommand == PARKOUR_ACTION::LOW_MANTLE);
	pBB->Set(m_CollectSlots.CmdClimb,     D.eCommand == PARKOUR_ACTION::CLIMB);
	pBB->Set(m_CollectSlots.CmdHang,      D.eCommand == PARKOUR_ACTION::HANG);
	pBB->Set(m_CollectSlots.CmdWallRun,   D.eCommand == PARKOUR_ACTION::WALL_RUN);

	// Climb 도메인 플래그
	if (m_pStateMachineCom->Get_CurrentCategory() == ENUM_CLASS(EStateCategory::CLIMB))
	{
		const CLIMB_EVAL& E = m_pClimbEvalCom->Get_Eval();
		pBB->Set(m_CollectSlots.EvalFall,    E.shouldFall);
		pBB->Set(m_CollectSlots.EvalLand,    E.isLanded);
		pBB->Set(m_CollectSlots.EvalArrive,  E.isArrived);
		pBB->Set(m_CollectSlots.EvalClimbMantle,  E.canMantle);
		pBB->Set(m_CollectSlots.EvalKneeHit, E.kneeHit);
	}
}

void CTraceur::Late_Update(_float fTimeDelta)
{
	PROFILE_ZONE();
	__super::Late_Update(fTimeDelta);
	// 1. 물리 반영된 위치로 지정
	Sync_Transform();

	// 2. IK 적용 예정 => 모든 물리 로직으로 인한 위치 보정이 끝난 시점이 IK를 적용할 시점.

	// Render
	Ready_Render();

#ifdef _DEBUG
	m_pGameInstance->Add_DebugSphere(m_pTransformCom->Get_State(STATE::POSITION), 0.1f, JPH::Color(0.F, 0.F, 0.F));
	if (m_pMotionWarpCom)
		m_pMotionWarpCom->Update_DebugTrail();
#endif // _DEBUG

	
}

void CTraceur::Render()
{
	if(FAILED(Bind_Matrices()))
		CRASH("Failed to Bind Matrices")

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
			return;

		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
			continue;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			CRASH("Ready Bone Matrices Failed");

		if (FAILED(m_pShaderCom->Begin(ENUM_CLASS(SHADER_VTXANIMMESH_PATH::NORMAL))))
			CRASH("Shader Begin Failed");

		if (FAILED(m_pModelCom->Render(i)))
			CRASH("Render Failed");
	}
}

_vector CTraceur::Get_CamForward() const
{
	return m_pSpringCamera->Get_LookVector_NoPitch();
}

_vector CTraceur::Get_CamRight() const
{
	return m_pSpringCamera->Get_RightVector_NoPitch();
}

void CTraceur::Notify_StateFlag(const _string& strFlag, _bool isOn)
{
	if (m_pStateBlackboardCom)
		m_pStateBlackboardCom->Set_Notify(strFlag, isOn);
}

void CTraceur::On_IK_Notify(const vector<IK_BINDING>& Bindings, _float fBlendSec, _bool isBegin)
{
	// 여러개의 Goal을 실행시킵니다.
	for (const auto& bind : Bindings) {
		if (isBegin) {
			m_pIKCom->Begin_Goal(bind.strGoalName, bind.eMode, bind.fPosWeight, bind.fRotWeight, fBlendSec);
		}
		else {
			m_pIKCom->End_Goal(bind.strGoalName, fBlendSec);
		}
	}
}

// 객체 생성시에 pDesc에 등록된 정보를 가져올 수 있습니다.
void CTraceur::OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
	
}

void CTraceur::OnCollider_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold)
{
}

void CTraceur::PreUpdate_Input(_float fTimeDelta)
{
	if (nullptr == m_pInputControllerCom)
		return;

	m_pInputControllerCom->Update();
}

void CTraceur::Save_PreviousPosition()
{
	if (nullptr == m_pTransformCom)
		return;

	m_pTransformCom->Save_PreviousPosition();
}

void CTraceur::Handle_Input(_float fTimeDelta)
{
#ifdef _DEBUG

	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D1), KEYSTATE::UP))
	{
		m_pColliderCom->Set_Gravity(false);
	}

	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D2), KEYSTATE::UP))
	{
		m_pColliderCom->Set_Gravity(true);
	}

	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D3), KEYSTATE::UP))
	{
		CGameSystem::GetInstance()->Reload_TransitionTable();
		CGameSystem::GetInstance()->Reload_ParkourTuning();
		m_pAnimControllerCom->Reload();
	}

	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D4), KEYSTATE::UP))
		m_IsShowTrajectory = !m_IsShowTrajectory;
#endif // _DEBUG

}

void CTraceur::Update_Collider(_float fTimeDelta)
{
	_vector vVelocity = m_pTransformCom->Get_Velocity();
	m_pColliderCom->Update(vVelocity / fTimeDelta);
}


void CTraceur::Update_Physics(_float fTimeDelta)
{
	Update_Collider(fTimeDelta);
}

void CTraceur::Update_EnvQuery(_float fTimeDelta)
{
	if (nullptr == m_pEnvQueryCom)
		return;

	if (nullptr != m_pStateMachineCom)
	{
		// 실행 중 EnvQuery/Decide를 멈춘다.
		const auto& Key = m_pStateMachineCom->Get_CurrentStateKey();
		if (Key.iCategory == ENUM_CLASS(EStateCategory::GROUND)
			&& (Key.iSubState == ENUM_CLASS(ETraceurGroundState::Vault)
			 || Key.iSubState == ENUM_CLASS(ETraceurGroundState::Mantle)))
			return;
	}

	m_pEnvQueryCom->Execute();

	m_pParkourDeciderCom->Decide(m_pEnvQueryCom->Get_Perception(), fTimeDelta);

	if (m_pStateMachineCom->Get_CurrentCategory() == ENUM_CLASS(EStateCategory::CLIMB))
		m_pClimbEvalCom->Evaluate(m_pEnvQueryCom->Get_Perception(),
			m_pParkourDeciderCom->Get_Decision(), fTimeDelta);
}

void CTraceur::Sync_Camera(_float fTimeDelta)
{
	if (nullptr == m_pSpringCamera)
		return;

	m_pSpringCamera->Update_Target(m_pTransformCom->Get_State(STATE::POSITION), 1.2f);
}

// 변경된 위치를 Transform에 반영해 줍니다.
void CTraceur::Sync_Transform()
{
	m_pColliderCom->Sync_Position(m_pTransformCom);
}



void CTraceur::Ready_Render()
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}




HRESULT CTraceur::Ready_Components(const CHARACTER_DESC* pDesc)
{
	if (FAILED(__super::Ready_Components(pDesc)))
		return E_FAIL;

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->inputControllerData.first)
		, pDesc->inputControllerData.second, TEXT("Com_InputController"), reinterpret_cast<CComponent**>(&m_pInputControllerCom), nullptr)))
		CRASH("Input Controller");

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_StateMachine"),
		TEXT("Com_StateMachine"), reinterpret_cast<CComponent**>(&m_pStateMachineCom), nullptr)))
		CRASH("StateMachine");

	m_vColliderOffSet = { 0.f, 0.8f, 0.f };
	m_fColliderRadius = 0.4f;
	m_fColliderHeight = 0.8f;

	/* 실제 콜라이더 */
	Engine::CCollider::COLLIDER_DESC ColliderDesc{};
	ColliderDesc.vPos = pDesc->vPosition;
	ColliderDesc.vOffset = m_vColliderOffSet;
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::PLAYER);
	ColliderDesc.fHeight = m_fColliderHeight;
	ColliderDesc.fRadius = m_fColliderRadius;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc)))
		CRASH("Collider");

	{
		const _float fRadius = m_pColliderCom->Get_Radius();
		const _float fTotal  = m_pColliderCom->Get_Height() + 2.f * fRadius;
		m_BodyProfile.fHeight      = fTotal;
		m_BodyProfile.fRadius      = fRadius;
		m_BodyProfile.fKneeHeight  = fTotal * 0.35f;
		m_BodyProfile.fChestHeight = fTotal * 0.8f;
		m_BodyProfile.fHeadHeight  = fTotal * 1.1f;
		m_BodyProfile.fMaxReach    = fTotal * 1.5f;
	}

	if (FAILED(Ready_EnvQueryComponents(pDesc)))
		return E_FAIL;

	CIKComponent::IKCOMPONENT_DESC IKDesc{};
	IKDesc.pOwner = this;
	IKDesc.pOwnerModelCom = m_pModelCom;
	IKDesc.pOwnerTransformCom = m_pTransformCom;

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_IK"),
		TEXT("Com_IK"), reinterpret_cast<CComponent**>(&m_pIKCom), &IKDesc)))
		CRASH("IK");

	return S_OK;
}

HRESULT CTraceur::Ready_EnvQueryComponents(const CHARACTER_DESC* pDesc)
{
	CEnvironmentQueryComponent::ENV_QUERY_DESC EnvCompDesc{};
	EnvCompDesc.pOwner = this;
	EnvCompDesc.fShapeTraceDistance = 2.f;
	EnvCompDesc.fLineTraceDistance = 2.f;
	EnvCompDesc.eTargetLayer = COLLISIONLAYER::PARKOUR;
	EnvCompDesc.pBodyProfile = &m_BodyProfile;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_EnvQuery"),
		TEXT("Com_EnvQuery"), reinterpret_cast<CComponent**>(&m_pEnvQueryCom), &EnvCompDesc)))
		return E_FAIL;

	CParkourDeciderComponent::PARKOUR_DECIDER_DESC DeciderDesc{};
	DeciderDesc.pOwner = this;
	DeciderDesc.pBodyProfile = &m_BodyProfile;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_ParkourDecider"),
		TEXT("Com_ParkourDecider"), reinterpret_cast<CComponent**>(&m_pParkourDeciderCom), &DeciderDesc)))
		return E_FAIL;

	CMotionWarpingComponent::MOTION_WARP_DESC WarpDesc{};
	WarpDesc.pOwner = this;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_MotionWarp"),
		TEXT("Com_MotionWarp"), reinterpret_cast<CComponent**>(&m_pMotionWarpCom), &WarpDesc)))
		return E_FAIL;

	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_StateBlackboard"),
		TEXT("Com_StateBlackboard"), reinterpret_cast<CComponent**>(&m_pStateBlackboardCom), nullptr)))
		return E_FAIL;

	CTransitionEvaluator::TRANSITION_EVALUATOR_DESC TransEvalDesc{};
	TransEvalDesc.pOwner = this;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_TransitionEvaluator"),
		TEXT("Com_TransitionEvaluator"), reinterpret_cast<CComponent**>(&m_pTransitionEvalCom), &TransEvalDesc)))
		return E_FAIL;

	CClimbEvaluator::CLIMB_EVALUATOR_DESC ClimbEvalDesc{};
	ClimbEvalDesc.pOwner = this;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_ClimbEvaluator"),
		TEXT("Com_ClimbEvaluator"), reinterpret_cast<CComponent**>(&m_pClimbEvalCom), &ClimbEvalDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTraceur::Ready_Variables(const CHARACTER_DESC* pDesc)
{
	m_eCurLevel = pDesc->eCurLevel;

	_fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	m_pTransformCom->Scale(pDesc->vScale);

	m_pColliderCom->Set_Gravity(true);

	return S_OK;
}

HRESULT CTraceur::Bind_Matrices()
{
	_float4x4 worldMatrix{};
	XMStoreFloat4x4(&worldMatrix, m_pMeshAlignCom->Get_LocalMatrix() * m_pTransformCom->Get_WorldMatrix());

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &worldMatrix)))
		CRASH("Failed Bind Matrix");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed Bind Matrix");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed Proj Matrix");

	return S_OK;
}


CTraceur* CTraceur::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTraceur* pInstance = new CTraceur(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTraceur");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTraceur::Clone(void* pArg)
{
	CTraceur* pInstance = new CTraceur(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Clone Failed : CTraceur");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTraceur::Free()
{
	__super::Free();
	Safe_Release(m_pGameSystem);

	// GameObjects
	Safe_Release(m_pSpringCamera);

	// Components
	Safe_Release(m_pInputControllerCom);
	Safe_Release(m_pStateMachineCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pEnvQueryCom);
	Safe_Release(m_pParkourDeciderCom);
	Safe_Release(m_pMotionWarpCom);
	Safe_Release(m_pAnimControllerCom);
	Safe_Release(m_pMeshAlignCom);
	Safe_Release(m_pStateBlackboardCom);
	Safe_Release(m_pTransitionEvalCom);
	Safe_Release(m_pClimbEvalCom);
	Safe_Release(m_pIKCom);
}
