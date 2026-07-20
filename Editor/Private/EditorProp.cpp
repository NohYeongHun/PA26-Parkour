#include "EditorPch.h"
#include "EditorProp.h"
#include "Animator.h"

CEditorProp::CEditorProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPartObject{ pDevice, pContext }
{
}

CEditorProp::CEditorProp(const CPartObject& Prototype)
    : CPartObject(Prototype)
{
}

HRESULT CEditorProp::Initialize_Prototype()
{
    if (FAILED(CPartObject::Initialize_Prototype()))
        return E_FAIL;
    return S_OK;
}

HRESULT CEditorProp::Initialize_Clone(void* pArg)
{
    PROP_DESC* pDesc = static_cast<PROP_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    m_eWeaponType = pDesc->eWeaponType;
    return S_OK;
}

void CEditorProp::Priority_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;
    CPartObject::Priority_Update(fTimeDelta);
}

void CEditorProp::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    CPartObject::Update(fTimeDelta);
}

void CEditorProp::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    CPartObject::Late_Update(fTimeDelta);
}

void CEditorProp::Activate(_bool IsActive)
{
    SetActivate(IsActive);

	if (!IsActive)
	{
		m_fTrackPosition = 0.f;
	}

    if (nullptr == m_pRigidbodyCom)
        return;

    if (IsActive)
        m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
        
}


void CEditorProp::Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate, _bool IsRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate, _bool IsLoop)
{
    ASSERT_CRASH(m_pModelCom);

    if (nullptr == m_pAnimator)
        m_pAnimator = CAnimator::Create(m_pModelCom);

    m_IsAnimationEnd = m_pAnimator->Play_Animation_CPU(
        strAnimName, fTimeDelta, &m_fTrackPosition, false, IsRootMotion, fRootMotionRate);
    m_pAnimator->Sync_RootNode(m_pTransformCom, fTimeDelta);
}

void CEditorProp::Clear_Animation(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    m_pModelCom->Clear_Animation(strAnimName);
	m_fTrackPosition = 0.f;
	m_strCurrentAnimName.clear();  // 추가
}

#pragma region NOTIFY
void CEditorProp::Collider_Active(const _wstring& wStrColliderTag, _bool IsActive)
{
	if (!IsActive)
		m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
}
void CEditorProp::Effect_Active(const _wstring& wStrEffectTag)
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom)
		return;

	_matrix matWorld = XMLoadFloat4x4(&m_CombinedMatrix);
	m_pGameInstance->Spawn_PoolingObject(wStrEffectTag, matWorld, m_pModelCom);
}
void CEditorProp::Collider_Active(_bool isActive)
{
    if (!isActive)
        m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
    
}
#pragma endregion





void CEditorProp::Register_AllNotifies(const _string& strFolderPath)
{
	ASSERT_CRASH(m_pModelCom);
	auto colliderCallback = [this](const _wstring& tag, bool active) {
		this->Collider_Active(tag, active); // 
		};

	auto effectCallBack = [this](const _wstring& tag) {
		this->Effect_Active(tag);
		};
	auto objectCallBack = [this](const _wstring& tag) {
		this->Object_Func(tag);
		};

	m_pModelCom->Register_AllNotifies(strFolderPath, colliderCallback, effectCallBack, objectCallBack);
}

void CEditorProp::Free()
{
    CPartObject::Free();
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pComputeShaderCom);
    Safe_Release(m_pAnimator);
    Safe_Release(m_pModelCom);
    Safe_Release(m_pRigidbodyCom);
    m_pParentTransform = { nullptr };
}
