#include "EditorPch.h"
#include "SQ_Effect_Edit.h"

CSQ_Effect_Edit::CSQ_Effect_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSQ_Item_Edit { pDevice, pContext }
{
}

CSQ_Effect_Edit::CSQ_Effect_Edit(const CSQ_Effect_Edit& Prototype)
	: CSQ_Item_Edit { Prototype }
{
}

HRESULT CSQ_Effect_Edit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSQ_Effect_Edit::Initialize_Clone(void* pArg)
{
	if(FAILED(__super::Initialize_Clone(pArg)))
		CRASH("SQ_Actor_Edit")

    return S_OK;
}

void CSQ_Effect_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSQ_Effect_Edit::Update(_float fTimeDelta)
{
}

void CSQ_Effect_Edit::Late_Update(_float fTimeDelta)
{
}

void CSQ_Effect_Edit::Render()
{
}

void CSQ_Effect_Edit::Render_Shadow()
{
}

void CSQ_Effect_Edit::Render_OutLine()
{
}

void CSQ_Effect_Edit::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
}

CSQ_Effect_Edit* CSQ_Effect_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSQ_Effect_Edit* pInstance = new CSQ_Effect_Edit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
		CRASH("SQ_Effect_Edit")

    return pInstance;
}

CGameObject* CSQ_Effect_Edit::Clone(void* pArg)
{
	CSQ_Effect_Edit* pClone = new CSQ_Effect_Edit(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("SQ_Effect_Edit (Clone)")

	return pClone;
}

void CSQ_Effect_Edit::Free()
{
	__super::Free();
}
