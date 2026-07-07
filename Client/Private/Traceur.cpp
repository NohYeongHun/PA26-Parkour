#include "ClientPch.h"
#include "Traceur.h"
#include "GameSystem.h"

#include "SpringCamera.h"

#include "Rigidbody.h"
#include "Collider.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

#include "TraceurFactory.h"


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

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	if (FAILED(Ready_Variables(pDesc)))
		return E_FAIL;

	CTraceurFactory::Register_Camera(LEVEL::STATIC, m_eCurLevel, this, m_pGameInstance, &m_pSpringCamera);
	CTraceurFactory::Register_KeyInputs(m_pInputControllerCom, this);

	

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

	/* 임시 */
	m_AnimPlayDesc.fTimeDelta = fTimeDelta;
	_bool IsPlayAnimationEnd = m_pModelCom->Play_Animation_CPU(m_AnimPlayDesc.strAnimationName, m_AnimPlayDesc, m_RootModtionDesc);
	m_pModelCom->Sync_RootNode(m_pTransformCom, m_AnimPlayDesc.fTimeDelta);

	// 1. StateMachine => 이동량, 회전량 생성

	// 2. Physics => 바뀐 이동량에 따른 물리 확인.
	Update_Physics(fTimeDelta);

	// 3. 환경 탐지.
	Update_EnvQuery(fTimeDelta);
	
	// 4. 해당 위치를 이용한 카메라 이동.
	Sync_Camera(fTimeDelta);
}

void CTraceur::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
	// 1. 물리 반영된 위치로 지정
	Sync_Transform();

	

	Ready_Render();
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

	Move(CMovementComponent::Calculate_Direction(m_pInputControllerCom)
		, m_pSpringCamera->Get_LookVector_NoPitch()
		, m_pSpringCamera->Get_RightVector_NoPitch(), fTimeDelta, 0.5f);

	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D1), KEYSTATE::UP))
	{
		m_AnimPlayDesc.strAnimationName = "Run";
		m_AnimPlayDesc.fBlendDuration = 0.2f;
		m_RootModtionDesc.isEnable = false;
		m_pColliderCom->Set_Gravity(true);
	}

	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D2), KEYSTATE::UP))
	{
		m_AnimPlayDesc.strAnimationName = "Idle";
		m_AnimPlayDesc.fBlendDuration = 0.2f;
		m_pColliderCom->Set_Gravity(true);
	}

	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D3), KEYSTATE::UP))
	{
		m_AnimPlayDesc.strAnimationName = "ForwardShortJump";
		m_RootModtionDesc.isEnable = true;
		m_AnimPlayDesc.fBlendDuration = 0.f;
		m_pColliderCom->Set_Gravity(false);
	}

	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D4), KEYSTATE::PRESS))
	{
		m_pColliderCom->Set_Gravity(false);
		m_pTransformCom->Go_Dir(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);
		
	}
	if (m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::D5), KEYSTATE::PRESS))
	{
		m_pColliderCom->Set_Gravity(true);
	}

	
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
	
	// 주변 환경을 탐지합니다.
	m_pEnvQueryCom->Execute();
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

	if (FAILED(Ready_EnvQueryComponents(pDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTraceur::Ready_EnvQueryComponents(const CHARACTER_DESC* pDesc)
{
	CEnvironmentQueryComponent::ENV_QUERY_DESC EnvCompDesc{};
	EnvCompDesc.pOwner = this;
	EnvCompDesc.fShapeTraceDistance = 4.f;
	EnvCompDesc.fLineTraceDistance = 4.f;
	EnvCompDesc.eTargetLayer = COLLISIONLAYER::PARKOUR;
	if (FAILED(Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_EnvQuery"),
		TEXT("Com_EnvQuery"), reinterpret_cast<CComponent**>(&m_pEnvQueryCom), &EnvCompDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTraceur::Ready_Variables(const CHARACTER_DESC* pDesc)
{
	m_eCurLevel = pDesc->eCurLevel;

	_fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
	m_pTransformCom->Scale(pDesc->vScale);

	m_AnimPlayDesc.strAnimationName = "Idle";
	m_AnimPlayDesc.pTrackPosition = &m_fTrackPosition;
	m_AnimPlayDesc.isFacial = false;

	m_RootModtionDesc.fRate = 1.f;
	m_RootModtionDesc.isEnable = true;
	m_RootModtionDesc.isRotate = true;
	m_RootModtionDesc.isTranslate = true;

	m_pColliderCom->Set_Gravity(true);

	return S_OK;
}

HRESULT CTraceur::Bind_Matrices()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
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
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pEnvQueryCom);
}
