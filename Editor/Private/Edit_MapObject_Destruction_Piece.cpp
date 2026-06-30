#include"EditorPch.h"
#include "Edit_MapObject_Destruction_Piece.h"

CEdit_MapObject_Destruction_Piece::CEdit_MapObject_Destruction_Piece(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice, pContext)
{
}

CEdit_MapObject_Destruction_Piece::CEdit_MapObject_Destruction_Piece(const CEdit_MapObject_Destruction_Piece& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CEdit_MapObject_Destruction_Piece::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEdit_MapObject_Destruction_Piece::Initialize_Clone(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	strcpy_s(m_ModelName, pDesc->ModelName);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

	if (FAILED(Ready_Component(pArg)))
		return E_FAIL;

	m_iNumLOD = m_pModelCom->Get_NumMesh(0);

#ifdef _DEBUG
	Sync_BoundingBox(m_pModelCom->Get_BoundingBox(), m_pTransformCom->Get_WorldMatrix());
#endif // _DEBUG

	

	_vector vScale, vRotation, vTranslation;

	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, m_pTransformCom->Get_WorldMatrix());

	XMStoreFloat3(&m_vScale, vScale);
	XMStoreFloat3(&m_vTranslation, vTranslation);
	m_vNewScale = m_vScale;
	m_vRotation = m_vNewRotation = _float3(0.f, 0.f, 0.f);
	m_vNewTranslation = m_vTranslation;
	m_iShaderPassIndex = pDesc->iShaderPassIndex;
	m_eObjectType = pDesc->eObjectType;


	//XMStoreFloat3(&vImpluse, XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), 0.f) * -100.f);
	//m_pRigidbodyCom->Impulse(pDesc->vImpulse);
	return S_OK;
}

void CEdit_MapObject_Destruction_Piece::Priority_Update(_float fTimeDelta)
{
	if (m_IsTriggered)
	{
		m_pRigidbodyCom->IsActivate(true);
		m_pRigidbodyCom->Set_Transform(m_pTransformCom->Get_WorldMatrix());
		m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::MAP));
		m_pRigidbodyCom->Impulse(m_vImpulse);
		m_IsTriggered = false;
	}
}

void CEdit_MapObject_Destruction_Piece::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
	{
		m_isActivate = false;
		m_pRigidbodyCom->IsActivate(false);
	}

	m_fTimeDelta += fTimeDelta;
	if (m_fTimeDelta >= 3.f)
	{
		m_isActivate = false;
		m_pRigidbodyCom->IsActivate(false);
	}

}

void CEdit_MapObject_Destruction_Piece::Late_Update(_float fTimeDelta)
{
	m_pRigidbodyCom->Sync_Rigidbody(m_pTransformCom);
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_MapObject_Destruction_Piece::Render()
{
	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pModelCom->Bind_Buffer(m_pContext, m_iLODIndex);
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (m_pModelCom->Is_Overed(m_iLODIndex, i))
			return;

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

void CEdit_MapObject_Destruction_Piece::Render_Shadow()
{
}

void CEdit_MapObject_Destruction_Piece::Set_ImGuiOption()
{
}

HRESULT CEdit_MapObject_Destruction_Piece::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
	m_iLevel = pDesc->iLevel;

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};
	MultiByteToWideChar(CP_ACP, 0, m_ModelName, -1, Name, strlen(m_ModelName));
	lstrcat(Model, Name);
	if (FAILED(Add_Component(pDesc->iLevel, Model,
		StringToWString(m_ModelName), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

#ifdef _DEBUG
	m_pModelCom->Ready_BoundingBox();
#endif // _DEBUG

	
	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;


	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	//RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Dynamic;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);

#ifdef _DEBUG
	RigidbodyDesc.vExtent = m_pModelCom->Get_BoundingBox()->Extents;
#endif // _DEBUG

	

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);


	//CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	////CRigidbody::CONVEXHULLBODY_DESC RigidbodyDesc{};
	////RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	//XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	//RigidbodyDesc.eShape = SHAPE::BOX;
	////RigidbodyDesc.eShape = SHAPE::CONVEXHULL;
	////RigidbodyDesc.pModel = m_pModelCom;
	////RigidbodyDesc.eBodyType = BODYTYPE::
	//XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//RigidbodyDesc.eType = EMotionType::Dynamic;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	//RigidbodyDesc.vExtent = _float3(1.f, 1.f, 1.f);
	////RigidbodyDesc.vExtent = m_pModelCom->Get_BoundingBox()->Extents;

	//Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
	//	TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);



    return S_OK;
}

void CEdit_MapObject_Destruction_Piece::Bind_Resources()
{
}

void CEdit_MapObject_Destruction_Piece::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	m_isActivate = true;
	m_IsTriggered = true;
	m_pTransformCom->Set_WorldMatrix(WorldMatrix);
	RESET_DESC* pDesc = static_cast<RESET_DESC*>(pArg);
	m_vImpulse = pDesc->vImpulse;
	
	m_fTimeDelta = 0.f;
}

CEdit_MapObject_Destruction_Piece* CEdit_MapObject_Destruction_Piece::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_MapObject_Destruction_Piece* pInstance = new CEdit_MapObject_Destruction_Piece(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Map_Object_Destruction_Piece");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_MapObject_Destruction_Piece::Clone(void* pArg)
{
	CEdit_MapObject_Destruction_Piece* pClone = new CEdit_MapObject_Destruction_Piece(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Map_Object_Destruction_Piece (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CEdit_MapObject_Destruction_Piece::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pModelCom);
}
