#include "EditorPch.h"
#include "TestVA.h"

CTestVA::CTestVA(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTestVA::CTestVA(const CTestVA& Prototype)
	: CGameObject{ Prototype },
	m_tDesc{ Prototype.m_tDesc }
{
}

HRESULT CTestVA::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTestVA::Initialize_Clone(void* pArg)
{
	VA_DESC* pDesc = static_cast<VA_DESC*>(pArg);

	m_tDesc = *pDesc;

	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("Transform");

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_isActivate = false;

	m_fAnimationDuration = m_pVAMesh->Get_MaxFrame(0);

	return S_OK;
}

void CTestVA::Priority_Update(_float fTimeDelta)
{
}

void CTestVA::Update(_float fTimeDelta)
{
	if (m_fTrackPosition > m_fAnimationDuration)
		m_isActivate = false;

	m_fTrackPosition += (fTimeDelta * m_tDesc.fAnimSpeed);
}

void CTestVA::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::DYNAMIC, this)))
		return;
}

void CTestVA::Render()
{
	Bind_Resource();

	m_pShaderCom->Begin(0);

	m_pVAMesh->Bind_Resources();
	m_pVAMesh->Render();
}

void CTestVA::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	EFFECT_INFO* pDesc = static_cast<EFFECT_INFO*>(pArg);

	m_isActivate = pDesc->IsActive;

	m_fTrackPosition = 0.f;

	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
}

void CTestVA::Bind_Resource()
{
	if (FAILED(m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix")))
		CRASH("Failed to Bind WolrdMatrix");

	if (FAILED(m_pColorTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture", 0)))
		CRASH("Failed to Bind DiffuseTexture");

	if(FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW))))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ))))
		CRASH("Failed to Bind ProjMatrix");

	if(FAILED(m_pVAMesh->Bind_VAT(m_pShaderCom, "g_VatTexture", 0)))
		CRASH("Failed to Bind VatTexture");
	
	if (FAILED(m_pShaderCom->Bind_Value("g_fTrackPosition", &m_fTrackPosition, sizeof(_float))))
		CRASH("Failed to Bind TrackPosition");

	if (FAILED(m_pShaderCom->Bind_Value("g_fAnimationDuration", &m_fAnimationDuration, sizeof(_float))))
		CRASH("Failed to Bind AnimationDuration");

	if (FAILED(m_pShaderCom->Bind_Value("g_fMovementScale", &m_tDesc.fMovementScale, sizeof(_float))))
		CRASH("Failed to Bind AnimationDuration");
}

HRESULT CTestVA::Ready_Components()
{
	if(FAILED(CGameObject::Add_Component(m_pGameInstance->Get_CurrentLevel(), m_tDesc.strTextureTag, TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr)))
		ASSERT_CRASH(m_pTextureCom);

	if (FAILED(CGameObject::Add_Component(m_pGameInstance->Get_CurrentLevel(), m_tDesc.strColorTextureTag, TEXT("Com_ColorTexture"), reinterpret_cast<CComponent**>(&m_pColorTextureCom), nullptr)))
		ASSERT_CRASH(m_pColorTextureCom);

	if (FAILED(CGameObject::Add_Component(m_pGameInstance->Get_CurrentLevel(), m_tDesc.strMeshTag, TEXT("Com_Mesh"), reinterpret_cast<CComponent**>(&m_pVAMesh), nullptr)))
		ASSERT_CRASH(m_pVAMesh);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VAMesh"), TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		ASSERT_CRASH(m_pShaderCom);

	return S_OK;
}

CTestVA* CTestVA::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTestVA* pInstance = new CTestVA(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
		CRASH("EFfect_VA");

	return pInstance;
}

CGameObject* CTestVA::Clone(void* pArg)
{
	CTestVA* pClone = new CTestVA(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("EFfect_VA");

	return pClone;
}

void CTestVA::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pColorTextureCom);
	Safe_Release(m_pVAMesh);
}
