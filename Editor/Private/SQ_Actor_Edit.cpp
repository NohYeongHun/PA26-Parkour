#include "EditorPch.h"
#include "SQ_Actor_Edit.h"

CSQ_Actor_Edit::CSQ_Actor_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSQ_Item_Edit { pDevice, pContext }
{
}

CSQ_Actor_Edit::CSQ_Actor_Edit(const CSQ_Actor_Edit& Prototype)
	: CSQ_Item_Edit { Prototype }
{
}

HRESULT CSQ_Actor_Edit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSQ_Actor_Edit::Initialize_Clone(void* pArg)
{
	if(FAILED(__super::Initialize_Clone(pArg)))
		CRASH("SQ_Actor_Edit")

    return S_OK;
}

void CSQ_Actor_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSQ_Actor_Edit::Update(_float fTimeDelta)
{
}

void CSQ_Actor_Edit::Late_Update(_float fTimeDelta)
{
}

void CSQ_Actor_Edit::Render()
{
}

void CSQ_Actor_Edit::Render_Shadow()
{
}

void CSQ_Actor_Edit::Render_OutLine()
{
}

void CSQ_Actor_Edit::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
}

CSQ_Actor_Edit* CSQ_Actor_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSQ_Actor_Edit* pInstance = new CSQ_Actor_Edit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
		CRASH("SQ_Actor_Edit")

    return pInstance;
}

CGameObject* CSQ_Actor_Edit::Clone(void* pArg)
{
	CSQ_Actor_Edit* pClone = new CSQ_Actor_Edit(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("SQ_Actor_Edit (Clone)")

	return pClone;
}

void CSQ_Actor_Edit::Free()
{
	__super::Free();
}
