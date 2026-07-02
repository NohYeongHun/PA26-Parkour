#include"ClientPch.h" 
#include "MapObject_Water.h"

CMapObject_Water::CMapObject_Water(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CMapObject_Water::CMapObject_Water(const CMapObject_Water& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CMapObject_Water::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapObject_Water::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	Ready_Component(pArg);
	//m_iNumLOD = m_pModelCom->Get_LastLODIndex();
	//m_pGameInstance->Add_To_OctoTree(this, m_pBoundingBox);

	//Sync_Sectors();

	// Env Map Bake
	//m_pGameInstance->Add_EnvMap_StaticObject(this);

	//if (FAILED(m_pGameInstance->Add_Render_ShadowMapObject(this)))
	//	return E_FAIL;

	m_fAnimTime = 0.06f;

	return S_OK;
}

void CMapObject_Water::Priority_Update(_float fTimeDelta)
{
}

void CMapObject_Water::Update(_float fTimeDelta)
{
}

void CMapObject_Water::Late_Update(_float fTimeDelta)
{
	_uint iLevel = m_pGameInstance->Get_CurrentLevel();
	if (iLevel == ENUM_CLASS(LEVEL::LOGO))
		m_pGameInstance->Add_Render_Object(RENDERGROUP::WATER, this);
	else if (iLevel == ENUM_CLASS(LEVEL::HEAVEN))
	{
		m_fTime += fTimeDelta;
		if (m_fTime > m_fAnimTime)
		{
			m_iTextureIndex += 1;
			m_fTime -= m_fAnimTime;
		}

		if (m_iTextureIndex >= 25)
			m_iTextureIndex = 0;

		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONLIGHT, this);
	}
}

void CMapObject_Water::Render()
{
	_uint m_iLODIndex = 0;
	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	_bool HasNormal = { true };   
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
	
	if (FAILED(m_pShaderCom->Bind_Value("g_fTime", &m_fTime, sizeof(_float))))
		CRASH("Failed to Bind fTime");

	// Color

	_uint iLevel = m_pGameInstance->Get_CurrentLevel();
	_uint iPassIndex = {};

	if (iLevel == ENUM_CLASS(LEVEL::LOGO))
		iPassIndex = 4;
	else if (iLevel == ENUM_CLASS(LEVEL::HEAVEN))
		iPassIndex = 5;

	m_pModelCom->Bind_Buffer(m_pContext, m_iLODIndex);
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (m_pModelCom->Is_Overed(m_iLODIndex, i))
			return;
		if (iLevel == ENUM_CLASS(LEVEL::LOGO))
		{
			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", m_iLODIndex, i, TEXTURETYPE::MASK)))
			{
				m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
				HasMask = false;
			}
		}
		else if (iLevel == ENUM_CLASS(LEVEL::HEAVEN))
		{
			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskSprite", m_iLODIndex, i, TEXTURETYPE::MASK, m_iTextureIndex)))
			{
				m_pShaderCom->Bind_Texture("g_MaskSprite", nullptr);
				HasMask = false;
			}
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

		m_pShaderCom->Begin(iPassIndex); // TEST
//		m_pShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(m_iLODIndex, i);
	}
}


void CMapObject_Water::Render_Shadow()
{
}

void CMapObject_Water::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
}

BoundingBox* CMapObject_Water::Get_BoundingBox()
{
	return m_pBoundingBox;
}

void CMapObject_Water::Set_RenderTime(_uint iLODIndex, _float m_fTotalPlayTime)
{
	m_pModelCom->Set_RenderTime(iLODIndex, m_fTotalPlayTime);
}

void CMapObject_Water::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_iShaderPassIndex = pDesc->iShaderPassIndex;


	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_NonAnimMesh_Water"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return;

	// ShadowShader
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_NonAnimMesh_Water"),
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

CMapObject_Water* CMapObject_Water::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Water* pInstance = new CMapObject_Water(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Water::Clone(void* pArg)
{
	CMapObject_Water* pClone = new CMapObject_Water(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject_Water::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pShadowShaderCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pModelCom);
}