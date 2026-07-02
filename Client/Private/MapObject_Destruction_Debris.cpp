#include"ClientPch.h"
#include "MapObject_Destruction_Debris.h"

CMapObject_Destruction_Debris::CMapObject_Destruction_Debris(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice, pContext)
{
}

CMapObject_Destruction_Debris::CMapObject_Destruction_Debris(const CMapObject_Destruction_Debris& Prototype)
	:CStaticObject(Prototype)
{
}


HRESULT CMapObject_Destruction_Debris::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Destruction_Debris::Initialize_Clone(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

	if (FAILED(Ready_Component(pArg)))
		return E_FAIL;

	m_iNumLOD = 0;

	m_iShaderPassIndex = pDesc->iShaderPassIndex;
	m_pRigidbodyCom->IsActivate(false);
	m_isActivate = false;
	m_iLODIndex = 0;
	return S_OK;
}

void CMapObject_Destruction_Debris::Priority_Update(_float fTimeDelta)
{

	if (m_IsTriggered)
	{
		//위치가 다시 안돌아옴. ->SetPosition 안먹음.
		m_pRigidbodyCom->IsActivate(true);
		m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&m_FixedPos));
		m_pRigidbodyCom->Set_Transform(m_pTransformCom->Get_WorldMatrix());
		m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
		m_pRigidbodyCom->Impulse(m_vImpulse);
		m_IsTriggered = false;
	}
}

void CMapObject_Destruction_Debris::Update(_float fTimeDelta)
{
	m_fTimeDelta += fTimeDelta;

	if (m_fTimeDelta >= 4.f)
	{
		m_pRigidbodyCom->Set_Position(m_pTransformCom->Get_State(STATE::POSITION) - XMVectorSet(0.f, 1000.f, 0.f, 0.f));
		m_pRigidbodyCom->IsActivate(false);
		m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
		m_isActivate = false;
	}
}

void CMapObject_Destruction_Debris::Late_Update(_float fTimeDelta)
{
	m_pRigidbodyCom->Sync_Rigidbody(m_pTransformCom);
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONSTATIC, this);
}

void CMapObject_Destruction_Debris::Render()
{
	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pModelCom->Bind_Buffer(m_pContext, m_iLODIndex);
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", m_iLODIndex, i, TEXTURETYPE::MASK)))
		{
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
			HasMask = false;
		}

		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

		m_pShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(m_iLODIndex, i);
	}
}

void CMapObject_Destruction_Debris::Render_Shadow()
{
}

HRESULT CMapObject_Destruction_Debris::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};
	MultiByteToWideChar(CP_ACP, 0, pDesc->ModelName, -1, Name, strlen(pDesc->ModelName));
	lstrcat(Model, Name);

	_wstring WModelName = Model;

	if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), WModelName,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

	// DeferredShader
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Dynamic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);
	RigidbodyDesc.vExtent = _float3(1.f, 1.f, 1.f);

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

	return S_OK;
}

void CMapObject_Destruction_Debris::Bind_Resources()
{
}

void CMapObject_Destruction_Debris::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_IsTriggered = true;
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	RESET_DESC* pDesc = static_cast<RESET_DESC*>(pArg);
	XMStoreFloat4x4(&m_FixedPos, WorldMatrix);
	m_vImpulse = pDesc->vImpulse;
	
	m_fTimeDelta = 0.f;
}

CMapObject_Destruction_Debris* CMapObject_Destruction_Debris::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Destruction_Debris* pInstance = new CMapObject_Destruction_Debris(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Map_Object_Destruction_Piece");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Destruction_Debris::Clone(void* pArg)
{
	CMapObject_Destruction_Debris* pClone = new CMapObject_Destruction_Debris(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Map_Object_Destruction_Piece (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject_Destruction_Debris::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pModelCom);
}
