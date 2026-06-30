#include "EditorPch.h"
#include "SQ_SFX_Edit.h"

CSQ_SFX_Edit::CSQ_SFX_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSQ_Item_Edit { pDevice, pContext }
{
}

CSQ_SFX_Edit::CSQ_SFX_Edit(const CSQ_SFX_Edit& Prototype)
	: CSQ_Item_Edit { Prototype }
{
}

HRESULT CSQ_SFX_Edit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSQ_SFX_Edit::Initialize_Clone(void* pArg)
{
	if(FAILED(__super::Initialize_Clone(pArg)))
		CRASH("SQ_Actor_Edit")

    return S_OK;
}

void CSQ_SFX_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSQ_SFX_Edit::Update(_float fTimeDelta)
{
}

void CSQ_SFX_Edit::Late_Update(_float fTimeDelta)
{
}

void CSQ_SFX_Edit::Render()
{
}

void CSQ_SFX_Edit::Render_Shadow()
{
}

void CSQ_SFX_Edit::Render_OutLine()
{
}

void CSQ_SFX_Edit::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
}

CSQ_SFX_Edit* CSQ_SFX_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSQ_SFX_Edit* pInstance = new CSQ_SFX_Edit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
		CRASH("SQ_SFX_Edit")

    return pInstance;
}

CGameObject* CSQ_SFX_Edit::Clone(void* pArg)
{
	CSQ_SFX_Edit* pClone = new CSQ_SFX_Edit(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("SQ_SFX_Edit (Clone)")

	return pClone;
}

void CSQ_SFX_Edit::Free()
{
	__super::Free();
}
