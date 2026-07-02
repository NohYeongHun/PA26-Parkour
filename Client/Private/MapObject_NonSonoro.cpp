#include "ClientPch.h"
#include "MapObject_NonSonoro.h"
#include"GameSystem.h"

CMapObject_NonSonoro::CMapObject_NonSonoro(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CStaticObject{ pDevice, pContext }
{
}

CMapObject_NonSonoro::CMapObject_NonSonoro(const CMapObject_NonSonoro& Prototype)
	: CStaticObject{ Prototype },m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_NonSonoro::Initialize_Prototype()
{
	//현재 Sonoro가 침식된 놈들, Sonoro_Floor가 침식된 바닥.
	//NonSonoro가 침식안된 놈들. 바닥 포함.
	//사실 반대가 되어야함. 수정핤것.
	return S_OK;
}

HRESULT CMapObject_NonSonoro::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	Ready_Component(pArg);
	m_iNumLOD = m_pModelCom->Get_LastLODIndex();
	//Sync_BoundingBox(m_pModelComArray[0]->Get_BoundingBox(), m_pTransformCom->Get_WorldMatrix());
	//m_pGameInstance->Add_To_OctoTree(this, m_pModelComArray[0]->Get_BoundingBox());
	m_pGameInstance->Add_To_OctoTree(this, m_pBoundingBox);
	Sync_Sectors();

	/*if (FAILED(m_pGameInstance->Add_Render_ShadowMapObject(this)))
		return E_FAIL;*/
	XMStoreFloat4x4(&m_DefaultMatrix, m_pTransformCom->Get_WorldMatrix());

	m_eObjectType = pDesc->eObjectType;
	m_IsRender = m_pGameSystem->Add_To_Management(m_eObjectType, this, &m_SonoroMode);
	return S_OK;

}

void CMapObject_NonSonoro::Priority_Update(_float fTimeDelta)
{
	//m_pRigidbodyCom->IsActivate(!*m_SonoroMode);
}

void CMapObject_NonSonoro::Update(_float fTimeDelta)
{

}

void CMapObject_NonSonoro::Late_Update(_float fTimeDelta)
{
	//m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMapObject_NonSonoro::Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex)
{
	if ((*m_IsRender))
		return;

	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	if (m_pModelCom->Get_MeshState(m_iLODIndex) != LOADSTATE::LOADED)
	{
		if (m_pModelCom->Get_MeshState(m_iLODIndex) == LOADSTATE::NOTLOADED)
			m_pModelCom->Request_LOD(m_iLODIndex);

		m_pGameInstance->Add_Render_StaticObject(this, m_iLODIndex = m_pModelCom->Get_ReadyLOD());
		return;
	}
	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	ID3DX11Effect* pEffect = m_pGameInstance->Get_Shader_Effect(TEXT("Shader_Map"), iIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix", pEffect);
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW), pEffect);
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ), pEffect);

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (m_pModelCom->Is_Overed(m_iLODIndex, i))
			return;
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", m_iLODIndex, i, TEXTURETYPE::MASK, pEffect)))
		{
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr, pEffect);
			HasMask = false;
		}

		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE, pEffect);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL, pEffect)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE, 0, pEffect);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL, 0, pEffect)))
				HasNormal = false;
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool), pEffect);
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool), pEffect);

		m_pShaderCom->Begin(m_iShaderPassIndex, pDeferredContext, pEffect);

		m_pModelCom->Render(m_iLODIndex, i, pDeferredContext);
	}
}

void CMapObject_NonSonoro::Render_Shadow()
{
	m_pTransformCom->Bind_Matrix(m_pShadowShaderCom, "g_WorldMatrix");

	for (auto& iSector : m_Sectors)
	{
		m_pGameInstance->Bind_ShadowMap_Resources_StaticObject(m_pShadowShaderCom, "g_ShadowMapViewMatrix", "g_ShadowMapProjMatrix", iSector);

		_uint iLayer = m_pGameInstance->Get_ShadowMapLayer(iSector);
		if (FAILED(m_pShadowShaderCom->Bind_Value("g_iShadowMapLayer", &iLayer, sizeof(_uint))))
			CRASH("Failed Bind ShadowMapLayer");

		_uint iNumMesh = m_pModelComArray[m_iLODIndex]->Get_NumMesh();

		for (_uint i = 0; i < iNumMesh; ++i)
		{
			m_pShadowShaderCom->Begin(8);

			m_pModelComArray[m_iLODIndex]->Render(i);
		}
	}
}

BoundingBox* CMapObject_NonSonoro::Get_BoundingBox()
{
	return m_pBoundingBox;
}

void CMapObject_NonSonoro::Compute_DelayTime(_float4 vCamPos)
{
	if (m_eObjectType == OBJECTTYPE::NONSONORA_FLOOR)
		return;

	_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), 0.f) - XMVectorSetY(XMLoadFloat4(&vCamPos), 0.f)));

	float minDistance = 0.0f;
	float maxDistance = 600.0f;

	float maxDelay = 4.7f;
	float minDelay = 0.0f;

	float t = (fDistance - minDistance) / (maxDistance - minDistance);

	m_fDlayTime = maxDelay + (minDelay - maxDelay) * t;
}

void CMapObject_NonSonoro::Turn_Sonoro(_fvector vUpSpeed,_float fTriggerdTime)
{
	if (m_eObjectType == OBJECTTYPE::NONSONORA_FLOOR)
		return;

	if (m_fDlayTime <= fTriggerdTime)
	{
		m_pTransformCom->Set_State(STATE::POSITION, m_pTransformCom->Get_State(STATE::POSITION) + vUpSpeed);


	}
}

void CMapObject_NonSonoro::ReturnPos()
{
	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&m_DefaultMatrix));
	m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
	
}

void CMapObject_NonSonoro::Change_Collision_Layer(_bool SonoroMode)
{
	m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::MAP));
}

void CMapObject_NonSonoro::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_iShaderPassIndex = pDesc->iShaderPassIndex;

	// DeferredShader
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_DeferredShader_Map"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");
	// ShadowShader
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_ShadowShader"), reinterpret_cast<CComponent**>(&m_pShadowShaderCom), nullptr)))
		CRASH("FAILED");

	m_pBoundingBox = new BoundingBox(pDesc->vBoundingPos, pDesc->vBoundingExtends);
	if (!m_pBoundingBox)
		CRASH("Failed");

	_wstring ModelName = Model;
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();
	ModelName.pop_back();
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), ModelName,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");
	if (pDesc->eObjectType != OBJECTTYPE::NONRIGID)
	{

		CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
		RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
		XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
		RigidbodyDesc.eShape = SHAPE::MESH;
		XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
		RigidbodyDesc.eType = EMotionType::Static;
		RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
		RigidbodyDesc.pModel = m_pModelCom;
		//RigidbodyDesc.pModel = m_pModelComArray[0];

		//CRigidbody::BOXBODY_DESC RigidbodyDesc = {};
		//RigidbodyDesc.vPos = pDesc->vBoundingPos;
		//RigidbodyDesc.eShape = SHAPE::BOX;
		//RigidbodyDesc.eType = EMotionType::Static;
		//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
		//RigidbodyDesc.vExtent = pDesc->vBoundingExtends;

		Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
			TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);
	}
}

CMapObject_NonSonoro* CMapObject_NonSonoro::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_NonSonoro* pInstance = new CMapObject_NonSonoro(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject_NonSonoro");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_NonSonoro::Clone(void* pArg)
{
	CMapObject_NonSonoro* pClone = new CMapObject_NonSonoro(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject_NonSonoro (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject_NonSonoro::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pShadowShaderCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pModelCom);

}
