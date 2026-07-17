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
#include "StateBlackboard.h"
#include "TransitionEvaluator.h"
#include "ClimbEvaluator.h"


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

	

	m_pModelCom->Register_AllNotifies(
		pDesc->strNotfiyFolderPath,
		nullptr,   
		nullptr,   
		nullptr,   
		[this](const _string& strFlag, _bool isOn) { Notify_StateFlag(strFlag, isOn); },
		[this](const _string& strName, _bool isStart, _float fEndPos, _bool bTrans, _bool bRot) {
			if (m_pMotionWarpCom && m_pColliderCom)
			{
				m_pMotionWarpCom->On_WarpNotify(strName, isStart, fEndPos, bTrans, bRot);
				m_pColliderCom->Set_Gravity(false);
			}
				
		});

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
	__super::Priority_Update(fTimeDelta);
	PreUpdate_Input(fTimeDelta);
	Save_PreviousPosition();
	Handle_Input(fTimeDelta);
}

void CTraceur::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	// 0. 수집 — 지난 프레임 Decider 결정을 Blackboard 플래그로 (舊 각 State의 Apply_DecisionFlags와 동일 시점/데이터)
	Collect_StateFlags();

	// 1. StateMachine => 이동량, 회전량 생성 (State는 플래그를 쓰기만 하고 전환하지 않음)
	m_pStateMachineCom->Update(fTimeDelta);

	if (!m_pTransitionEvalCom->Evaluate())
		m_pStateBlackboardCom->Clear_Bools();

	m_pMeshAlignCom->Update(fTimeDelta);

	Update_Physics(fTimeDelta);

	Update_EnvQuery(fTimeDelta);

	Sync_Camera(fTimeDelta);
}

void CTraceur::Collect_StateFlags()
{
	const PARKOUR_DECISION& D = m_pParkourDeciderCom->Get_Decision();
	CStateBlackboard* pBB = m_pStateBlackboardCom;
	pBB->Set("Grounded",      D.isGrounded);
	pBB->Set("Supported",     D.isSupported);
	pBB->Set("Unsupported",   !D.isSupported);
	pBB->Set("Falling",       D.isFalling);
	pBB->Set("Airborne",      !D.isGrounded);
	pBB->Set("MoveInput",     D.hasMoveInput);
	pBB->Set("Run",           D.wantsRun);
	pBB->Set("Jump",          D.wantsJump);
	pBB->Set("Forward",       D.wantsForward);
	pBB->Set("Down",          D.wantsDown);
	pBB->Set("Cmd.LowVault",  D.eCommand == PARKOUR_ACTION::LOW_VAULT);
	pBB->Set("Cmd.HighVault", D.eCommand == PARKOUR_ACTION::HIGH_VAULT);
	pBB->Set("Cmd.Mantle",    D.eCommand == PARKOUR_ACTION::MANTLE);
	pBB->Set("Cmd.Climb",     D.eCommand == PARKOUR_ACTION::CLIMB);
	pBB->Set("Cmd.Hang",      D.eCommand == PARKOUR_ACTION::HANG);
	pBB->Set("Cmd.WallRun",   D.eCommand == PARKOUR_ACTION::WALL_RUN);

	// Climb 도메인 플래그 — CLIMB 카테고리에서만 유효
	if (m_pStateMachineCom->Get_CurrentCategory() == ENUM_CLASS(EStateCategory::CLIMB))
	{
		const CLIMB_EVAL& E = m_pClimbEvalCom->Get_Eval();
		pBB->Set("Fall",    E.shouldFall);
		pBB->Set("Land",    E.isLanded);
		pBB->Set("Arrive",  E.isArrived);
		pBB->Set("Mantle",  E.canMantle);
		pBB->Set("KneeHit", E.kneeHit);
	}
}

void CTraceur::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	// 1. 물리 반영된 위치로 지정
	Sync_Transform();
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
		const auto& Key = m_pStateMachineCom->Get_CurrentStateKey();
		if (Key.iCategory == ENUM_CLASS(EStateCategory::GROUND)
			&& Key.iSubState == ENUM_CLASS(ETraceurGroundState::Vault))
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
}
