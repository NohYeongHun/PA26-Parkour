#include"EditorPch.h"
#include "Edit_LightObject.h"
#include"Event_Level.h"
#include"Map_Interface.h"
#include"Level_Map.h"

_uint CEdit_LightObject::g_iLightIndex = 0;

CEdit_LightObject::CEdit_LightObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{
}

CEdit_LightObject::CEdit_LightObject(const CEdit_LightObject& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CEdit_LightObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEdit_LightObject::Initialize_Clone(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	Ready_Component(pArg);
	m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&pDesc->vWorldPos));

	m_iLightIndex = g_iLightIndex++;
	LIGHT_CREATE event(m_iLightIndex, this);

	m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Light_Create"), event);
	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);
	return S_OK;
}

void CEdit_LightObject::Priority_Update(_float fTimeDelta)
{
}

void CEdit_LightObject::Update(_float fTimeDelta)
{
}

void CEdit_LightObject::Late_Update(_float fTimeDelta)
{
}

void CEdit_LightObject::Render()
{
}

void CEdit_LightObject::Render_Shadow()
{
}

void CEdit_LightObject::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
	const LIGHT_DESC* CopyDesc = pDesc->CopyDesc;
	LIGHT_DESC LightDesc{};
	if (CopyDesc)
	{
		m_vLightAmbient.float_4 = LightDesc.vAmbient = CopyDesc->vAmbient;
		m_vLightDiffuse.float_4 = LightDesc.vDiffuse = CopyDesc->vDiffuse;
		m_vLightSpec.float_4 = LightDesc.vSpecular = CopyDesc->vSpecular;
		LightDesc.eType = CopyDesc->eType;

		if (LightDesc.eType == LIGHT_DESC::DIRECTION)
		{
			LightDesc.vDirection = CopyDesc->vDirection;
		}
		else if (LightDesc.eType == LIGHT_DESC::POINT)
		{
			LightDesc.vPosition = CopyDesc->vPosition;
			LightDesc.fRange = CopyDesc->fRange;
		}

		if (pDesc->IsCopy)
		{
			LightDesc.vDiffuse = pDesc->vLightDiffuse;
			LightDesc.vAmbient = pDesc->vLightAmbient;
			LightDesc.vSpecular = pDesc->vLightSpec;

			m_vLightDiffuse.float_4 = pDesc->vLightDiffuse;
			m_vLightAmbient.float_4 = pDesc->vLightAmbient;
			m_vLightSpec.float_4 = pDesc->vLightSpec;
		}
	}
	else
	{
		LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
		LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
		LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
		LightDesc.eType = LIGHT_DESC::POINT;
		LightDesc.vPosition = CLevel_Map::m_vPickedPos;
		LightDesc.fRange = 5.f;
	}
	m_pGameInstance->Add_Light(to_wstring(g_iLightIndex), LightDesc);

#ifdef _DEBUG
	m_LightDesc = m_pGameInstance->Get_LightDesc_For_Map(to_wstring(g_iLightIndex));
#endif // _DEBUG


}

void CEdit_LightObject::Set_ImGuiOption()
{
	m_pMapInterface->Set_Transform(m_pTransformCom);
	ImGui::Begin("Color");
	ImGui::ColorPicker4("Light Color", m_vLightDiffuse.arr);
	ImGui::End();
	m_LightDesc->vDiffuse = m_vLightDiffuse.float_4;

	ImGui::ColorPicker4("Set Diffuse", m_vLightDiffuse.arr);
	m_LightDesc->vDiffuse = m_vLightDiffuse.float_4;
	ImGui::InputFloat4("Set Ambient", m_vLightAmbient.arr);
	m_LightDesc->vAmbient = m_vLightAmbient.float_4;
	ImGui::InputFloat4("Set Specular", m_vLightSpec.arr);
	m_LightDesc->vSpecular = m_vLightSpec.float_4;

	if (m_LightDesc->eType == LIGHT_DESC::POINT)
	{
		XMStoreFloat4(&m_LightDesc->vPosition, m_pTransformCom->Get_State(STATE::POSITION));
		ImGui::SliderFloat("fRange", &m_LightDesc->fRange, 0.1f, 100.f, "%.3f");
	}

	if (ImGui::Button("Copy"))
		Copy();
	if(ImGui::Button("Delete"))
	{
		m_isActivate = false;
		m_pGameInstance->Set_LightActive(to_wstring(m_iLightIndex), false);
	}
}

void CEdit_LightObject::Copy()
{
	MAP_LOAD CopyDesc{};
	CopyDesc.CopyDesc = m_LightDesc;
	CopyDesc.vWorldPos = CLevel_Map::m_vPickedPos;
	CopyDesc.IsCopy = true;
	CopyDesc.vLightDiffuse = m_vLightDiffuse.float_4;
	CopyDesc.vLightAmbient = m_vLightAmbient.float_4;
	CopyDesc.vLightSpec = m_vLightSpec.float_4;

	m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_GameObject_LightObject"), ENUM_CLASS(LEVEL::MAP), TEXT("Layer_Light"), &CopyDesc);


}

CEdit_LightObject* CEdit_LightObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_LightObject* pInstance = new CEdit_LightObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : LightObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_LightObject::Clone(void* pArg)
{
	CEdit_LightObject* pInstance = new CEdit_LightObject(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : LightObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}
void CEdit_LightObject::Free()
{
	__super::Free();
	m_LightDesc = nullptr;
	Safe_Release(m_pMapInterface);
}
