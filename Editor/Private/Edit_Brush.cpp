#include "EditorPch.h"
#include "Edit_Brush.h"
#include"Edit_MapObject_Instance.h"
#include"Event_Level.h"

CEdit_Brush::CEdit_Brush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject(pDevice, pContext)

{
}

CEdit_Brush::CEdit_Brush(const CEdit_Brush& Prototype)
    :CGameObject(Prototype)
{
}

HRESULT CEdit_Brush::Initialize_Prototype()
{
	__super::Initialize_Clone(nullptr);

	Ready_Components();
	m_fRange = 100.f;
	m_iNumInstance = 10.f;
	m_iMinNum = 1;
	m_iMaxNum = 100;
	m_vMaxRotation = 360.f;

	INSTANCE_CREATE event();
	m_pGameInstance->Subscribe<INSTANCE_CREATE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Instance_Create"), [this](const INSTANCE_CREATE& event) {
		CEdit_MapObject_Instance* pObject = static_cast<CEdit_MapObject_Instance*>(event.pObject);

		m_SaveInstanceObjects[event.iNumSaveIndex].push_back(pObject);
		Safe_AddRef(pObject);
		});

	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map_Instance"), [this](const MAP_SAVE& event) {
		for (auto Pair : m_SaveInstanceObjects)
		{
			if (Pair.second.empty())
				continue;

			event.File.write(reinterpret_cast<const char*>(&Pair.first), sizeof(_uint));

			_uint Length = strlen(Pair.second[0]->GetName());
			event.File.write(reinterpret_cast<const char*>(&Length), sizeof(_uint));
			event.File.write(Pair.second[0]->GetName(), Length);
			_uint ShaderPass = Pair.second[0]->Get_ShaderPass();
			event.File.write(reinterpret_cast<const char*>(&ShaderPass), sizeof(_uint));

			event.File.write(reinterpret_cast<const char*>(Pair.second[0]->Get_Color()), sizeof(_float4));
			//포지션을 총합한 뒤 나눠서 진짜 중점 찾기. 각 매트릭스들의 위치. 인스턴스 개수. 셰이더패스..?

			vector<_float4x4> Totalmatrix;
			vector<_vector> Objectmatrix;
			_uint iNumTotalInstance = { 0 };
			_float4 m_RealCenterPos = {};

			INSTANCE_SAVE SaveEvent(Totalmatrix, Objectmatrix, &iNumTotalInstance);

			m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Instance") + to_wstring(Pair.first), SaveEvent);

			event.File.write(reinterpret_cast<const char*>(&iNumTotalInstance), sizeof(_uint));

			_vector CenterPos = XMVectorSet(0.f, 0.f, 0.f, 0.f);
			float posX = Totalmatrix[0].m[3][0];
			float posY = Totalmatrix[0].m[3][1];
			float posZ = Totalmatrix[0].m[3][2];

			_vector vPos = XMVectorSet(posX, posY, posZ, 1.f);

			_vector MaxPos = vPos;
			_vector MinPos = vPos;
			for (_uint i = 0; i < Totalmatrix.size(); ++i)
			{
				event.File.write(reinterpret_cast<const char*>(&Totalmatrix[i]), sizeof(_float4x4));
				float posX = Totalmatrix[i].m[3][0];
				float posY = Totalmatrix[i].m[3][1];
				float posZ = Totalmatrix[i].m[3][2];

				_vector vPos = XMVectorSet(posX, posY, posZ, 1.f);

				MinPos = XMVectorMin(MinPos, vPos);
				MaxPos = XMVectorMax(MaxPos, vPos);
			}

			MinPos -= XMVectorSet(2.f, 2.f, 2.f, 0.f);
			MaxPos += XMVectorSet(2.f, 2.f, 2.f, 0.f);

			_float3 vBoundingBoxPos;
			XMStoreFloat3(&vBoundingBoxPos, (MinPos + MaxPos) / 2.f);

			_float3 vBoundingBoxExtends;
			XMStoreFloat3(&vBoundingBoxExtends, XMVectorSet(
				(MaxPos.m128_f32[0] - MinPos.m128_f32[0]) / 2.f,
				(MaxPos.m128_f32[1] - MinPos.m128_f32[1]) / 2.f,
				(MaxPos.m128_f32[2] - MinPos.m128_f32[2]) / 2.f, 1.f));

			for (_uint i = 0; i < Objectmatrix.size(); ++i)
			{
				CenterPos += Objectmatrix[i];
			}
			CenterPos /= static_cast<_float>(Objectmatrix.size());

			_float4x4 CombinedMatirx{};
			XMStoreFloat4x4(&CombinedMatirx, XMMatrixTranslationFromVector(CenterPos));
			event.File.write(reinterpret_cast<const char*>(&CombinedMatirx), sizeof(_float4x4));

			event.File.write(reinterpret_cast<const _char*>(&vBoundingBoxPos), sizeof(_float3));
			event.File.write(reinterpret_cast<const _char*>(&vBoundingBoxExtends), sizeof(_float3));

			INSTANCETYPE Type = Pair.second[0]->Get_Type();
			event.File.write(reinterpret_cast<const _char*>(&Type), sizeof(INSTANCETYPE));
		}
		});



	return S_OK;
}

HRESULT CEdit_Brush::Initialize_Clone(void* pArg)
{
    return S_OK;
}

void CEdit_Brush::Priority_Update(_float fTimeDelta)
{
	_uint minus = -1;
	_uint plus = 1;

	About_InstanceInfo();

	ImGui::Begin("Color");
	ImGui::ColorPicker4("SelectColor", vDiffuseColor.arr);

	if (!m_SaveInstanceObjects[m_iCurSaveIndex].empty())
	{
		for (auto& pObject : m_SaveInstanceObjects[m_iCurSaveIndex])
			pObject->Set_Color(vDiffuseColor.float_4);
	}
	ImGui::End();

	const _char* pObejceTType[] = { "Default","Sonoro","NonSonoro" };

	if (ImGui::BeginCombo("Instance_Type", pObejceTType[ENUM_CLASS(m_eInstanceType)]))
	{
		for (_uint i = 0; i < ENUM_CLASS(INSTANCETYPE::END); ++i)
		{
			if (ImGui::Selectable(pObejceTType[i]))
			{
				m_eInstanceType = static_cast<INSTANCETYPE>(i);
				for (auto& pObject : m_SaveInstanceObjects[m_iCurSaveIndex])
					pObject->Set_Type(m_eInstanceType);
			}
		}
		ImGui::EndCombo();
	}
	//이게 하나만 쓰다보니 애들이 다 바뀜.
	ImGui::InputScalar("Instance ShaderPass: ", ImGuiDataType_U32, &m_iShaderPassIndex);

	ImGui::InputFloat("Range : ", &m_fRange);
	ImGui::SliderFloat("Range Slider : ", &m_fRange, 1.f, 4000.f, "%.1f");
	ImGui::InputScalar("Instance Num Value : ", ImGuiDataType_U32, &m_iNumInstance, &minus, &plus);
	ImGui::SliderScalar("Instance Num Value Slider : ", ImGuiDataType_U32, &m_iNumInstance, &m_iMinNum, &m_iMaxNum, "%d");

	ImGui::SliderFloat("Min Degree", &m_vMinRotation, 0.0f, 359.9f, "%.1f");
	ImGui::SliderFloat("Max Degree", &m_vMaxRotation, 0.0f, 360.f, "%.1f");

	//버튼으로 되게.
	ImGui::InputScalar("Instance SaveIndex: ", ImGuiDataType_U32, &m_iCurSaveIndex, &plus);
	//객체들중에서도 선택하고, 그 선택된 놈 중에서도 지들이 가진 매트릭스를 넣어서 위치 수정 가능하게..
	//m_pGameInstance->Use_Gizmo_Offset();
	//ImGui::End();
}

void CEdit_Brush::Update(_float fTimeDelta)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        Foliage();
    }

//#ifdef _DEBUG
//    m_pGameInstance->Add_Render_Object(RENDERGROUP::RD_DEBUG, this);
//#endif
}

void CEdit_Brush::Late_Update(_float fTimeDelta)
{
}

void CEdit_Brush::Render()
{
    return;

    Bind_Resources();
    m_pShaderCom->Begin(0);
    m_pVIBufferCom->Bind_Resources();
    m_pVIBufferCom->Render();
}

void CEdit_Brush::About_InstanceInfo()
{
	ImGui::Text("Current Instance");

	ImGui::Text(WStringToString(m_szModelName).c_str());

	auto iter = m_SaveInstanceObjects.find(m_iCurSaveIndex);
	if (iter != m_SaveInstanceObjects.end())
	{
		if (!m_SaveInstanceObjects[m_iCurSaveIndex].empty())
			ImGui::Text(m_SaveInstanceObjects[m_iCurSaveIndex][0]->GetName());
	}
	else
		m_SaveInstanceObjects.erase(m_iCurSaveIndex);
	ImGuiID ShaderId = ImGui::GetID("Container");
	ImGui::BeginChildFrame(ShaderId, ImVec2(200, 200));

	for (auto& pPair : m_SaveInstanceObjects)
	{
		_string ButtonName = to_string(pPair.first);
		if (!pPair.second.empty())
			ButtonName += pPair.second[0]->GetName();
		if (ImGui::Button(ButtonName.c_str())) {

			m_iCurSaveIndex = pPair.first;
			m_iPickedSpecipic = 0;
		}
	}
	ImGui::EndChildFrame();

	auto iter2 = m_SaveInstanceObjects.find(m_iCurSaveIndex);
	if (iter2 != m_SaveInstanceObjects.end())
	{
		if(!m_SaveInstanceObjects[m_iCurSaveIndex].empty())
		{
			ImGui::SameLine();
			m_iShaderPassIndex = m_SaveInstanceObjects[m_iCurSaveIndex][0]->ShaderPassWindow();
			m_eInstanceType = m_SaveInstanceObjects[m_iCurSaveIndex][0]->Get_Type();
			if (!XMVector4Equal(vDiffuseColor.Vec, XMLoadFloat4(m_SaveInstanceObjects[m_iCurSaveIndex][0]->Get_Color())))
				vDiffuseColor.float_4 = *m_SaveInstanceObjects[m_iCurSaveIndex][0]->Get_Color();

			for (auto& pObject : m_SaveInstanceObjects[m_iCurSaveIndex])
				pObject->Change_ShaderIndex(m_iShaderPassIndex);
		}
	}
	

	ImGui::Button("Undo");
	if ((ImGui::IsItemHovered() && m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::PRESS) || m_pGameInstance->Get_DIKeyState(DIK_DELETE) == KEYSTATE::PRESS)
	{
		if (!m_SaveInstanceObjects[m_iCurSaveIndex].empty())
		{
			CEdit_MapObject_Instance* pObject = dynamic_cast<CEdit_MapObject_Instance*>(m_SaveInstanceObjects[m_iCurSaveIndex].back());
			pObject->SetActivate(false);
			Safe_Release(pObject);
			m_SaveInstanceObjects[m_iCurSaveIndex].pop_back();
		}
		else
			m_SaveInstanceObjects.erase(m_iCurSaveIndex);
	}

	if (ImGui::Button("Combined Instance"))
	{
		if (m_SaveInstanceObjects[m_iCurSaveIndex].empty())
			return;

		vector<_float4x4> Totalmatrix;
		vector<_vector> Objectmatrix;
		_uint iNumTotalInstance = {};
		_float4 m_RealCenterPos = {};

		INSTANCE_SAVE SaveEvent(Totalmatrix, Objectmatrix, &iNumTotalInstance);

		m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Instance") + to_wstring(m_iCurSaveIndex), SaveEvent);

		CEdit_MapObject_Instance::MAP_LOAD Desc{};
		Desc.iSaveIndex = m_iCurSaveIndex;
		Desc.iNumInstance = iNumTotalInstance;
		Desc.iShaderPassIndex = m_SaveInstanceObjects[m_iCurSaveIndex][0]->Get_ShaderPass();
		Desc.IsLoaded = false;
		strcpy_s(Desc.ModelName, m_SaveInstanceObjects[m_iCurSaveIndex][0]->GetName());
		if (Desc.iShaderPassIndex == 2)
			Desc.vDiffuseColor = *m_SaveInstanceObjects[m_iCurSaveIndex][0]->Get_Color();

		_float4x4* pInstanceArray = new _float4x4[Desc.iNumInstance];
		
		copy(Totalmatrix.begin(), Totalmatrix.end(), pInstanceArray);
		Desc.InstanceWorldMatrix = pInstanceArray;

		_vector CenterPos = {};

		for (_uint i = 0; i < Objectmatrix.size(); ++i)
		{
			CenterPos += Objectmatrix[i];
		}

		CenterPos /= static_cast<_float>(Objectmatrix.size());
		XMStoreFloat4x4(&Desc.WorldMatrix, XMMatrixTranslationFromVector(CenterPos));

		for (auto& pObject : m_SaveInstanceObjects[m_iCurSaveIndex])
			pObject->SetActivate(false);

		m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_GameObject_MapObject_Instance")
			, ENUM_CLASS(LEVEL::MAP), TEXT("Layer_Instance"), &Desc);

		Safe_Delete_Array(pInstanceArray);
	}
}

void CEdit_Brush::Set_ModelName(const _wstring& pModelName)
{
    lstrcpy(m_szModelName, pModelName.c_str());
}

void CEdit_Brush::Bind_Resources()
{
    m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
    m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
    m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

    if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Depth"), m_pShaderCom, "g_DepthTexture")))
        CRASH("Render Fail")

    m_pShaderCom->Bind_Value("g_fRange", &m_fRange, sizeof(_float));
    m_pShaderCom->Bind_Value("g_iNumInstance", &m_iNumInstance, sizeof(_uint));
}

void CEdit_Brush::Ready_Components()
{
    __super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_VIBuffer_Point"),
        TEXT("Com_VIBufferCom"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr);
}

void CEdit_Brush::Foliage()
{
    if (IsSelected(m_szModelName) == false)
	{
		MSG_BOX("Model Not Selected");
		return;
	}

    if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::PRESS)
    {
		if (Check_Duplicate() == false)
		{
			MSG_BOX("Different Model Selected");
			return;
		}

		m_TempPoints.clear();
		m_TransformMatrices.clear();

		if (m_pGameInstance->Get_Points(m_fRange, m_TempPoints, &m_vMousePos) == true)
		{
			Generate_WorldMatrices(m_TransformMatrices, m_TempPoints);

			if (m_TransformMatrices.empty() == true)
			{
				MSG_BOX("TransformMatrices Create Failed");
				return;
			}

			Generate_Instance(m_TransformMatrices, m_vMousePos);
		}
    }
}
	
_bool CEdit_Brush::Check_Duplicate()
{		
	if (m_SaveInstanceObjects[m_iCurSaveIndex].empty())
		return true;
	_string szSavedModel = m_SaveInstanceObjects[m_iCurSaveIndex][0]->GetName();
	szSavedModel.pop_back();
	_string szCurModel = WStringToString(m_szModelName);
	szCurModel.pop_back();
	if (szSavedModel != szCurModel)
	{
		MSG_BOX("Diffrent Model");
		return false;
	}
		
	return true;
}		

void CEdit_Brush::Generate_WorldMatrices(vector<_float4x4>& TransformMatrices, const vector<_float4>& Points)
{
	if (m_iNumInstance == 0 || Points.empty() == true)
	{
		MSG_BOX("Check Settings");
		return;
	}

	size_t iRandSize = Points.size() - 1;
	_uint iRandNum = { 0 };
	_float fRotation = { 0.f };
	_float4x4 InstanceMatrix = {};

	for (_uint i = 0; i < m_iNumInstance; ++i)
	{
		iRandNum = GetVaildRandomIndex(Points, 0, iRandSize);
		fRotation = m_pGameInstance->Rand(m_vMinRotation, m_vMaxRotation);
		InstanceMatrix = Composite_WorldMatrix(Points[iRandNum], fRotation);
		TransformMatrices.push_back(InstanceMatrix);
	}

	return;
}

_bool CEdit_Brush::Generate_Instance(vector<_float4x4>& TransformMatrices, _float4 vMousePos)
{
	CEdit_MapObject_Instance::MAP_LOAD Desc;
	Desc.InstanceWorldMatrix = TransformMatrices.data();
	Desc.iNumInstance = m_iNumInstance;
	strcpy_s(Desc.ModelName, WStringToString(m_szModelName).c_str());
	XMStoreFloat4x4(&Desc.WorldMatrix, XMMatrixTranslationFromVector(XMLoadFloat4(&vMousePos)));
	Desc.iSaveIndex = m_iCurSaveIndex;
	Desc.iShaderPassIndex = m_iShaderPassIndex;
	Desc.IsLoaded = false;
	Desc.eInstanceType = m_eInstanceType;

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_GameObject_MapObject_Instance")
		, ENUM_CLASS(LEVEL::MAP), TEXT("Layer_Instance"), &Desc)))
	{
		MSG_BOX("Failed to Create Instance");
		return false;
	}

	return true;
}

_uint CEdit_Brush::GetVaildRandomIndex(const vector<_float4>& Points, _uint iMin, _uint iMax)
{
	_uint RandIndex = { 0 };

	do {
		RandIndex = m_pGameInstance->Rand(iMin, iMax);
	} while (Points[RandIndex].w == 0);

	return RandIndex;
}

_float4x4 CEdit_Brush::Composite_WorldMatrix(const _float4& RandWorldPos, _float fRotation)
{
	_float4x4 TransfromMatrix = {};

	if (m_vMaxRotation == 0.0f)
		XMStoreFloat4x4(&TransfromMatrix, XMMatrixTranslationFromVector(XMLoadFloat4(&RandWorldPos)));
	else
	{
		_vector RotationQuat = XMQuaternionRotationNormal(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(fRotation));
		XMStoreFloat4x4(&TransfromMatrix, XMMatrixRotationQuaternion(RotationQuat) * XMMatrixTranslationFromVector(XMLoadFloat4(&RandWorldPos)));
	}

	return TransfromMatrix;
}

CEdit_Brush* CEdit_Brush::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEdit_Brush* pInstance = new CEdit_Brush(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : Edit_Brush");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEdit_Brush::Free()
{
    __super::Free();
    Safe_Release(m_pShaderCom);
    Safe_Release(m_pVIBufferCom);

	for (auto& Pair: m_SaveInstanceObjects)
	{
		for (auto& pObject : Pair.second)
			Safe_Release(pObject);
		Pair.second.clear();
	}
	m_SaveInstanceObjects.clear();
}
