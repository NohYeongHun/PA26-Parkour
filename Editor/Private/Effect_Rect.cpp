#include "Editorpch.h"
#include "Effect_Rect.h"

CEffect_Rect::CEffect_Rect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CEffect_Rect::CEffect_Rect(const CEffect_Rect& Prototype)
    : CGameObject{ Prototype }
{
}

HRESULT CEffect_Rect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffect_Rect::Initialize_Clone(void* pArg)
{
	FXRECT_DESC* pDesc = static_cast<FXRECT_DESC*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components(*pDesc)))
        return E_FAIL;

    m_iShaderPass = pDesc->iShaderPass;
	m_iMaskFlag = pDesc->iMaskFlag;
	m_iColorFlag = pDesc->iColorFlag;

    m_vColor = pDesc->vColor;
    m_vLifeTime = pDesc->vLifeTime;

	m_fSweepSpeed = pDesc->fSweepSpeed;
	m_fSoft = pDesc->fSoft;

	m_fXSize = pDesc->fXSize;
	m_fYSize = pDesc->fYSize;

    _vector Pos = XMVectorSet(pDesc->vPos.x, pDesc->vPos.y, pDesc->vPos.z, 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, Pos);

	m_IsSprite = pDesc->IsSprite;
	m_iRow = pDesc->iRows;
	m_iCol = pDesc->iCols;
	m_IsRoot = pDesc->IsRootOn;
	m_IsLoop = pDesc->IsLoop;

    m_isActivate = true;

    return S_OK;
}

void CEffect_Rect::Priority_Update(_float fTimeDelta)
{
	if (m_fPhase >= 1.f)
		m_fPhase -= 1.f;
}

void CEffect_Rect::Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

   m_fSweep += fTimeDelta * m_fSweepSpeed;

   if (m_IsSprite)
	   Sprite_Update(fTimeDelta);

   if (m_IsRoot)
	   Update_Root_Transform();

   if (!m_IsLoop)
   {
	   m_vLifeTime.x += fTimeDelta;

	   if (m_vLifeTime.x >= m_vLifeTime.y)
	   {
		   m_isActivate = false;
		   m_vLifeTime.x = 0.f;
		   m_fSweep = 0.f;
		   m_fPhase = 0.f;
	   }
   }
   //일정주기마다 초기화 해줘야하나 ?

}

void CEffect_Rect::Late_Update(_float fTimeDelta)
{
    if (!m_isActivate)
        return;

    m_pGameInstance->Add_Render_Object(RENDERGROUP::EFFECT, this);
}

void CEffect_Rect::Render()
{   
	if (FAILED(Bind_ShaderResources()))
        return;
	
    m_pShaderCom->Begin(m_iShaderPass);
	
    m_pVIBufferCom->Bind_Resources();
	
    m_pVIBufferCom->Render();
}

void CEffect_Rect::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	EFFECT_INFO* pDesc = static_cast<EFFECT_INFO*>(pArg);

	m_isActivate = pDesc->IsActive;

	//기본 초기화
	m_vLifeTime.x = 0.f;
	m_fSweep = 0.f;
	m_fPhase = 0.f;

	if (m_isActivate && !m_IsRoot)
	{
		//뼈에 안붙을 얘면 프리팹이 계산해서 던져준 월드매트릭스 그대로 사용해도 됨.
		Default_Transform(WorldMatrix);
	}
	else if (m_isActivate && m_IsRoot)
	{
		//뼈에 붙을 얘면 프리팹이 넘겨준 정보 토대로 업데이트에서 갱신해주는 작업이 필요.
		m_pBoneMatrixPtr = pDesc->pBoneMatrixPtr;
		m_pObjectMatrixPtr = pDesc->pObjectMatrixPtr;
		m_OffsetMatrix = pDesc->OffsetMatrix;
	}
}

void CEffect_Rect::Default_Transform(_fmatrix WorldMatrix)
{
    _vector vPos =  XMVectorSetW(WorldMatrix.r[3], 1.f);

    m_pTransformCom->Set_State(STATE::POSITION, vPos);
}

void CEffect_Rect::Sprite_Update(_float fTimeDelta)
{
	m_fPhase += fTimeDelta * m_fSweepSpeed;
}

void CEffect_Rect::Update_Root_Transform()
{
	if (m_pBoneMatrixPtr == nullptr)
		return;

	_matrix OffsetMatrix = m_OffsetMatrix;

	_float4x4 ObjectMatrix = *m_pObjectMatrixPtr;
	_float4x4 BoneMatrix = *m_pBoneMatrixPtr;

	_matrix SpawnMatrix = XMLoadFloat4x4(&BoneMatrix) * XMLoadFloat4x4(&ObjectMatrix);

	_vector vScale = {};
	_vector vPos = {};
	_vector vRot = {};
	XMMatrixDecompose(&vScale, &vRot, &vPos, SpawnMatrix);

	//렉트는 뼈 회전 안먹어도 될거 같음.
	_matrix OffsetSpawnMatrix =/* XMMatrixRotationQuaternion(vRot) * */XMMatrixTranslationFromVector(vPos);

	XMStoreFloat4x4(&m_ComBindMatrix,
		m_pTransformCom->Get_WorldMatrix() *
		OffsetMatrix * OffsetSpawnMatrix);
}


HRESULT CEffect_Rect::Ready_Components(FXRECT_DESC& Desc)
{
    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxFXRect"),
        TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT),TEXT("Prototype_Componnent_VIBuffer_FXRect"),
        TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
        return E_FAIL;

    if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
        TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Rect::Bind_ShaderResources()
{
	if (!m_IsRoot)
	{
		if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_ComBindMatrix)))
			return E_FAIL;
	}

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture", 0)))
        return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_Sweep", &m_fSweep, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_Soft", &m_fSoft, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_fXSize", &m_fXSize, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_fYSize", &m_fYSize, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_MaskFlag", &m_iMaskFlag, sizeof(_int))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_ColorFlag", &m_iColorFlag, sizeof(_int))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Value("g_vLifeTime", &m_vLifeTime, sizeof(_float2))))
		return E_FAIL;

	if (m_IsSprite)
	{
		if (FAILED(m_pShaderCom->Bind_Value("g_iRow", &m_iRow, sizeof(_int))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Value("g_iCol", &m_iCol, sizeof(_int))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Value("g_fPhase", &m_fPhase, sizeof(_float))))
			return E_FAIL;
	}

    return S_OK;
}

CEffect_Rect* CEffect_Rect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEffect_Rect* pInstance = new CEffect_Rect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEffect_Rect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEffect_Rect::Clone(void* pArg)
{
    CEffect_Rect* pInstance = new CEffect_Rect(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Created : CEffect_Rect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEffect_Rect::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
}
