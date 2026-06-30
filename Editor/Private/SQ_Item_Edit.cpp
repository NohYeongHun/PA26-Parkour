#include "EditorPch.h"
#include "SQ_Item_Edit.h"

CSQ_Item_Edit::CSQ_Item_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CSQ_Item_Edit::CSQ_Item_Edit(const CSQ_Item_Edit& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CSQ_Item_Edit::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSQ_Item_Edit::Initialize_Clone(void* pArg)
{
	if(FAILED(__super::Initialize_Clone(pArg)))
		CRASH("SQ_Item_Edit")

	return S_OK;
}

void CSQ_Item_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSQ_Item_Edit::Update(_float fTimeDelta)
{
}

void CSQ_Item_Edit::Late_Update(_float fTimeDelta)
{
}

void CSQ_Item_Edit::Render()
{
}

void CSQ_Item_Edit::Render_Shadow()
{
}

void CSQ_Item_Edit::Render_OutLine()
{
}

void CSQ_Item_Edit::Free()
{
	__super::Free();
}
