#include"ClientPch.h"
#include "MapObject_Instance.h"
#include"GameSystem.h"

CMapObject_Instance::CMapObject_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CStaticObject{ pDevice, pContext }

{
}

CMapObject_Instance::CMapObject_Instance(const CMapObject_Instance& Prototype)
	: CStaticObject{ Prototype }, m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_Instance::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Instance::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	Ready_Component(pArg);
	m_iLODIndex = 0;
	//나중에 풀때기 흔드는 거 하려면 업데이트가 필요해서 임시 조치.
	//m_pGameInstance->Add_To_OctoTree(this, m_pBoundingBox);
	//AddRef();
	//Sync_Sectors();
	_bool* Test;
	if (m_eInstanceType != INSTANCETYPE::DEFAULT)
		m_bSonoroMode = m_pGameSystem->Add_To_Management(m_eInstanceType, this, &Test);

	if (m_eInstanceType == INSTANCETYPE::SONORO)
		m_TypeMode = true;
	else if (m_eInstanceType == INSTANCETYPE::NONSONORO)
		m_TypeMode = false;

	return S_OK;
}

void CMapObject_Instance::Priority_Update(_float fTimeDelta)
{
}

void CMapObject_Instance::Update(_float fTimeDelta)
{
	m_fTotalTime += fTimeDelta;
	if (m_fTotalTime >= XM_2PI)
		m_fTotalTime -= XM_2PI;
}

void CMapObject_Instance::Late_Update(_float fTimeDelta)
{
	_bool IsRender = true;
	if (!(m_eInstanceType == INSTANCETYPE::DEFAULT))
		if (m_TypeMode != *m_bSonoroMode)
			IsRender = false;
	
	if (IsRender && m_pGameInstance->IsIn_WorldSpace(m_pBoundingBox))
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CMapObject_Instance::Render()
{
	_uint iLODIndex = m_iLODIndex;
	if (m_iNumLOD <= iLODIndex)
		iLODIndex = m_iNumLOD;

	//m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMesh; ++i)
	{
		_bool HasNormal = { true };
		_bool HasMask = { true };

		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
		{
			//m_pModelCom->Clear_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK);
			HasMask = false;
		}
		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}

		//m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		//m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_vDiffuseColor", &m_vDiffuseColor, sizeof(_float4));
		m_pShaderCom->Bind_Value("g_fRaidan", &m_fTotalTime, sizeof(_float));
		m_pShaderCom->Begin(m_iShaderPassIndex);

		m_pModelCom->Render(i);

		// Clear Pre Resource
		//m_pModelCom->Clear_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);
		//m_pModelCom->Clear_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL);
		//m_pModelCom->Clear_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK);
	}
}

void CMapObject_Instance::Render_Shadow()
{
}

void CMapObject_Instance::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_wstring ProtoName = TEXT("Prototype_Component_Model_Instance_");
	ProtoName += StringToWString(pDesc->ModelName);


	m_iShaderPassIndex = pDesc->iShaderPassIndex;
	m_vDiffuseColor = pDesc->vDiffuseColor;
	m_eInstanceType = pDesc->eInstanceType;

		_char ModelName[MAX_PATH] = {};
		sprintf_s(ModelName, "Com_Model%d", 0);
		if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), ProtoName,
			StringToWString(ModelName), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
			CRASH("FAILED");

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->WorldMatrix));

	// DeferredShader
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh_Instance"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	// ShadowShader
	//if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh_Instance"),
	//	TEXT("Com_ShadowShader"), reinterpret_cast<CComponent**>(&m_pShadowShaderCom), nullptr)))
	//	CRASH("FAILED");

	m_pBoundingBox = new BoundingBox(pDesc->vBoundingPos, pDesc->vBoundingExtends);
	if (!m_pBoundingBox)
		CRASH("Failed");

	CRigidbody::BOXBODY_DESC RigidbodyDesc{};
	////RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	//XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	//RigidbodyDesc.eShape = SHAPE::BOX;
	//RigidbodyDesc.vPos = pDesc->vBoundingPos;
	//RigidbodyDesc.eType = EMotionType::Static;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	//RigidbodyDesc.vExtent = pDesc->vBoundingExtends;

	//RigidbodyDesc.eShape = SHAPE::BOX;
	//RigidbodyDesc.vPos = m_pBoundingBox->Center;
	//RigidbodyDesc.eType = EMotionType::Static;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	//RigidbodyDesc.vExtent = m_pBoundingBox->Extents;

	//Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
	//	TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

}

CMapObject_Instance* CMapObject_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Instance* pInstance = new CMapObject_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Instance::Clone(void* pArg)
{
	CMapObject_Instance* pClone = new CMapObject_Instance(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject_Instance::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pShadowShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pGameSystem);
}
