#include"ClientPch.h"
#include "MapObject_Burn.h"
#include"GameSystem.h"

_bool CMapObject_Burn::m_IsSound = { false };

CMapObject_Burn::CMapObject_Burn(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CMapObject_Burn::CMapObject_Burn(const CMapObject_Burn& Prototype)
	:CStaticObject(Prototype),
	m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_Burn::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Burn::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	Ready_Component(pArg);
    return S_OK;
}

void CMapObject_Burn::Priority_Update(_float fTimeDelta)
{
	m_iShaderPassIndex = 23;
}

void CMapObject_Burn::Update(_float fTimeDelta)
{
	if (m_IsBurn)
	{
		m_fTime += fTimeDelta / 2.f;
		if (!m_IsSound)
		{
			_uint iSoundChannel = m_pGameInstance->Register_Channel();
			m_pGameInstance->Play_Sound_Dynamic(TEXT("Fire0"), iSoundChannel, 0.2f);
			m_pGameInstance->Return_Channel(iSoundChannel);
			m_pGameSystem->OnTriggerActivate(61);
			m_IsSound = !m_IsSound;
		}
		m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
	}
	else
		m_fTime = 0.f;
}

void CMapObject_Burn::Late_Update(_float fTimeDelta)
{
	if (m_fTime < 1.f)
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONSTATIC, this);
	else
	{
		m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
#ifndef _DEBUG
		m_pRigidbodyCom->IsActivate(false);
#endif
	}
}

void CMapObject_Burn::Render()
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
		if (m_IsBurn)
			m_pShaderCom->Bind_Value("g_DissolveTime", &m_fTime, sizeof(_float));
		m_pShaderCom->Bind_Value("g_DissolveStart", &m_IsBurn, sizeof(_bool));

		m_pShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(m_iLODIndex, i);
	}
}
void CMapObject_Burn::Render_Shadow()
{
}

void CMapObject_Burn::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
}

BoundingBox* CMapObject_Burn::Get_BoundingBox()
{
    return nullptr;
}

void CMapObject_Burn::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_iShaderPassIndex = pDesc->iShaderPassIndex;

	_wstring WModelName = Model;
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	WModelName.pop_back();
	if (FAILED(Add_Component(ENUM_CLASS(pDesc->iLevel), WModelName,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	RigidbodyDesc.eShape = SHAPE::MESH;
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eType = EMotionType::Static;
	RigidbodyDesc.eBodyType = CRigidbody::BODY; // 어떤 충돌 형태로 할ㅈ ㅣ정의
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::BURN);
	RigidbodyDesc.pModel = m_pModelCom;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);


	CRigidbody::BOXBODY_DESC DetectRigidbodyDesc{};
	DetectRigidbodyDesc.eShape = SHAPE::BOX;
	XMStoreFloat3(&DetectRigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	DetectRigidbodyDesc.eType = EMotionType::Kinematic;
	DetectRigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::DETECT);
	DetectRigidbodyDesc.vExtent = _float3(10.f, 10.f, 10.f);
	//플레이어 감지용 1개
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_DetectRigidbody"), reinterpret_cast<CComponent**>(&m_pDetectRigidbodyCom), &DetectRigidbodyDesc);

	m_pDetectRigidbodyCom->SetUp_CallBack(COLLIDE_STATE::ENTER, [this](_uint iLayer, void* pDesc, const ContactManifold& Manifold) {
		if (iLayer == ENUM_CLASS(COLLISIONLAYER::THROW))
			m_IsBurn = true;

		});
}

CMapObject_Burn* CMapObject_Burn::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Burn* pInstance = new CMapObject_Burn(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject_Collaps");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Burn::Clone(void* pArg)
{
	CMapObject_Burn* pInstance = new CMapObject_Burn(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject_Collaps (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapObject_Burn::Free()
{
	__super::Free();

	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pDetectRigidbodyCom);
	
	Safe_Release(m_pGameSystem);
}
