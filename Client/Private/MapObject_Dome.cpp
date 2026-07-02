#include"ClientPch.h"
#include "MapObject_Dome.h"
#include"GameSystem.h"

CMapObject_Dome::CMapObject_Dome(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CMapObject_Dome::CMapObject_Dome(const CMapObject_Dome& Prototype)
	:CStaticObject(Prototype),m_pGameSystem(CGameSystem::GetInstance())
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CMapObject_Dome::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapObject_Dome::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	Ready_Component(pArg);
    return S_OK;
}

void CMapObject_Dome::Priority_Update(_float fTimeDelta)
{
}

void CMapObject_Dome::Update(_float fTimeDelta)
{
	m_fTotalTime += fTimeDelta;
	m_fAlpha += 0.001f;
	switch (m_iPhaze)
	{
	case 0:
		m_fMaxAlpha = 0.f;
		break;
	case 1:
		m_fMaxAlpha = 0.3f;
		break;
	case 2:
		m_fAlpha += 0.003f;
		m_fMaxAlpha = 1.f;
		break;
	}
	if (m_fAlpha >= m_fMaxAlpha)
		m_fAlpha = m_fMaxAlpha;

	if (m_IsDissolveStart)
		m_fDissolveTime += fTimeDelta;
}

void CMapObject_Dome::Late_Update(_float fTimeDelta)
{
	if (m_iPhaze == 2 && m_fAlpha >= m_fMaxAlpha)
	{
		m_iShaderPassIndex = 27;
	}
	else
	{
		m_iShaderPassIndex = 24;
	}
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONLIGHT, this);
}

void CMapObject_Dome::Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex)
{
}

void CMapObject_Dome::Render()
{
	if (!m_IsRender)
		return;

	_uint m_iLODIndex = 0;
	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pGameInstance->Bind_SharedBuffer(m_iLODIndex, m_pContext);

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
		m_pShaderCom->Bind_Value("g_fAlpha", &m_fAlpha, sizeof(_float));
		m_pShaderCom->Bind_Value("g_DistortionTime", &m_fTotalTime, sizeof(_float));
		m_pShaderCom->Bind_Value("g_DissolveTime", &m_fDissolveTime, sizeof(_float));
		m_pShaderCom->Bind_Value("g_DissolveStart", &m_IsDissolveStart, sizeof(_bool));
		
		m_pShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(m_iLODIndex, i);
	}
}

void CMapObject_Dome::Render_Shadow()
{
}

void CMapObject_Dome::Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix)
{
}

BoundingBox* CMapObject_Dome::Get_BoundingBox()
{
    return m_pBoundingBox;
}

void CMapObject_Dome::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	lstrcat(Model, StringToWString(pDesc->ModelName).c_str());

	m_iShaderPassIndex = pDesc->iShaderPassIndex;
	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
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
#ifdef _DEBUG
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);
#endif

#ifndef _DEBUG
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
#endif
	RigidbodyDesc.pModel = m_pModelCom;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), reinterpret_cast<CComponent**>(&m_pRigidbodyCom), &RigidbodyDesc);
}

CMapObject_Dome* CMapObject_Dome::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapObject_Dome* pInstance = new CMapObject_Dome(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MapObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapObject_Dome::Clone(void* pArg)
{
	CMapObject_Dome* pClone = new CMapObject_Dome(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MapObject (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMapObject_Dome::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pGameSystem);
}