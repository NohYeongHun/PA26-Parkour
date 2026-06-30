#include"EditorPch.h"
#include "Edit_MapObject_Collaps.h"
#include"Map_Interface.h"
#include"Level_Map.h"
#include"Event_Level.h"
CEdit_MapObject_Collaps::CEdit_MapObject_Collaps(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice,pContext)
{
}

CEdit_MapObject_Collaps::CEdit_MapObject_Collaps(const CEdit_MapObject_Collaps& Prototype)
	:CGameObject(Prototype)
{
}

HRESULT CEdit_MapObject_Collaps::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEdit_MapObject_Collaps::Initialize_Clone(void* pArg)
{
	//옥토트리에 안넣을 놈들. 넣을까?

	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	strcpy_s(m_ModelName, pDesc->ModelName);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->vSourWorldMatrix));

	Ready_Components(pArg);
	_char Tag[MAX_PATH] = "Collaps";
	MAP_CREATE event(Tag, this);
	m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), event);
	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Collaps"), [this](const MAP_SAVE& event) {
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

		event.File.write(reinterpret_cast<const _char*>(&m_vSourMat), sizeof(_float4x4));
		event.File.write(reinterpret_cast<const _char*>(&m_vDestMat), sizeof(_float4x4));

		event.File.write(reinterpret_cast<const _char*>(&m_fDuration), sizeof(_float));

		//여기 들어가면 발동되는 거
		event.File.write(reinterpret_cast<const _char*>(&m_iTriggerIndex), sizeof(_uint));

		//이게 발동시킬 거
		event.File.write(reinterpret_cast<const _char*>(&m_iTriggerActiveIndex), sizeof(_int));



		//이거를 로컬로 보내거나 회전이랑 스케일까지 전부 변환된 걸 보내야함.

		});

	return S_OK;
}

void CEdit_MapObject_Collaps::Priority_Update(_float fTimeDelta)
{
}

void CEdit_MapObject_Collaps::Update(_float fTimeDelta)
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
	if (m_Test)
		LerpPos(fTimeDelta);
}

void CEdit_MapObject_Collaps::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}
void CEdit_MapObject_Collaps::Render()
{
	_uint m_iLODIndex = 0;

	_bool HasNormal = { true };
	_bool HasMask = { true };
	_uint iNumMesh = m_pModelCom->Get_NumMesh(m_iLODIndex);

	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pModelCom->Bind_Buffer(m_pContext, m_iLODIndex);
	for (_uint i = 0; i < iNumMesh; ++i)
	{
		if (m_pModelCom->Is_Overed(m_iLODIndex, i))
			return;

		if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_MaskTexture", m_iLODIndex, i, TEXTURETYPE::MASK)))
		{
			m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
			HasMask = false;
		}

		if (HasMask)
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL)))
				HasNormal = false;
		}
		else
		{
			m_pModelCom->Bind_Materials(m_pShaderCom, "g_DiffuseTexture", m_iLODIndex, i, TEXTURETYPE::DIFFUSE, 0);

			if (FAILED(m_pModelCom->Bind_Materials(m_pShaderCom, "g_NormalTexture", m_iLODIndex, i, TEXTURETYPE::NORMAL, 0)))
				HasNormal = false;
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));

		m_pShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(m_iLODIndex, i);
	}
}
void CEdit_MapObject_Collaps::LerpPos(_float fTimeDelta)
{
	m_fFall += fTimeDelta;
	_float Time = m_fFall / m_fDuration;

	_vector vNewTrans = XMVectorLerp(m_vSourTrans.Vec, m_vDestTrans.Vec, Time);
	_vector vNewRot = XMQuaternionSlerp(m_vSourRot.Vec, m_vDestRot.Vec, Time);

	m_pTransformCom->Set_WorldMatrix(XMMatrixAffineTransformation(m_vSourScale.Vec, XMVectorZero(), vNewRot, vNewTrans));

	if (m_fFall >= m_fDuration)
	{
		m_fFall = 0.f;
		m_Test = false;
		m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&m_vDestMat));
		m_pSourRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::MAP));
		m_pDestRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
		return;
	}
}

void CEdit_MapObject_Collaps::Set_ImGuiOption()
{
	m_pMapInterface->Set_Transform(m_pTransformCom);
	m_pMapInterface->Set_ShaderPass(m_pShaderCom, &m_iShaderPassIndex);

	if (ImGui::Button("Set Sour"))
	{
		XMStoreFloat4x4(&m_vSourMat, m_pTransformCom->Get_WorldMatrix());
		XMMatrixDecompose(&m_vSourScale.Vec, &m_vSourRot.Vec, &m_vSourTrans.Vec, XMLoadFloat4x4(&m_vSourMat));
	}
	ImGui::SameLine();
	if (ImGui::Button("Move To Start"))
		m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&m_vSourMat));

	if (ImGui::Button("Set Dest"))
	{
		XMStoreFloat4x4(&m_vDestMat, m_pTransformCom->Get_WorldMatrix());
		XMMatrixDecompose(&m_vDestScale.Vec, &m_vDestRot.Vec, &m_vDestTrans.Vec, XMLoadFloat4x4(&m_vDestMat));
	}
	ImGui::SameLine();
	if (ImGui::Button("Move To Dest"))
		m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&m_vDestMat));
	ImGui::InputFloat(" - Duration", &m_fDuration, 0.1f, 0.1f, "%.1f");
	ImGui::InputScalar("TriggerIndex : ", ImGuiDataType_U32, &m_iTriggerIndex);
	ImGui::InputInt("TriggerActiveIndex : ", &m_iTriggerActiveIndex);

	if (ImGui::Button("Test"))
	{
		m_pSourRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
		m_pDestRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::MAP));
		m_Test = true;
	}
	ImGui::SameLine();

	if (ImGui::Button("Destroy"))
		m_isActivate = false;

}

void CEdit_MapObject_Collaps::Ready_Components(void* pArg)
{
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);
	m_iLevel = pDesc->iLevel;
	m_vSourMat = pDesc->vSourWorldMatrix;
	m_vDestMat = pDesc->vDestWorldMatrix;
	m_fDuration = pDesc->fDuration;
	m_iTriggerIndex = pDesc->TriggerIndex;
	m_iTriggerActiveIndex = pDesc->TriggerActiveIndex;
	m_iShaderPassIndex = pDesc->iShaderPassIndex;
	XMMatrixDecompose(&m_vSourScale.Vec, &m_vSourRot.Vec, &m_vSourTrans.Vec, XMLoadFloat4x4(&m_vSourMat));
	XMMatrixDecompose(&m_vDestScale.Vec, &m_vDestRot.Vec, &m_vDestTrans.Vec, XMLoadFloat4x4(&m_vDestMat));
	_wstring Model = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};
	MultiByteToWideChar(CP_ACP, 0, m_ModelName, -1, Name, strlen(m_ModelName));
	Model += Name;
	
	Model.pop_back();
	Model.pop_back();
	Model.pop_back();
	Model.pop_back();
	Model.pop_back();

	if (FAILED(Add_Component(pDesc->iLevel, Model,
		StringToWString(m_ModelName), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

#ifdef _DEBUG
	m_pModelCom->Ready_BoundingBox();
#endif // _DEBUG


	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	CRigidbody::MESHBODY_DESC RigidbodyDesc = {};
	RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.eShape = SHAPE::MESH;
	RigidbodyDesc.eType = EMotionType::Static;
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	RigidbodyDesc.pModel = m_pModelCom;

	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody_Sour"), reinterpret_cast<CComponent**>(&m_pSourRigidbodyCom), &RigidbodyDesc);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->vDestWorldMatrix));
	RigidbodyDesc.vScale = m_pTransformCom->Get_Scaled();
	XMStoreFloat4(&RigidbodyDesc.vQuat, m_pTransformCom->Get_Quaternion());
	XMStoreFloat3(&RigidbodyDesc.vPos, m_pTransformCom->Get_State(STATE::POSITION));
	RigidbodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::NONE);
	Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody_Dest"), reinterpret_cast<CComponent**>(&m_pDestRigidbodyCom), &RigidbodyDesc);

	m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(&pDesc->vSourWorldMatrix));

	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);

}

CEdit_MapObject_Collaps* CEdit_MapObject_Collaps::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_MapObject_Collaps* pInstance = new CEdit_MapObject_Collaps(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : Edit_MapObject_Collaps");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEdit_MapObject_Collaps::Clone(void* pArg)
{
	CEdit_MapObject_Collaps* pInstance = new CEdit_MapObject_Collaps(*this);

	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Edit_MapObject_Collaps (Clone)");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEdit_MapObject_Collaps::Free()
{
	__super::Free();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pMapInterface);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pSourRigidbodyCom);
	Safe_Release(m_pDestRigidbodyCom);
}
