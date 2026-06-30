#include "EditorPch.h"
#include "EditDummy_Target.h"

CEditDummy_Target::CEditDummy_Target(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEditDummy { pDevice, pContext }
{
}

CEditDummy_Target::CEditDummy_Target(const CEditDummy_Target& Prototype)
	: CEditDummy { Prototype }
{
}

HRESULT CEditDummy_Target::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEditDummy_Target::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		CRASH("Failed to Cloned : Dummy_Map");

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	DUMMY_TARGET_DESC* pDesc = static_cast<DUMMY_TARGET_DESC*>(pArg);

	if (FAILED(Ready_Component(pDesc->PreTransformMatrix)))
		return E_FAIL;
	m_pColliderCom->Set_Gravity(false);

	return S_OK;
}

void CEditDummy_Target::Priority_Update(_float fTimeDelta)
{
}

void CEditDummy_Target::Update(_float fTimeDelta)
{
	_vector vVelocity = XMVectorSet(0.f, -9.8f, 0.f, 0.f);
	m_pColliderCom->Update(vVelocity);
}

void CEditDummy_Target::Late_Update(_float fTimeDelta)
{
	m_pColliderCom->Sync_Position(m_pTransformCom);

	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEditDummy_Target::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	_uint iNumMesh = m_pModelCom->Get_NumMesh();
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE);

		_bool HasNormal = { false };
		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
			HasNormal = true;

		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));

		m_pShaderCom->Begin(0);
		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	m_pColliderCom->Render();
	//m_pGameInstance->DrawRay(XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 200.f, 200.f, 1.f));
#endif
}

void CEditDummy_Target::Render_Shadow()
{
}

HRESULT CEditDummy_Target::Ready_Component(_fmatrix PreTransformMatrix)
{
	m_pModelCom = CModel::Create(m_pDevice, m_pContext, MODELTYPE::NONANIM, PreTransformMatrix, "../../Client/Bin/Resource/Dummy/Wolf/Wolf.dat");
	ASSERT_CRASH(m_pModelCom);

	m_pShaderCom = CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements);
	ASSERT_CRASH(m_pShaderCom);

	// Com_Collider
	CCollider::COLLIDER_DESC ColliderDesc = {};
	XMStoreFloat3(&ColliderDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	ColliderDesc.vOffset = _float3(0.f, 20.f, 0.f);
	ColliderDesc.eType = EMotionType::Kinematic;
	ColliderDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::ENEMY);
	ColliderDesc.fHeight = 10.f;
	ColliderDesc.fRadius = 20.f; //m_pGameInstance->Rand(5.f, 20.f);
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Collider"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &ColliderDesc);
	ASSERT_CRASH(m_pColliderCom);

	m_pColliderCom->Set_Desc(m_pTransformCom);

	return S_OK;
}

CEditDummy_Target* CEditDummy_Target::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEditDummy_Target* pInstance = new CEditDummy_Target(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : EditDummy_Target");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CEditDummy_Target::Clone(void* pArg)
{
	CEditDummy_Target* pInstance = new CEditDummy_Target(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : EditDummy_Target");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEditDummy_Target::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pColliderCom);
}
