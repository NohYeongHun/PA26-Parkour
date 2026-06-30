#include "EditorPch.h"
#include "EditDummy.h"

CEditDummy::CEditDummy(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CEditDummy::CEditDummy(const CEditDummy& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CEditDummy::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEditDummy::Initialize_Clone(void* pArg)
{
	ASSERT_CRASH(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("Failed to Cloned : Dummy");

	DUMMY_DESC* pDesc = static_cast<DUMMY_DESC*>( pArg );

	m_pTransformCom->Scale(pDesc->vScale);
	m_pTransformCom->Set_State(STATE::POSITION, pDesc->vPosition);
	m_pTransformCom->Rotation_Quaternion(pDesc->vRotation);
	
	return S_OK;
}

void CEditDummy::Priority_Update(_float fTimeDelta)
{
}

void CEditDummy::Update(_float fTimeDelta)
{
}

void CEditDummy::Late_Update(_float fTimeDelta)
{
}

void CEditDummy::Render()
{
}

void CEditDummy::Render_Shadow()
{
}

void CEditDummy::Free()
{
	__super::Free();
}
