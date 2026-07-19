#include "EditorPch.h"
#include "EditorPropWeapon.h"
#include "Model.h"
#include "Animator.h"

CEditorPropWeapon::CEditorPropWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEditorProp{ pDevice, pContext }
{
}

CEditorPropWeapon::CEditorPropWeapon(const CPartObject& Prototype)
    : CEditorProp(Prototype )
{
}

HRESULT CEditorPropWeapon::Initialize_Prototype()
{
    if (FAILED(CEditorProp::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEditorPropWeapon::Initialize_Clone(void* pArg)
{
    PROP_DESC* pDesc = static_cast<PROP_DESC*>(pArg);
    ASSERT_CRASH(pDesc);

    if (FAILED(CPartObject::Initialize_Clone(pDesc)))
        return E_FAIL;

    Ready_Components(pDesc);
    Ready_Variables(pDesc);
    Ready_Positions(pDesc);

    // Default Animation
#ifdef _DEBUG
    if (m_pModelCom && !m_pModelCom->Get_AnimationNames().empty())
        m_strCurrentAnimName = m_pModelCom->Get_AnimationNames()[0];
#endif

    return S_OK;
}

void CEditorPropWeapon::Priority_Update(_float fTimeDelta)
{
    CEditorProp::Priority_Update(fTimeDelta);
}

void CEditorPropWeapon::Update(_float fTimeDelta)
{
    CEditorProp::Update(fTimeDelta);

    m_fTimeDelta = fTimeDelta;

    // Animation Play
    if (m_IsPlayAnimation && m_pModelCom)
    {
        if (nullptr == m_pAnimator)
            m_pAnimator = CAnimator::Create(m_pModelCom);

        m_pAnimator->Play_Animation_CPU(m_strCurrentAnimName, fTimeDelta, &m_fTrackPosition, false, true, 1.f);
        m_pAnimator->Sync_RootNode(m_pTransformCom, fTimeDelta);
    }

    // Combined Matrix 계산
    XMStoreFloat4x4(&m_CombinedMatrix,
        m_pTransformCom->Get_WorldMatrix() *
        XMLoadFloat4x4(m_pSocketMatrix) *
        m_pParentTransform->Get_WorldMatrix());
}

void CEditorPropWeapon::Late_Update(_float fTimeDelta)
{
    CEditorProp::Late_Update(fTimeDelta);

    if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this)))
        return;
}

void CEditorPropWeapon::Render()
{
    Bind_Resources();

    _uint iNumMeshes = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0)))
            CRASH("Ready Diffuse Texture Failed");

        if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
            CRASH("Ready Bone Matrices Failed");

        if (FAILED(m_pShaderCom->Begin(m_iShaderPath)))
            CRASH("Ready Shader Begin Failed");

        if (FAILED(m_pModelCom->Render(i)))
            CRASH("Ready Render Failed");
    }
}

#ifdef _DEBUG
const vector<_string>& CEditorPropWeapon::Get_AnimationNames() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_AnimationNames();
}

_float* CEditorPropWeapon::Get_TrackPositionPtr(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_TrackPositionPtr(strAnimName);
}

_float CEditorPropWeapon::Get_Duration(const _string& strAnimName)
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_Duration(strAnimName);
}

const _float CEditorPropWeapon::Get_CurrentAnimationDuration() const
{
    ASSERT_CRASH(m_pModelCom);
    return m_pModelCom->Get_Duration(m_strCurrentAnimName);
}

void CEditorPropWeapon::Set_TrackPosition(_float fTrackPosition)
{
    ASSERT_CRASH(m_pModelCom);
    m_pModelCom->Set_TrackPosition(m_strCurrentAnimName, fTrackPosition);

    if (!m_IsPlayAnimation)
    {
        if (nullptr == m_pAnimator)
            m_pAnimator = CAnimator::Create(m_pModelCom);

        m_fTrackPosition = fTrackPosition;
        m_pAnimator->Play_Animation_CPU(m_strCurrentAnimName, 0.f, &m_fTrackPosition, false, true, 1.f);
    }
}
#endif

void CEditorPropWeapon::Bind_Resources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedMatrix)))
		CRASH("Failed Bind Matrix");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed Bind Matrix");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed Proj Matrix");
}

void CEditorPropWeapon::Ready_Components(const PROP_DESC* pDesc)
{
    // Shader
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->shaderData.first)
        , pDesc->shaderData.second, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        CRASH("Shader");

    // Model
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(pDesc->modelData.first)
        , pDesc->modelData.second, TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
        CRASH("Model");
}

void CEditorPropWeapon::Ready_Variables(const PROP_DESC* pDesc)
{
    m_pSocketMatrix = pDesc->pSocketMatrix;
    m_pParentTransform = pDesc->pParentTransform;
    m_iShaderPath = 0; // Default shader path
}

void CEditorPropWeapon::Ready_Positions(const PROP_DESC* pDesc)
{
    _fvector vPos = XMVectorSetW(XMLoadFloat3(&pDesc->vPosition), 1.f);
    m_pTransformCom->Set_State(STATE::POSITION, vPos);
    m_pTransformCom->Scale(pDesc->vScale);
}

CEditorPropWeapon* CEditorPropWeapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEditorPropWeapon* pInstance = new CEditorPropWeapon(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        Safe_Release(pInstance);
        MSG_BOX("Create Failed CEditorPropWeapon");
    }
    return pInstance;
}

CGameObject* CEditorPropWeapon::Clone(void* pArg)
{
    CEditorPropWeapon* pInstance = new CEditorPropWeapon(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        Safe_Release(pInstance);
        MSG_BOX("Clone Failed CEditorPropWeapon");
    }
    return pInstance;
}

void CEditorPropWeapon::Free()
{
    CEditorProp::Free();
}
