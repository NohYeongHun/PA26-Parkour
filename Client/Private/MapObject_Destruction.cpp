#include "ClientPch.h"
#include "MapObject_Destruction.h"
#include "GameSystem.h"
#include "MapObject_Destruction_Debris.h"

vector<_wstring> CMapObject_Destruction::m_SoundTags;

CMapObject_Destruction::CMapObject_Destruction(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice, pContext)
{
}

CMapObject_Destruction::CMapObject_Destruction(const CMapObject_Destruction& Prototype)
	:CStaticObject(Prototype),m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_Destruction::Initialize_Prototype()
{
	m_SoundTags.push_back(TEXT("Rock_Broken0"));
	m_SoundTags.push_back(TEXT("Rock_Broken1"));
	m_SoundTags.push_back(TEXT("Rock_Broken2"));
	m_SoundTags.push_back(TEXT("Rock_Broken3"));
	m_SoundTags.push_back(TEXT("Rock_Broken4"));
	m_SoundTags.push_back(TEXT("Rock_Broken5"));
	m_SoundTags.push_back(TEXT("Rock_Broken6"));
	m_SoundTags.push_back(TEXT("Rock_Broken7"));
	m_SoundTags.push_back(TEXT("Rock_Broken8"));
	m_SoundTags.push_back(TEXT("Rock_Broken9"));
	m_SoundTags.shrink_to_fit();
	return S_OK;
}

HRESULT CMapObject_Destruction::Initialize_Clone(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));
	m_iTriggerIndex = pDesc->iTriggerIndex;

	if (FAILED(Ready_Component(pArg)))
		return E_FAIL;

	m_iNumLOD = m_pModelCom->Get_LastLODIndex();

	m_pBoundingBox = new BoundingBox(pDesc->vBoundingPos, pDesc->vBoundingExtends);
	if (!m_pBoundingBox)
		CRASH("Failed");

	//m_pGameInstance->Add_To_OctoTree(this, m_pBoundingBox);

	m_iShaderPassIndex = pDesc->iShaderPassIndex;
	m_vImpulsePos = pDesc->m_vImpulsePos;
	m_vImpulsePower = pDesc->m_vImpulsePower;

	m_IsDestroy = false;

	return S_OK;
}

void CMapObject_Destruction::Priority_Update(_float fTimeDelta)
{
}

void CMapObject_Destruction::Update(_float fTimeDelta)
{
	if (m_pBoxRigidbodyCom)
		m_pBoxRigidbodyCom->Update_Rigidbody(m_pTransformCom->Get_WorldMatrix(), fTimeDelta);
}

void CMapObject_Destruction::Late_Update(_float fTimeDelta)
{
	if (!m_IsDestroy)
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONSTATIC, this);
	else
		if (!m_IsChange)
		{
			m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
			m_IsChange = true;
		}
}

void CMapObject_Destruction::Render()
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

	//m_pGameInstance->Bind_SharedBuffer(0, m_pContext);

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
#ifdef _DEBUG
		if (m_pBoxRigidbodyCom)
			m_pBoxRigidbodyCom->Render();
#endif // _DEBUG
}


void CMapObject_Destruction::Render_Shadow()
{
}

HRESULT CMapObject_Destruction::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};
	
	MultiByteToWideChar(CP_ACP, 0, pDesc->ModelName, -1, Name, strlen(pDesc->ModelName));
	lstrcat(Model, Name);

	m_szDebrisName = Model;
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), m_szDebrisName,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");


	_wstring BoneName = Name;
	BoneName += TEXT("_Bone");

	strcpy_s(m_BoneModelName, WStringToString(BoneName).c_str());

	if (FAILED(Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Model_") + BoneName,
		StringToWString(pDesc->ModelName) + TEXT("_Bone"), reinterpret_cast<CComponent**>(&m_pBoneModel), nullptr)))
		CRASH("FAILED");

	m_pBoneModel->Update_BoneMatrix_Map();

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
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

	_string ModelName = pDesc->ModelName;

	for (_uint i = 2; i < m_pBoneModel->Get_BoneSize() - 1; ++i)
	{
		_string Name = ModelName; // 예: "SM_Sev_Roc_24BS_"

		// 1. 문자열 스트림 생성
		std::stringstream ss;

		// 2. 스트림에 포맷팅 룰 적용
		//    (3자리로 고정하고, 빈 칸은 '0'으로 채우기)
		ss << std::setw(3) << std::setfill('0') << i - 2;

		// 3. 스트림의 문자열을 Name에 추가
		Name += "_";
		Name += ss.str(); // ss.str()이 "000", "001", ..., "010", ..., "100" 등을 반환
		//Name += "_LOD0";
		CMapObject_Destruction_Debris::MAP_LOAD Desc;
		Desc.iLevel = pDesc->iLevel;
		Desc.iShaderPassIndex = 0;
		strcpy_s(Desc.ModelName, Name.c_str());
		_vector vScale, vRot, vTrans;
		_float4x4 TestMat = *m_pBoneModel->Get_BoneMatrixPtr(i);
		XMMatrixDecompose(&vScale, &vRot, &vTrans, XMLoadFloat4x4(&TestMat));
		_vector TT = XMQuaternionNormalize(vRot);
		_float4x4 Mat;
		XMStoreFloat4x4(&Mat,
			XMMatrixRotationQuaternion(TT) *
			XMMatrixTranslationFromVector(vTrans) *
			m_pTransformCom->Get_WorldMatrix());
		Desc.WorldMatrix = &Mat;

		_vector Pos = XMVectorSetW(XMLoadFloat3(&m_vImpulsePos), 1.f);
		_vector Power = XMVectorSetW(XMLoadFloat3(&m_vImpulsePower), 0.f);

		_vector vDeltaPos = XMVectorSetW(XMLoadFloat3(reinterpret_cast<_float3*>(&Mat.m[3])) - Pos, 0.f);

		XMStoreFloat3(&Desc.vImpulse, vDeltaPos * Power);
		if (FAILED(m_pGameInstance->Add_PoolingObject(pDesc->iLevel, TEXT("Prototype_GameObject_MapObject_Destruction_Debris")
			, pDesc->iLevel, TEXT("Layer_Destruction_Debris"), m_szDebrisName + to_wstring(i), 3, &Desc)))
			return S_OK;
	}
	return S_OK;
}

void CMapObject_Destruction::Bind_Resources()
{
}

CMapObject_Destruction* CMapObject_Destruction::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Destruction* pInstance = new CMapObject_Destruction(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject_Destruction");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Destruction::Clone(void* pArg)
{
	CMapObject_Destruction* pClone = new CMapObject_Destruction(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject_Destruction (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject_Destruction::Free()
{
	__super::Free();

	m_pPullUI = nullptr;
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pBoneModel);
	Safe_Release(m_pGameSystem);
	Safe_Delete(m_pBoundingBox);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pBoxRigidbodyCom);
	
}
