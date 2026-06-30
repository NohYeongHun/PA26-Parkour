#include"EditorPch.h"
#include "Edit_MapObject_Water.h"
#include"Event_Level.h"
#include"Map_Interface.h"
#include"Level_Map.h"

CEdit_MapObject_Water::CEdit_MapObject_Water(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CStaticObject(pDevice,pContext)
{
}

CEdit_MapObject_Water::CEdit_MapObject_Water(const CEdit_MapObject_Water& Prototype)
	:CStaticObject(Prototype)
{
}

HRESULT CEdit_MapObject_Water::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}
HRESULT CEdit_MapObject_Water::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Component(pArg)))
		return E_FAIL;
	Ready_Events();

	MAP_CREATE event(m_ModelName, this);
	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);
	//MAP_CREATE event(m_ModelName, this);
	m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), event);

	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Water") + to_wstring(m_iSaveIndex), [this](const MAP_SAVE& event) {

		if (!m_isActivate)
			return;

		auto iter = event.ModelName.find(m_ModelName);

		if (iter == event.ModelName.end())
			event.ModelName.insert(m_ModelName);

		_uint Length = strlen(m_ModelName);
		event.File.write(reinterpret_cast<const char*>(&Length), sizeof(_uint));
		event.File.write(m_ModelName, Length);

#ifdef _DEBUG
		if (!strcmp(m_pShaderCom->Get_PassName(m_iShaderPassIndex), "SelectedObject"))
			m_iShaderPassIndex = 0;
#endif // _DEBUG

		

		event.File.write(reinterpret_cast<const char*>(&m_iShaderPassIndex), sizeof(_uint));
		event.File.write(reinterpret_cast<const char*>(&m_eObjectType), sizeof(OBJECTTYPE));
		_float4x4 WorldMatrix;
		XMStoreFloat4x4(&WorldMatrix, m_pTransformCom->Get_WorldMatrix());
		event.File.write(reinterpret_cast<const _char*>(&WorldMatrix), sizeof(_float4x4));

		_float4 vDiffuseColor = m_vDiffuseColor.float_4;
		event.File.write(reinterpret_cast<const char*>(&vDiffuseColor), sizeof(_float4));

		});

	m_iShaderPassIndex = 0;

	return S_OK;
}

void CEdit_MapObject_Water::Priority_Update(_float fTimeDelta)
{
	if (m_IsOffset)
	{
		m_XBindOffSet += m_fXOffset;
		//if (m_XBindOffSet >= 1.f)
		//	m_XBindOffSet -= 1.f;
		
		m_YBindOffSet += m_fYOffset;
		/*if (m_YBindOffSet >= 1.f)
			m_YBindOffSet -= 1.f;*/
	}
}

void CEdit_MapObject_Water::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		if (m_iLevel == ENUM_CLASS(LEVEL::MAP))
		{
			if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
			{

				{
					_float fDistance = {};
					_vector RayPos = XMVector3TransformCoord(XMLoadFloat3(&CLevel_Map::m_vWorldPos), m_pTransformCom->Get_WorldMatrix_Inv());
					_vector RayDir = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&CLevel_Map::m_vWorldDir), m_pTransformCom->Get_WorldMatrix_Inv()));
					if (m_pModelCom->Is_Picked(RayPos, RayDir, &fDistance))
					{
						MAP_PICK event(this, fDistance);

						m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), event);
					}
				}
			}
		}
	}
#endif
}

void CEdit_MapObject_Water::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_MapObject_Water::Render()
{
	Bind_Resources();
	for (_uint i = 0; i < m_pModelCom->Get_NumMesh(); ++i)
	{
		//이름만 마스킹. 사실은 Flow;
		/*if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", i, TEXTURETYPE::MASK)))
			return;*/

		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", i, TEXTURETYPE::DIFFUSE)))
			return;

		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", i, TEXTURETYPE::NORMAL)))
			return;

		m_pShaderCom->Bind_Value("g_vDiffuseColor", &m_vDiffuseColor, sizeof(_float4));

		if(m_IsOffset)
		{
			m_pShaderCom->Bind_Value("g_fXOffset", &m_XBindOffSet, sizeof(_float));
			m_pShaderCom->Bind_Value("g_fYOffset", &m_YBindOffSet, sizeof(_float));
		}

		m_pShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(i);
	}
}

void CEdit_MapObject_Water::Render_Shadow()
{

}

void CEdit_MapObject_Water::Set_ImGuiOption()
{
	m_pMapInterface->Set_Transform(m_pTransformCom);
	m_pMapInterface->Set_ShaderPass(m_pShaderCom, &m_iShaderPassIndex);
	ImGui::SliderFloat(" - X Offset", &m_fXOffset, 0.00f, 0.1f, "%.3f");
	ImGui::SliderFloat(" - Y Offset", &m_fYOffset, 0.00f, 0.1f, "%.3f");
	ImGui::Begin("Color Water");
	ImGui::ColorPicker4("SelectColor", m_vDiffuseColor.arr);
	ImGui::End();

	ImGui::Checkbox("Offset Bind", &m_IsOffset);

	if (ImGui::Button("Destroy"))
		m_isActivate = false;

}

HRESULT CEdit_MapObject_Water::Ready_Component(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
	strcpy_s(m_ModelName, pDesc->ModelName);
	m_iLevel = pDesc->iLevel;

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};

	MultiByteToWideChar(CP_ACP, 0, m_ModelName, -1, Name, strlen(m_ModelName));
	lstrcat(Model, Name);
	//_uint V = m_ModelName[strlen(m_ModelName) - 1] - '0' + 1;

	//
	//_wstring ModelCom = Model;
	//ModelCom.pop_back();
	//ModelCom += to_wstring(0);

	if (FAILED(__super::Add_Component(pDesc->iLevel, Model,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh_Water"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;
	
	return S_OK;
}

void CEdit_MapObject_Water::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

}

_uint CEdit_MapObject_Water::ShaderPassWindow()
{
	return m_iShaderPassIndex;
}

void CEdit_MapObject_Water::Set_ShaderPass(_uint iShaderPass)
{
	m_iShaderPassIndex = iShaderPass;
	//m_pMapInterface->Set_ShaderPass(m_pShaderCom, &m_iShaderPassIndex);
}

void CEdit_MapObject_Water::Ready_Events()
{
}

CEdit_MapObject_Water* CEdit_MapObject_Water::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_MapObject_Water* pInstance = new CEdit_MapObject_Water(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Edit_MapObject_Water");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_MapObject_Water::Clone(void* pArg)
{
	CEdit_MapObject_Water* pInstance = new CEdit_MapObject_Water(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Edit_MapObject_Water (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_MapObject_Water::Free()
{
	__super::Free();
	Safe_Release(m_pMapInterface);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

}
