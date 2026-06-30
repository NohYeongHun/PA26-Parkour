#include "EnginePch.h"
#include "Effect.h"

CEffect::CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject { pDevice, pContext }
{
}

CEffect::CEffect(const CEffect& Prototype)
	: CGameObject { Prototype }
{
}

HRESULT CEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEffect::Initialize_Clone(void* pArg)
{
	return S_OK;
}

void CEffect::Priority_Update(_float fTimeDelta)
{
}

void CEffect::Update(_float fTimeDelta)
{
}

void CEffect::Late_Update(_float fTimeDelta)
{
}

void CEffect::Render()
{
}

void CEffect::Free()
{
	__super::Free();
}
