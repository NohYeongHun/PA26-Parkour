#include "ClientPch.h"
#include "MapObject.h"
#include "GameSystem.h"

CMapObject::CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CStaticObject{ pDevice, pContext }
{
}

CMapObject::CMapObject(const CMapObject& Prototype)
	: CStaticObject{ Prototype }
	, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapObject::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	Ready_Component(pArg);
	m_iNumLOD = m_pModelCom->Get_LastLODIndex();
	m_pGameInstance->Add_To_OctoTree(this, m_pBoundingBox);

	Sync_Sectors();

	// Env Map Bake


	return S_OK;
}

void CMapObject::Priority_Update(_float fTimeDelta)
{
}

void CMapObject::Update(_float fTimeDelta)
{

}

void CMapObject::Late_Update(_float fTimeDelta)
{
}

void CMapObject::Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex)
{
	if (!m_IsRender)
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
	m_pShaderCom->Bind_Value("g_CamPos", m_pGameInstance->Get_CamPos(), sizeof(_float4), pEffect);

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

void CMapObject::Render_Shadow()
{
	_uint iLODIndex = 0;
	if (iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	m_pTransformCom->Bind_Matrix(m_pShadowShaderCom, "g_WorldMatrix");

	for (auto& iSector : m_Sectors)
	{
		m_pGameInstance->Bind_ShadowMap_Resources_StaticObject(m_pShadowShaderCom, "g_ShadowMapViewMatrix", "g_ShadowMapProjMatrix", iSector);

		_uint iLayer = m_pGameInstance->Get_ShadowMapLayer(iSector);
		if (FAILED(m_pShadowShaderCom->Bind_Value("g_iShadowMapLayer", &iLayer, sizeof(_uint))))
			CRASH("Failed Bind ShadowMapLayer");

		_uint iNumMesh = m_pModelCom->Get_NumMesh(0);

		m_pModelCom->Bind_Buffer(m_pContext, iLODIndex);
		for (_uint i = 0; i < iNumMesh; ++i)
		{
			m_pShadowShaderCom->Begin(8);

			m_pModelCom->Render(iLODIndex, i);
		}
	}
}

void CMapObject::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
	_uint iLODIndex = 0;
	if (iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(iLODIndex);

	m_pTransformCom->Bind_Matrix(m_pShadowShaderCom, "g_WorldMatrix");
	m_pShadowShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix);
	m_pShadowShaderCom->Bind_Matrix("g_ProjMatrix", &ProjMatrix);

	m_pModelCom->Bind_Buffer(m_pContext, iLODIndex);
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (m_pModelCom->Is_Overed(iLODIndex, i))
			return;
		if (FAILED(m_pModelCom->Bind_Materials(m_pShadowShaderCom, "g_MaskTexture", iLODIndex, i, TEXTURETYPE::MASK)))
		{
			m_pShadowShaderCom->Bind_Texture("g_MaskTexture", nullptr);
			HasMask = false;
		}

		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShadowShaderCom, "g_DiffuseTexture", iLODIndex, i, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShadowShaderCom, "g_NormalTexture", iLODIndex, i, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShadowShaderCom, "g_DiffuseTexture", iLODIndex, i, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShadowShaderCom, "g_NormalTexture", iLODIndex, i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}
		m_pShadowShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShadowShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

		m_pShadowShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(iLODIndex, i);
	}
}

BoundingBox* CMapObject::Get_BoundingBox()
{
	return m_pBoundingBox;
}

void CMapObject::Set_RenderTime(_uint iLODIndex, _float m_fTotalPlayTime)
{
	m_pModelCom->Set_RenderTime(iLODIndex, m_fTotalPlayTime);
}

void CMapObject::Ready_Component(void* pArg)
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


	CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::MESH;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Static;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	RigidbodyDesc.pModel = m_pModelCom;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

	m_pGameInstance->Add_EnvMap_StaticObject(this);

	if (FAILED(m_pGameInstance->Add_Render_ShadowMapObject(this)))
		CRASH("Failed");
}

CMapObject* CMapObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject* pInstance = new CMapObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject::Clone(void* pArg)
{
	CMapObject* pClone = new CMapObject(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject::Free()
{
	__super::Free();
	Safe_Release(m_pGameSystem);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pShadowShaderCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pModelCom);
}