#include "EditorPch.h"
#include "EditDummy_Wolf.h"

#include "SpringCamera_Edit.h"

CEditDummy_Wolf::CEditDummy_Wolf(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEditDummy { pDevice, pContext }
{
}

CEditDummy_Wolf::CEditDummy_Wolf(const CEditDummy_Wolf& Prototype)
    : CEditDummy { Prototype }
{
}

HRESULT CEditDummy_Wolf::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        CRASH("Failed Init_Prototype Dummy_Wolf");

    return S_OK;
}

HRESULT CEditDummy_Wolf::Initialize_Clone(void* pArg)
{
    if (nullptr == pArg)
        CRASH("Failed to Cloned : Dummy_Wolf");

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;

    DUMMY_WOLF_DESC* pDesc = static_cast<DUMMY_WOLF_DESC*>( pArg );

    if (FAILED(Ready_Components(pDesc->PreTransformMatrix)))
        return E_FAIL;

	if (FAILED(Ready_Camera()))
		CRASH("Camera");

    return S_OK;
}

void CEditDummy_Wolf::Priority_Update(_float fTimeDelta)
{
}

void CEditDummy_Wolf::Update(_float fTimeDelta)
{
#ifdef _DEBUG
    if (m_pGameInstance->Get_DIKeyState(DIK_LSHIFT) == KEYSTATE::PRESS)
        m_pTransformCom->Change_Speed(200.f);
    else
        m_pTransformCom->Change_Speed(20.f);
#endif // DEBUG

	

	// Spring Test
	if (m_pGameInstance->Get_DIKeyState(DIK_T) == KEYSTATE::DOWN)
		m_pSpringCamera->Use_Spring(50.f, 0.5f);

	Key_Move(fTimeDelta);

	m_pSpringCamera->Update_Target(m_pTransformCom->Get_State(STATE::POSITION), 5.f);
}

void CEditDummy_Wolf::Late_Update(_float fTimeDelta)
{
    m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
    m_pGameInstance->Add_Render_Object(RENDERGROUP::SHADOW, this);
}

void CEditDummy_Wolf::Render()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

    _uint iNumMesh = m_pModelCom->Get_NumMesh();
    for (_uint i = 0; i < iNumMesh; ++i)
    {
        m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

        _bool HasNormal = { false };
        if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
            HasNormal = true;

        m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));

        m_pShaderCom->Begin(0);
        m_pModelCom->Render(i);
    }
}

void CEditDummy_Wolf::Render_Shadow()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");

    m_pGameInstance->Bind_CSM_Resources(m_pShaderCom, "g_ShadowViewMatrix", "g_ShadowProjMatrix");

    _uint iNumMesh = m_pModelCom->Get_NumMesh();

    for (_uint i = 0; i < iNumMesh; ++i)
    {
        m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
        m_pShaderCom->Begin(5);

        m_pModelCom->Render(i);
    }
}

void CEditDummy_Wolf::Key_Move(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_W) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Straight(fTimeDelta);
	if (m_pGameInstance->Get_DIKeyState(DIK_S) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Backward(fTimeDelta);
	if (m_pGameInstance->Get_DIKeyState(DIK_A) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Left(fTimeDelta);
	if (m_pGameInstance->Get_DIKeyState(DIK_D) == KEYSTATE::PRESS)
		m_pTransformCom->Go_Right(fTimeDelta);
}

HRESULT CEditDummy_Wolf::Ready_Camera()
{
	m_pSpringCamera = CSpringCamera_Edit::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pSpringCamera);

	CSpringCamera_Edit::CAMERA_DESC CameraDesc = {};
	CameraDesc.fSpeedPerSec = 100.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CameraDesc.fFovy = XMConvertToRadians(60.f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 5000.f;
	CameraDesc.vEye = _float4(0.f, 200.f, -150.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 200.f, 1.f);
	CameraDesc.fMouseSensor = 0.004f;

	m_pSpringCamera->Initialize_Clone(&CameraDesc);

	m_pGameInstance->Add_Camera(ENUM_CLASS(LEVEL::CAMERA), TEXT("Camera_Spring"), m_pSpringCamera);
	Safe_AddRef(m_pSpringCamera);

	m_pGameInstance->Change_MainCamera(ENUM_CLASS(LEVEL::CAMERA), TEXT("Camera_Spring"));

	return S_OK;
}

HRESULT CEditDummy_Wolf::Ready_Components(_fmatrix PreTransformMatrix)
{
    m_pModelCom = CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat");
    ASSERT_CRASH(m_pModelCom);

    m_pShaderCom = CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements);
    ASSERT_CRASH(m_pShaderCom);

    return S_OK;
}

CEditDummy_Wolf* CEditDummy_Wolf::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEditDummy_Wolf* pInstance = new CEditDummy_Wolf(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CEditDummy_Wolf");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CGameObject* CEditDummy_Wolf::Clone(void* pArg)
{
    CEditDummy_Wolf* pInstance = new CEditDummy_Wolf(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Cloned : CEditDummy_Wolf");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CEditDummy_Wolf::Free()
{
    __super::Free();

    Safe_Release(m_pModelCom);
    Safe_Release(m_pShaderCom);

	Safe_Release(m_pSpringCamera);
}
