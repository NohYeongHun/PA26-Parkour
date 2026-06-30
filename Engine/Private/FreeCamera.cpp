#include "EnginePch.h"
#include "FreeCamera.h"

#include "GameInstance.h"

CFreeCamera::CFreeCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera { pDevice, pContext }
{
}

CFreeCamera::CFreeCamera(const CFreeCamera& Prototype)
	: CCamera { Prototype }
{
}

HRESULT CFreeCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFreeCamera::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_fSpeed = 200.f; // Default Speed

	return S_OK;
}

void CFreeCamera::Priority_Update(_float fTimeDelta)
{
}

void CFreeCamera::Update(_float fTimeDelta)
{
	__super::Key_Move(fTimeDelta);
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::RB) == KEYSTATE::PRESS)
		__super::Mouse_Move_Up();

//#ifdef _DEBUG

	ImGui::Begin("Camera Speed");

	ImGui::InputFloat("##", &m_fSpeed);

	if (ImGui::Button("Apply", ImVec2(50.f, 20.f)))
		m_pTransformCom->Change_Speed(m_fSpeed);

	ImGui::InputFloat3("##", reinterpret_cast<_float*>(&m_vPosition), "%.2f");
	if (ImGui::Button("Move", ImVec2(50.f, 20.f)))
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_vPosition), 1.f));

	_float3 vCamPos = {};
	XMStoreFloat3(&vCamPos, m_pTransformCom->Get_State(STATE::POSITION));
	ImGui::PushID(1000);
	ImGui::InputFloat3("##", reinterpret_cast<_float*>(&vCamPos), "%.2f");
	ImGui::PopID();

	ImGui::End();

	m_pGameInstance->Update_Listener(m_pTransformCom, fTimeDelta);
//#endif
}

void CFreeCamera::Late_Update(_float fTimeDelta)
{


}

void CFreeCamera::Render()
{
}

CFreeCamera* CFreeCamera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFreeCamera* pInstance = new CFreeCamera(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : FreeCamera");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFreeCamera::Clone(void* pArg)
{
	CFreeCamera* pClone = new CFreeCamera(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : FreeCamera (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CFreeCamera::Free()
{
	__super::Free();
}