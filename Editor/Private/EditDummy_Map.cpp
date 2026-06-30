#include "EditorPch.h"
#include "EditDummy_Map.h"

CEditDummy_Map::CEditDummy_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEditDummy { pDevice, pContext }
{
}

CEditDummy_Map::CEditDummy_Map(const CEditDummy_Map& Prototype)
	: CEditDummy { Prototype }
{
}

HRESULT CEditDummy_Map::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEditDummy_Map::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		CRASH("Failed to Cloned : Dummy_Map");

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	DUMMY_MAP_DESC* pDesc = static_cast<DUMMY_MAP_DESC*>(pArg);

	if (FAILED(Ready_Component(pDesc->PreTransformMatrix)))
		return E_FAIL;

	return S_OK;
}

void CEditDummy_Map::Priority_Update(_float fTimeDelta)
{
}

void CEditDummy_Map::Update(_float fTimeDelta)
{
}

void CEditDummy_Map::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEditDummy_Map::Render()
{

	_uint m_iLODIndex = 0;

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

		m_pShaderCom->Begin(10);
		m_pModelCom->Render(m_iLODIndex, i);
	}
}

void CEditDummy_Map::Render_Shadow()
{
}

HRESULT CEditDummy_Map::Ready_Component(_fmatrix PreTransformMatrix)
{
	//m_pModelCom = CModel_Streaming::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Map/The_False_Sovereign/Rock/Common_QiQue/SM_Sev_Roc_54AS/");
	//m_pModelCom = CModel_Streaming::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Map/The_False_Sovereign/Rock/Common/1024/SM_Com2_Roc_14AM/");
	m_pModelCom = CModel_Streaming::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Map/Asphodel_Barrens/Rock/1026/SM_Tab_APD_Roc_11AH/");
	//m_pModelCom = CModel_Streaming::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Map/The_False_Sovereign/Rock/Common/1025/SM_Com2_Roc_39AX/");
//	m_pModelCom = CModel_Streaming::Create(m_pDevice, m_pContext, "../../Client/Bin/Resource/Map/Asphodel_Barrens/Rock/1026/SM_Tab_APD_Roc_02AH/");
	ASSERT_CRASH(m_pModelCom);	


	m_pShaderCom = CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements);
	ASSERT_CRASH(m_pShaderCom);

	m_pGameInstance->LoadLastLOD();

	//CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	//RigidbodyDesc.eShape = SHAPE::MESH;
	//XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	//RigidbodyDesc.eType = EMotionType::Static;
	//RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	////RigidbodyDesc.pModel = m_pModelCom;

	//Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
	//	TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);

	return S_OK;
}

CEditDummy_Map* CEditDummy_Map::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEditDummy_Map* pInstance = new CEditDummy_Map(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : EditDummy_Map");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CEditDummy_Map::Clone(void* pArg)
{
	CEditDummy_Map* pInstance = new CEditDummy_Map(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : EditDummy_Map");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEditDummy_Map::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRigidbodyCom);
}
