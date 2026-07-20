#include "ClientPch.h"
#include "TraceurGroundStand.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurGroundStand::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	return S_OK;
}

void CTraceurGroundStand::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(true);

#ifdef _DEBUG
	_float4 vVector = {};
	XMStoreFloat4(&vVector, m_pTransformCom->Get_Velocity());
	OutPutDebugFloat4(TEXT("Collider Stand 이동량 : "), vVector);
#endif // _DEBUG
}

void CTraceurGroundStand::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurGroundStand::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);
}

CTraceurGroundStand* CTraceurGroundStand::Create(CTraceur* pOwner)
{
	CTraceurGroundStand* pInstance = new CTraceurGroundStand();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurGroundStand");
		return nullptr;
	}

	return pInstance;
}

void CTraceurGroundStand::Free()
{
	__super::Free();
}
