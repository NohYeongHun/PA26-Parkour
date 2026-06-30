#include"EditorPch.h"
#include "Edit_FireFly_Manager.h"
#include"Level_Map.h"
#include"Event_Level.h"
#include"Model_Instance_FireFly.h"
#include"Edit_FireFly.h"
#include"Mesh_Instance_FireFly.h"

_uint CEdit_FireFly_Manager::g_iFlyIndex = { 0 };
CEdit_FireFly_Manager::CEdit_FireFly_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:m_pGameInstance(CGameInstance::GetInstance()),
	m_pDevice(pDevice),m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}
HRESULT CEdit_FireFly_Manager::Initialize()
{
	m_iLevel = ENUM_CLASS(LEVEL::MAP);
	m_vPickedPos.float_4 = _float4(0.f, 0.f, 0.f, 1.f);
	m_pGameInstance->Subscribe<FLY>(ENUM_CLASS(LEVEL::STATIC), TEXT("FLY"), [this](const FLY& event) {
		m_pPickedFly = static_cast<CEdit_FireFly*>(event.pObject);
		Safe_AddRef(m_pPickedFly);
		m_Fly.emplace(g_iFlyIndex++, m_pPickedFly);
		});
	
	Create_Fly();
	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_FireFly"), [this](const MAP_SAVE& event) {
		for (auto& pFly : m_Fly)
		{
			if (!pFly.second)
				continue;
			if (!pFly.second->IsActivate())
				continue;

			pFly.second->SaveData(event.File);
		}
		});


    return S_OK;
}

void CEdit_FireFly_Manager::Set_ImGuiOption()
{
	ImGui::Begin("Flies");
	ImGuiID Light = ImGui::GetID("Fly");
	ImGui::BeginChildFrame(Light, ImVec2(100, 200));
	ImGui::Text(to_string(m_iPickedIndex).c_str());

	for (auto iter = m_Fly.begin(); iter != m_Fly.end(); iter++)
	{
		if (ImGui::Button(to_string(iter->first).c_str())) {
			m_iPickedIndex = iter->first;
			m_pPickedFly = iter->second;
		}
	}
	ImGui::EndChildFrame();
	ImGui::SameLine();

	ImGui::Checkbox("Render Gizmo", &m_IsRenderGizmo);

	ImGui::InputFloat3("Pos", m_vPickedPos.arr);
	ImGui::SameLine();
	if (ImGui::Button("Get PickPos"))
		m_vPickedPos.float_4 = CLevel_Map::m_vPickedPos;

	ImGui::SameLine();
	if (ImGui::Button("Get CamPos"))
		m_vPickedPos.float_4 = *m_pGameInstance->Get_CamPos();

	_int minus = -1;
	_uint plus = 1;
	ImGui::InputScalar("Instance Num Value : ", ImGuiDataType_U32, &m_iTempInstanceNum, &plus, &minus);
	
	ImGui::InputFloat2("Range", m_iTempRangePerInstance.arr);
	ImGui::InputFloat2("X", m_vSinPerInstance.arr);
	ImGui::InputFloat2("Y", m_vCosPerInstance.arr);
	ImGui::InputFloat2("Z", m_vSin2PerInstance.arr);

	if (ImGui::Button("Save"))
		Create_Fly(m_iTempInstanceNum, m_iTempRangePerInstance.float_2, m_iTempShaderPassIndex, m_vSinPerInstance.float_2, m_vCosPerInstance.float_2, m_vSin2PerInstance.float_2);

	if (ImGui::Button("Delete"))
	{
		if (!m_Fly.empty() && m_Fly[m_iPickedIndex])
		{
			m_Fly[m_iPickedIndex]->SetActivate(false);
			Safe_Release(m_Fly[m_iPickedIndex]);
			m_Fly.erase(m_iPickedIndex);
		}
	}

	if (m_pPickedFly && m_IsRenderGizmo)
		m_pPickedFly->Set_ImGuiOption();

	ImGui::End();
}

void CEdit_FireFly_Manager::Create_Fly()
{
	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_FireFly"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh_Instance_FireFly.hlsl"), VTXMESHINSTANCE_FIREFLY::Elements, VTXMESHINSTANCE_FIREFLY::iNumElements));

	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_FireFly"),
		CEdit_FireFly::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");
}

void CEdit_FireFly_Manager::Create_Fly(_uint InstanceNum, _float2 Range, _uint ShaderPassIndex, _float2 Sin, _float2 Cos, _float2 Sin2)
{
	m_iPickedIndex = g_iFlyIndex;
	_float Size = 0.00005f;
	_matrix PreTransformMatrix = XMMatrixScaling(Size, Size, Size);
	_string Path = "../../Client/Bin/Resource/Map/Asphodel_Barrens/FireFly/FireFly.dat";

	_wstring ProtoName = TEXT("Fly") + to_wstring(g_iFlyIndex);
	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
		CModel_Instance_FireFly::Create(m_pDevice, m_pContext, PreTransformMatrix, Path.c_str(), true))))
		CRASH("Prototype Create Failed");

	CEdit_FireFly::MAP_LOAD Desc{};
	Desc.iNumInstance = InstanceNum;
	Desc.iShaderPassIndex = ShaderPassIndex;
	strcpy_s(Desc.ModelName, WStringToString(ProtoName).c_str());
	Desc.IsLoaded = false;
	Desc.vPerSin = Sin;
	Desc.vPerCos = Cos;
	Desc.vPerSin2 = Sin2;
	Desc.vRange = Range;
	_float4x4* pFireFlyMat= new _float4x4[InstanceNum];
	for (_uint i = 0; i < InstanceNum; ++i)
		XMStoreFloat4x4(&pFireFlyMat[i], XMMatrixTranslationFromVector(m_vPickedPos.Vec));
	Desc.InstanceWorldMatrix = pFireFlyMat;
	m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_FireFly"), m_iLevel, TEXT("Layer_FireFly"), &Desc);
	Safe_Delete_Array(pFireFlyMat);
}

void CEdit_FireFly_Manager::Map_Load(CEdit_FireFly::MAP_LOAD& Desc)
{

	_float Size = 0.00005f;
	_matrix PreTransformMatrix = XMMatrixScaling(Size, Size, Size);
	_string Path = "../../Client/Bin/Resource/Map/Asphodel_Barrens/FireFly/FireFly.dat";
	CMesh_Instance_FireFly::MESH_INST_DESC MeshDesc{};
	MeshDesc.iNumInstance = Desc.iNumInstance;

	_float4x4* pFireFlyMat = new _float4x4[Desc.iNumInstance];
	for (_uint i = 0; i < Desc.iNumInstance; ++i)
		pFireFlyMat[i] = Desc.WorldMatrix;

	MeshDesc.pTransformMatrix = pFireFlyMat;
	MeshDesc.vPerMoveSin = Desc.vPerSin;
	MeshDesc.vPerMoveCos = Desc.vPerCos;
	MeshDesc.vPerMoveSin2 = Desc.vPerSin2;
	MeshDesc.vRange = Desc.vRange;
	_wstring ProtoName = TEXT("Fly") + to_wstring(g_iFlyIndex);
	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
		CModel_Instance_FireFly::Create(m_pDevice, m_pContext, PreTransformMatrix, Path.c_str(), false, &MeshDesc))))
		CRASH("Prototype Create Failed");
	
	strcpy_s(Desc.ModelName, WStringToString(ProtoName).c_str());
	Desc.IsLoaded = true;
	//_float4x4* pFireFlyMat = new _float4x4[Desc.iNumInstance];
	//for (_uint i = 0; i < Desc.iNumInstance; ++i)
	//	pFireFlyMat[i] = Desc.WorldMatrix;
	Desc.InstanceWorldMatrix = pFireFlyMat;
	m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_FireFly"), m_iLevel, TEXT("Layer_FireFly"), &Desc);
	Safe_Delete_Array(pFireFlyMat);
	m_iPickedIndex = g_iFlyIndex;
}

CEdit_FireFly_Manager* CEdit_FireFly_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_FireFly_Manager* pInstance = new CEdit_FireFly_Manager(pDevice, pContext);
	pInstance->Initialize();
	return pInstance;
}

void CEdit_FireFly_Manager::Free()
{
	__super::Free();
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	m_pPickedFly = nullptr;
	Safe_Release(m_pGameInstance);
	
	for (auto& pFly : m_Fly)
	{
		Safe_Release(pFly.second);
	}
	m_Fly.clear();
}
