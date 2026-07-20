#include"Editorpch.h"
#include "Edit_MapObject.h"
#include"Event_Level.h"
//#include "AnimationActor.h"
#include"Level_Map.h"
#include"Map_Interface.h"

_uint CEdit_MapObject::g_iNumObjects = {};

CEdit_MapObject::CEdit_MapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CStaticObject(pDevice, pContext)
{
}

CEdit_MapObject::CEdit_MapObject(const CEdit_MapObject& Prototype)
    :CStaticObject(Prototype)
{
}

HRESULT CEdit_MapObject::Initialize_Prototype()
{
    if (FAILED(__super::Initialize_Prototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT CEdit_MapObject::Initialize_Clone(void* pArg)
{

	// Load_SoundTag("/Client/Bin/Resource/Sound/2D/BGM/rock/sound/");

    MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

    if (FAILED(__super::Initialize_Clone(pArg)))
        return E_FAIL;
#ifdef _DEBUG
    strcpy_s(m_ModelName, pDesc->ModelName);

    m_pTransformCom->Set_WorldMatrix(XMLoadFloat4x4(pDesc->WorldMatrix));

    if (FAILED(Ready_Component(pArg)))
        return E_FAIL;

	m_iNumLOD = m_pModelCom->Get_LastLODIndex();

    //박스 모델에서 종합해서 최종 크기.
    //m_pGameInstance->Add_To_OctoTree(this, m_pModelCom->Get_BoundingBox(0));
    _vector vScale, vRotation, vTranslation;

    XMMatrixDecompose(&vScale, &vRotation, &vTranslation, m_pTransformCom->Get_WorldMatrix());

    XMStoreFloat3(&m_vScale, vScale);
    XMStoreFloat3(&m_vTranslation, vTranslation);
    m_vNewScale = m_vScale;
    m_vRotation = m_vNewRotation = _float3(0.f, 0.f, 0.f);
    m_vNewTranslation = m_vTranslation;
    m_iShaderPassIndex = pDesc->iShaderPassIndex;
    m_eObjectType = pDesc->eObjectType;
	m_eParkourFlag = pDesc->eParkourFlag;
    MODELTYPE::MAP;

    _char Tag[MAX_PATH] = "NonInteraction";
    MAP_CREATE event(Tag, this);

    m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), event);

	m_pGameInstance->Subscribe<MAP_BOUND>(ENUM_CLASS(LEVEL::STATIC), TEXT("Calc_Size"), [this](const MAP_BOUND& event) {
		if (!m_isActivate)
			return;

		_float4x4 WorldMatrix;
		XMStoreFloat4x4(&WorldMatrix, m_pTransformCom->Get_WorldMatrix());

		_float3 vBoundingBoxPos = m_pModelCom->Get_BoundingBox()->Center;
		_float3 vBoundingBoxExtends = m_pModelCom->Get_BoundingBox()->Extents;
		_float3 vLocalCorners[BoundingBox::CORNER_COUNT];

		m_pModelCom->Get_BoundingBox()->GetCorners(vLocalCorners);

		_float3 vTransformedCorners[BoundingBox::CORNER_COUNT];

		for (_uint i = 0; i < BoundingBox::CORNER_COUNT; ++i)
		{
			XMStoreFloat3(&vTransformedCorners[i],
				XMVector3TransformCoord(XMLoadFloat3(&vLocalCorners[i]), XMLoadFloat4x4(&WorldMatrix)));
		}

		BoundingBox RealBox;
		BoundingBox::CreateFromPoints(RealBox, BoundingBox::CORNER_COUNT, vTransformedCorners, sizeof(_float3));

		_vector Center = XMLoadFloat3(&RealBox.Center);
		_vector Extents = XMLoadFloat3(&RealBox.Extents);

		*event.vMin = XMVectorMin(*event.vMin, Center - Extents);
		*event.vMax = XMVectorMax(*event.vMax, Center + Extents);
		});

	// 일반 Map Object 파일 Save 로직.
	m_pGameInstance->Subscribe<MAP_SAVE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map"), [this](const MAP_SAVE& event) {
		if (!m_isActivate)
			return;

		auto iter = event.ModelName.find(m_ModelName);

		if (iter == event.ModelName.end())
			event.ModelName.insert(m_ModelName);

		_uint Length = strlen(m_ModelName);
		event.File.write(reinterpret_cast<const char*>(&Length), sizeof(_uint));
		event.File.write(m_ModelName, Length);

		if (!strcmp(m_pShaderCom->Get_PassName(m_iShaderPassIndex), "SelectedObject"))
			m_iShaderPassIndex = 0;

		event.File.write(reinterpret_cast<const char*>(&m_iShaderPassIndex), sizeof(_uint));
		event.File.write(reinterpret_cast<const char*>(&m_eObjectType), sizeof(OBJECTTYPE));
		event.File.write(reinterpret_cast<const char*>(&m_eParkourFlag), sizeof(PARKOUR_FLAG));
		_float4x4 WorldMatrix;
		XMStoreFloat4x4(&WorldMatrix, m_pTransformCom->Get_WorldMatrix());
		event.File.write(reinterpret_cast<const _char*>(&WorldMatrix), sizeof(_float4x4));

		//이거를 로컬로 보내거나 회전이랑 스케일까지 전부 변환된 걸 보내야함.
		
		_float3 vBoundingBoxPos = m_pModelCom->Get_BoundingBox()->Center;
		_float3 vBoundingBoxExtends = m_pModelCom->Get_BoundingBox()->Extents;
		_float3 vLocalCorners[BoundingBox::CORNER_COUNT];
		
		m_pModelCom->Get_BoundingBox()->GetCorners(vLocalCorners);

		_float3 vTransformedCorners[BoundingBox::CORNER_COUNT];

		for (_uint i = 0; i < BoundingBox::CORNER_COUNT; ++i)
		{
			XMStoreFloat3(&vTransformedCorners[i],
				XMVector3TransformCoord(XMLoadFloat3(&vLocalCorners[i]), XMLoadFloat4x4(&WorldMatrix)));
		}

		BoundingBox RealBox;
		BoundingBox::CreateFromPoints(RealBox, BoundingBox::CORNER_COUNT, vTransformedCorners, sizeof(_float3));

		event.File.write(reinterpret_cast<const _char*>(&RealBox.Center), sizeof(_float3));
		event.File.write(reinterpret_cast<const _char*>(&RealBox.Extents), sizeof(_float3));

		});
#endif

    m_pDiffuseTextureCom.resize(m_pModelCom->Get_NumMesh(0));
    m_pNormalTextureCom.resize(m_pModelCom->Get_NumMesh(0));
	m_pMaskTextureCom.resize(m_pModelCom->Get_NumMesh(0));
	m_pMaskDiffuseTextureCom.resize(m_pModelCom->Get_NumMesh(0));
	m_pMaskNormalTextureCom.resize(m_pModelCom->Get_NumMesh(0));

	m_SelectedDiffuseName.resize(m_pModelCom->Get_NumMesh(0));
	m_SelectedNormalName.resize(m_pModelCom->Get_NumMesh(0));
	m_SelectedMaskTextureName.resize(m_pModelCom->Get_NumMesh(0));
	m_SelectedMaskDiffuseName.resize(m_pModelCom->Get_NumMesh(0));
	m_SelectedMaskNormalName.resize(m_pModelCom->Get_NumMesh(0));


	m_SelectedDiffuseTexturePath.resize(m_pModelCom->Get_NumMesh(0));
	m_SelectedNormalTexturePath.resize(m_pModelCom->Get_NumMesh(0));
	m_SelectedMaskTexturePath.resize(m_pModelCom->Get_NumMesh(0));
	m_SelectedMaskTexturePath.resize(m_pModelCom->Get_NumMesh(0));
	m_SelectedMaskDiffusePath.resize(m_pModelCom->Get_NumMesh(0));
	m_SelectedMaskNormalPath.resize(m_pModelCom->Get_NumMesh(0));


	m_iSelectedDiffuseIndex = new _uint[m_pModelCom->Get_NumMesh(0)];
	m_iSelectedNormalIndex = new _uint[m_pModelCom->Get_NumMesh(0)];
	m_iSelectedMaskIndex = new _uint[m_pModelCom->Get_NumMesh(0)];
	m_iSelectedMaskDiffuseIndex = new _uint[m_pModelCom->Get_NumMesh(0)];
	m_iSelectedMaskNormalIndex = new _uint[m_pModelCom->Get_NumMesh(0)];


	m_iSelectedMesh = 0;
	m_iSelectedMeshName = "Mesh : 0";


	m_iNumObject = CEdit_MapObject::g_iNumObjects++;

	XMStoreFloat4x4(&m_DefaultMat, m_pTransformCom->Get_WorldMatrix());
	/*if (m_eObjectType == OBJECTTYPE::SONORA)
		m_IsRender = false;*/

	return S_OK;
}

void CEdit_MapObject::Priority_Update(_float fTimeDelta)
{
	m_fDistortionTime += fTimeDelta;
	if (m_pGameInstance->Get_DIKeyState(DIK_J) == KEYSTATE::DOWN)
	{

		m_fMode = true;
		m_IsFlying = true;
		/*if (m_eObjectType == OBJECTTYPE::SONORA)
		{
			m_IsRender = false;
		}*/

		_float fDistance = XMVectorGetX(XMVector3Length(XMVectorSetY(m_pTransformCom->Get_State(STATE::POSITION), 0.f) - XMVectorSetY(XMLoadFloat4(m_pGameInstance->Get_CamPos()), 0.f)));

		float minDistance = 0.0f;
		float maxDistance = 600.0f;

		float maxDelay = 1.7f;
		float minDelay = 0.0f;

		float t = (fDistance - minDistance) / (maxDistance - minDistance);

		m_fDlayTime = maxDelay + (minDelay - maxDelay) * t;
	}


	if (m_pGameInstance->Get_DIKeyState(DIK_K) == KEYSTATE::DOWN)
	{
		m_fMode = false;
		m_IsFlying = false;
	}

	if (m_fMode)
	{
		m_fTotalTime += fTimeDelta;
		if (m_fDlayTime <= 0.f)
		{
			if (m_IsFlying)
			{
				m_fFlyingTime += fTimeDelta;
			}
		}
		else
			m_fDlayTime -= fTimeDelta;
	}

	if (m_fTotalTime >= 3.f)
	{
		m_fMode = false;
		m_IsFlying = false;
		m_fFlyingTime = 0.f;
		m_fTotalTime = 0.f;
	}
}

void CEdit_MapObject::Update(_float fTimeDelta)
{
#ifdef _DEBUG
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		if (m_iLevel == ENUM_CLASS(LEVEL::MAP))
		{
			if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
			{
				if (!m_IsRender)
					return;
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

void CEdit_MapObject::Late_Update(_float fTimeDelta)
{
	if (m_IsRender)
		m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);

}

void CEdit_MapObject::Render()
{
	if (m_iLODIndex > m_pModelCom->Get_LastLODIndex())
		return;

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
		if (m_TexMode)
		{

			if (m_pDiffuseTextureCom[i])
				m_pDiffuseTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture", 0);

			if (m_pNormalTextureCom[i])
				if (FAILED(m_pNormalTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_NormalTexture")))
					HasNormal = false;

			if (m_pMaskTextureCom[i])
				m_pMaskTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_MaskTexture");
			else
			{
				HasMask = false;
                m_pShaderCom->Bind_Texture("g_MaskTexture", nullptr);
			}
			if (m_pMaskDiffuseTextureCom[i])
				m_pMaskDiffuseTextureCom[i]->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture");
		}
		else
		{
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
		}
		m_pShaderCom->Bind_Value("g_HasNormal", &HasNormal, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_HasMask", &HasMask, sizeof(_bool));
		m_pShaderCom->Bind_Value("g_DistortionTime", &m_fDistortionTime, sizeof(_float));
		
		m_pShaderCom->Begin(m_iShaderPassIndex);
		m_pModelCom->Render(m_iLODIndex, i);
	}
}

void CEdit_MapObject::Render_Shadow()
{

}

void CEdit_MapObject::Set_ImGuiOption()
{
#ifdef _DEBUG
	ImGui::Text(m_ModelName);
	ImGui::SameLine();

	//현재 자기 타입 볼 수 있게, 타입 변경할 수 있게 하기.

	const _char* pObejceTType[] = { "Default","Parkour" };
	if (ImGui::BeginCombo("Object_Type", pObejceTType[ENUM_CLASS(m_eObjectType)]))
	{
		for (_uint i = 0; i < ENUM_CLASS(OBJECTTYPE::END); ++i)
		{
			if (ImGui::Selectable(pObejceTType[i]))
			{
				m_eObjectType = static_cast<OBJECTTYPE>(i);
			}
		}
		ImGui::EndCombo();
	}

	About_Parent();
	About_Transform();
	if (m_eObjectType == OBJECTTYPE::PARKOUR)
		About_Parkour();



	m_pMapInterface->Set_ShaderPass(m_pShaderCom, &m_iShaderPassIndex);
	ImGui::SameLine();
	m_pMapInterface->Set_LOD(&m_iLODIndex, &m_iNumLOD);

	if (ImGui::Button("Set Texture"))
	{
		if (!m_IsCustomTexture)
		{
			m_EntireDiffuseTextureName.clear();
			m_EntireNormalTextureName.clear();
			m_EntireMaskTextureName.clear();

		}
		m_IsCustomTexture = !m_IsCustomTexture;
	}

	ImGui::SameLine();
	ImGui::Checkbox("Custom Tex", &m_TexMode);

	if (ImGui::Button("Copy"))
		Copy_MapObject();

	if (ImGui::Button("Destroy"))
		m_isActivate = false;

	if (ImGui::Button("Destroy_All"))
	{
		m_isActivate = false;
		for (auto& pChild : m_ChildObjects)
			pChild->SetActivate(false);
	}

	About_Texture();

	
#endif
}


//void CEdit_MapObject::Set_ImGuiOption()
//{
//#ifdef _DEBUG
//	ImGui::Text(m_ModelName);
//	ImGui::SameLine();
//
//	//현재 자기 타입 볼 수 있게, 타입 변경할 수 있게 하기.
//
//	const _char* pObejceTType[] = { "Default","Sonoro","InterAction","MonsterSpawn","Destruction","NonRigid" ,"TriggerBox","NonSonoro","Sonoro_Floor" ,"Meteo","Water","Collaps","Throw" ,"Burn" ,"Dome" ,"Turn" };
//	if (ImGui::BeginCombo("Object_Type", pObejceTType[ENUM_CLASS(m_eObjectType)]))
//	{
//		for (_uint i = 0; i < ENUM_CLASS(OBJECTTYPE::END); ++i)
//		{
//			if (ImGui::Selectable(pObejceTType[i]))
//			{
//				m_eObjectType = static_cast<OBJECTTYPE>(i);
//			}
//		}
//		ImGui::EndCombo();
//	}
//
//	About_Parent();
//
//	About_Transform();
//
//	m_pMapInterface->Set_ShaderPass(m_pShaderCom, &m_iShaderPassIndex);
//	ImGui::SameLine();
//	m_pMapInterface->Set_LOD(&m_iLODIndex, &m_iNumLOD);
//
//	if (ImGui::Button("Set Texture"))
//	{
//		if (!m_IsCustomTexture)
//		{
//			m_EntireDiffuseTextureName.clear();
//			m_EntireNormalTextureName.clear();
//			m_EntireMaskTextureName.clear();
//
//		}
//		m_IsCustomTexture = !m_IsCustomTexture;
//	}
//
//	ImGui::SameLine();
//	ImGui::Checkbox("Custom Tex", &m_TexMode);
//
//	if (ImGui::Button("Copy"))
//		Copy_MapObject();
//
//	if (ImGui::Button("Destroy"))
//		m_isActivate = false;
//
//	if (ImGui::Button("Destroy_All"))
//	{
//		m_isActivate = false;
//		for (auto& pChild : m_ChildObjects)
//			pChild->SetActivate(false);
//	}
//
//	About_Texture();
//
//	/*if (ImGui::BeginCombo("Sound Tag", m_SoundTags[m_iPickedSoundTag].c_str()))
//	{
//		for (_uint i = 0; i < ENUM_CLASS(OBJECTTYPE::END); ++i)
//		{
//			if (ImGui::Selectable(m_SoundTags[i].c_str()))
//			{
//				m_iPickedSoundTag = i;
//			}
//		}
//		ImGui::EndCombo();
//	}
//	ImGui::InputFloat("Sound Volume", &m_fDynamicVolume);
//
//	if (ImGui::Button("Play Sound"))
//	{
//		m_pGameInstance->Stop_Sound_Dynamic(ENUM_CLASS(CHANNEL::ENEMY_VOICE));
//		m_pGameInstance->Play_Sound_Dynamic(StringToWString(m_SoundTags[m_iPickedSoundTag]), ENUM_CLASS(CHANNEL::ENEMY_VOICE), m_fDynamicVolume, m_pTransformCom, 0.f, 100.f, 1.f);
//	}*/
//#endif
//}


HRESULT CEdit_MapObject::Ready_Component(void* pArg)
{
	//m_pGameInstance->Wait_Thread_End();
	MAP_LOAD* pDesc = static_cast<MAP_LOAD*>(pArg);

	m_iLevel = pDesc->iLevel;

	_tchar Model[MAX_PATH] = TEXT("Prototype_Component_Model_");
	_tchar Name[MAX_PATH] = {};

	MultiByteToWideChar(CP_ACP, 0, m_ModelName, -1, Name, strlen(m_ModelName));
	lstrcat(Model, Name);
	_uint V = m_ModelName[strlen(m_ModelName) - 1] - '0' + 1;

	_wstring ModelCom = Model;
	ModelCom.pop_back();
	ModelCom.pop_back();
	ModelCom.pop_back();
	ModelCom.pop_back();
	ModelCom.pop_back();

	if (FAILED(Add_Component(pDesc->iLevel, ModelCom,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom), nullptr)))
		CRASH("FAILED");

#ifdef _DEBUG
	m_pModelCom->Ready_BoundingBox();
#endif // _DEBUG

	

	if (FAILED(__super::Add_Component(pDesc->iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		return E_FAIL;


	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);
	//m_pGameInstance->Wait_Thread_End();

	if (pDesc->pCopyObject)
		SetCopyData(pDesc->pCopyObject);

	if (pDesc->IsChild)
	{
		MAP_CREATE event(m_ModelName, this);
		m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Set_Parent"), event);
		pDesc->pParent->Add_Child(this);
		_float fDistance = 0;
		_vector RayPos = XMVector3TransformCoord(XMLoadFloat3(&CLevel_Map::m_vWorldPos), pDesc->pParent->m_pTransformCom->Get_WorldMatrix_Inv());
		_vector RayDir = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&CLevel_Map::m_vWorldDir), pDesc->pParent->m_pTransformCom->Get_WorldMatrix_Inv()));
		{
			MAP_PICK event(this, fDistance);

			m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), event);
		}
	}
	return S_OK;
}

void CEdit_MapObject::Bind_Resources()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));
}

void CEdit_MapObject::Add_Child(CEdit_MapObject* pObject)
{
	_bool Same = { true };

	if (pObject->m_pParent || pObject == this || m_pParent == pObject)
		return;

	for (auto& pChild : m_ChildObjects)
	{
		if (pChild == pObject)
		{
			Same = false;
			break;
		}
	}
	if (Same)
	{
		pObject->m_pParent = this;
		pObject->Make_ChildLocalMatrix(m_pTransformCom->Get_WorldMatrix());
		m_ChildObjects.push_back(pObject);
		m_IsParent = true;
	}
	pObject->m_IsSetParent = false;
}

void CEdit_MapObject::Quit_Child(CEdit_MapObject* pObject)
{
	m_ChildObjects.remove(pObject);
	if (m_ChildObjects.empty())
		m_IsParent = false;
}

void CEdit_MapObject::Make_ChildLocalMatrix(_fmatrix ParentMatrix)
{
	XMStoreFloat4x4(&m_ChildLocalMat, m_pTransformCom->Get_WorldMatrix() * XMMatrixInverse(nullptr, ParentMatrix));
	_matrix NewChildWolrd = XMLoadFloat4x4(&m_ChildLocalMat) * ParentMatrix;
	m_pTransformCom->Set_WorldMatrix(NewChildWolrd);
}


void CEdit_MapObject::Load_SoundTag(_string FilePath)
{
	_char ModelPath[MAX_PATH] = {};

	strcat_s(ModelPath, filesystem::current_path().parent_path().parent_path().string().c_str());


	strcat_s(ModelPath, FilePath.c_str());

	for (const auto& entry : filesystem::recursive_directory_iterator(ModelPath))
	{
		if (!entry.is_regular_file())
			continue;

		if (entry.path().extension() != ".wav")
			continue;

		_string Name = entry.path().filename().replace_extension().string();
		m_SoundTags.push_back(Name);
	}
}

void CEdit_MapObject::Export_MaterialData()
{

	//理쒖쥌 ?대뜑 寃쎈줈.

	IGFD::FileDialogConfig config1;
	_char ModelPath[MAX_PATH] = {};

	strcat_s(ModelPath, filesystem::current_path().parent_path().parent_path().string().c_str());


	strcat_s(ModelPath, "/Client/Bin/Resource/Map/");
	for (const auto& entry : filesystem::recursive_directory_iterator(ModelPath))
	{
		if (entry.path().extension() != ".dat")
			continue;
		_string Name = entry.path().filename().replace_extension().string();

		if (entry.is_regular_file() && !strcmp(Name.c_str(), m_ModelName))
		{
			strcpy_s(ModelPath, entry.path().parent_path().string().c_str());
			strcat_s(ModelPath, "/");
			break;
		}
	}

	config1.path = string(ModelPath);
	config1.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;
	_char Text[32] = {};
	sprintf_s(Text, "Export %d###Select.dat", m_iNumObject);
	ImGuiFileDialog::Instance()->OpenDialog("Export Json", Text, ".dat", config1);

	if (ImGuiFileDialog::Instance()->Display("Export Json")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			_string strFolderName = ImGuiFileDialog::Instance()->GetCurrentPath();
			_string strTexturePath = {};
			strFolderName += "/Mat/";
			strTexturePath = strFolderName;
			strFolderName += m_ModelName;
			strFolderName.pop_back();

			_string FileExt = ".json";
			_int i = 0;

			ofstream File(strFolderName + to_string(i++) + FileExt);

#pragma region Json 추출
			strTexturePath += "/Tex/";
			filesystem::create_directories(strTexturePath);
			json Totaljson;
			Totaljson["NumMaterial"] = m_pModelCom->Get_NumMesh(0);
			Totaljson["Materials"] = json::array();

			for (_uint i = 0; i < m_pModelCom->Get_NumMesh(0); ++i)
			{
				_char FileDrive[MAX_PATH] = {};
				_char FileDir[MAX_PATH] = {};
				_char DiffuseFileName[MAX_PATH] = {};
				_char NormalFileName[MAX_PATH] = {};
				_char MaskFileName[MAX_PATH] = {};
				_char MaskDiffuseFileName[MAX_PATH] = {};
				_char MaskNormalFileName[MAX_PATH] = {};
				_char FileExt[MAX_PATH] = {};

				_splitpath_s(m_SelectedDiffuseTexturePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, DiffuseFileName, MAX_PATH, FileExt, MAX_PATH);
				_splitpath_s(m_SelectedNormalTexturePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, NormalFileName, MAX_PATH, FileExt, MAX_PATH);
				if (!m_SelectedMaskTexturePath[i].empty())
				{
					_splitpath_s(m_SelectedMaskTexturePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, MaskFileName, MAX_PATH, FileExt, MAX_PATH);
					_splitpath_s(m_SelectedMaskDiffusePath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, MaskDiffuseFileName, MAX_PATH, FileExt, MAX_PATH);
					_splitpath_s(m_SelectedMaskNormalPath[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, MaskNormalFileName, MAX_PATH, FileExt, MAX_PATH);
				}

				json MaterialData;

				json DiffuseData;

				_uint TextureCnt = { 0 };
				DiffuseData["FileName"] = json::array();

				_char DiffuseName[MAX_PATH] = {};
				strcat_s(DiffuseName, DiffuseFileName);
				strcat_s(DiffuseName, FileExt);

				DiffuseData["FileName"].push_back(DiffuseName);

				if (!m_SelectedMaskTexturePath[i].empty())
				{
					_char MaskDiffuseName[MAX_PATH] = {};
					strcat_s(MaskDiffuseName, MaskDiffuseFileName);
					strcat_s(MaskDiffuseName, FileExt);

					DiffuseData["FileName"].push_back(MaskDiffuseName);
					DiffuseData["TextureCnt"] = 2;
				}
				else
					DiffuseData["TextureCnt"] = 1;

				MaterialData["Diffuse"] = DiffuseData;

				json NormalData;


				NormalData["FileName"] = json::array();

				_char NormalName[MAX_PATH] = {};
				strcat_s(NormalName, NormalFileName);
				strcat_s(NormalName, FileExt);

				NormalData["FileName"].push_back(NormalName);

				if (!m_SelectedMaskTexturePath[i].empty())
				{
					_char MaskNormalName[MAX_PATH] = {};
					strcat_s(MaskNormalName, MaskNormalFileName);
					strcat_s(MaskNormalName, FileExt);

					NormalData["FileName"].push_back(MaskNormalName);
					NormalData["TextureCnt"] = 2;
				}
				else
					NormalData["TextureCnt"] = 1;

				MaterialData["Normal"] = NormalData;

				json MaskData;
				if (!m_SelectedMaskTexturePath[i].empty())
				{
					MaskData["TextureCnt"] = 1;
					MaskData["FileName"] = json::array();

					_char MaskName[MAX_PATH] = {};
					strcat_s(MaskName, MaskFileName);
					strcat_s(MaskName, FileExt);

					MaskData["FileName"].push_back(MaskName);

					MaterialData["Mask"] = MaskData;
				}


				Totaljson["Materials"].push_back(MaterialData);

				//텍스쳐 정상화중
				filesystem::copy_file(m_SelectedDiffuseTexturePath[i], strTexturePath + DiffuseFileName + FileExt, filesystem::copy_options::overwrite_existing);
				filesystem::copy_file(m_SelectedNormalTexturePath[i], strTexturePath + NormalFileName + FileExt, filesystem::copy_options::overwrite_existing);
				if (!m_SelectedMaskTexturePath[i].empty())
				{
					filesystem::copy_file(m_SelectedMaskTexturePath[i], strTexturePath + MaskFileName + FileExt, filesystem::copy_options::overwrite_existing);
					filesystem::copy_file(m_SelectedMaskDiffusePath[i], strTexturePath + MaskDiffuseFileName + FileExt, filesystem::copy_options::overwrite_existing);
					filesystem::copy_file(m_SelectedMaskNormalPath[i], strTexturePath + MaskNormalFileName + FileExt, filesystem::copy_options::overwrite_existing);
				}
			}

			File << Totaljson.dump(4);
			File.close();

			if (m_ExportAllLOD)
			{
				for (_uint i = 1; i <= m_iNumLOD; ++i)
				{
					_string JsonName = strFolderName + to_string(i) + FileExt;
					ofstream File(JsonName);
					File << Totaljson.dump(4);
					File.close();
				}
			}

			m_MakeJson = !m_MakeJson;
			ImGuiFileDialog::Instance()->Close();

#pragma endregion
		}
		else
		{
			ImGuiFileDialog::Instance()->Close();
			m_MakeJson = !m_MakeJson;
		}
	}
}

void CEdit_MapObject::Child_UpdateMatrix(_fmatrix Matrix, _fvector vParentsPos, _fvector vDeltaTranslation)
{
	_matrix NewChildWolrd = XMLoadFloat4x4(&m_ChildLocalMat) * Matrix;
	m_pTransformCom->Set_WorldMatrix(NewChildWolrd);


	XMStoreFloat3(&m_vNewTranslation, NewChildWolrd.r[3]);

	for (auto& pChild : m_ChildObjects)
		pChild->Child_UpdateMatrix(m_pTransformCom->Get_WorldMatrix(), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation), 1.f), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation) - XMLoadFloat3(&m_vTranslation), 1.f));
}

void CEdit_MapObject::About_Parent()
{
	if (ImGui::Button("Set Parent"))
	{
		MAP_CREATE event(m_ModelName, this);
		m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Set_Parent"), event);
	}

	if (m_pParent)
	{
		ImGui::SameLine();
		if (ImGui::Button("Delete Parent"))
		{
			m_pParent->Quit_Child(this);
			m_pParent = nullptr;
		}
	}

	if (m_IsParent)
	{
		ImGuiID ShaderId = ImGui::GetID("Child");
		ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
		ImGui::Text("Child");

		for (auto& pChildObject : m_ChildObjects)
		{
			_char ChildName[MAX_PATH] = {};
			sprintf_s(ChildName, "%d", pChildObject->m_iNumObject);

			if (ImGui::Button(ChildName) || ImGui::IsItemHovered())
			{
				if (m_pPickedChild)
					m_pPickedChild->m_iShaderPassIndex = 0;
				m_pPickedChild = pChildObject;
				m_pPickedChild->m_iShaderPassIndex = 3;
			}
			else
				if (m_pPickedChild)
					m_pPickedChild->m_iShaderPassIndex = 0;
		}
		ImGui::EndChildFrame();
	}
}

void CEdit_MapObject::About_Transform()
{
	ImGui::SeparatorText("About_Transform");
	m_pMapInterface->Set_Transform(m_pTransformCom);


	for (auto& pChild : m_ChildObjects)
		pChild->Child_UpdateMatrix(m_pTransformCom->Get_WorldMatrix(), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation), 1.f), XMVectorSetW(XMLoadFloat3(&m_vNewTranslation) - XMLoadFloat3(&m_vTranslation), 1.f));

	if (m_pParent)
		Make_ChildLocalMatrix(dynamic_cast<CTransform*>(m_pParent->Get_Component(TEXT("Com_Transform")))->Get_WorldMatrix());
}

void CEdit_MapObject::About_Parkour()
{
	ImGui::SeparatorText("About_Parkour");

	_uint* pFlag = reinterpret_cast<_uint*>(&m_eParkourFlag);

	ImGui::CheckboxFlags("Vaultable", pFlag, ENUM_CLASS(PARKOUR_FLAG::VAULTABLE));
	ImGui::CheckboxFlags("Climbable", pFlag, ENUM_CLASS(PARKOUR_FLAG::CLIMBABLE));
	ImGui::CheckboxFlags("Hangable", pFlag, ENUM_CLASS(PARKOUR_FLAG::HANGABLE));
	ImGui::CheckboxFlags("HighMantleable", pFlag, ENUM_CLASS(PARKOUR_FLAG::HIGH_MANTLEABLE));
	ImGui::CheckboxFlags("Wallrunnable", pFlag, ENUM_CLASS(PARKOUR_FLAG::WALLRUNNABLE));
	ImGui::CheckboxFlags("LowMantleable", pFlag, ENUM_CLASS(PARKOUR_FLAG::LOW_MANTLEABLE));
}

void CEdit_MapObject::About_Texture()
{
	ImGui::SeparatorText("About_Texture");
	if (m_IsCustomTexture)
	{
		IGFD::FileDialogConfig config;

		//원래 ㅅ쓰던거
		//config.path = filesystem::current_path().parent_path().parent_path().parent_path().string();

		//When Many Model Need Same texture, use this
		//config.path = "C:/Users/dnheu/Downloads/FModel/Output/Exports/Client/Content/Aki/Scene/Assets/Levels/LiNaXiTa/QiQiu/Common/Obj/Tex";
		config.path = "C:/Hilde/Wuthering_Wave_Final/Client/Bin/Resource/Map/Test/Heaven_Tex";
		//config.path = "C:/Users/dnheu/Downloads/FModel/Output/Exports/Client/Content/Aki/Scene/Assets/Levels/LiNaXiTa/DiSiTaiDi/Rock/";

		for (_uint i = 0; i < m_pModelCom->Get_NumMesh(0); ++i)
		{

			//m_SelectedDiffuseTexturePath[i] = "C:/Users/dnheu/Downloads/FModel/Output/Exports/Client/Content/Aki/Scene/Assets/Levels/LiNaXiTa/DiSiTaiDi/Rock/Json_Texture/T4_Com2_Roc_05A_D.png";
			//m_SelectedMaskDiffusePath[i] = "C:/Users/dnheu/Downloads/FModel/Output/Exports/Client/Content/Aki/Scene/Assets/Levels/LiNaXiTa/DiSiTaiDi/Rock/Json_Texture/T4_Sev_Roc_01A_D.png";
		//m_SelectedDiffuseTexturePath[i] = "C:/Users/dnheu/Downloads/FModel/Output/Exports/Client/Content/Aki/Scene/Assets/Levels/LiNaXiTa/DiSiTaiDi/Rock/Tex/T_Tab_Roc_25A_D.png";
			//m_SelectedNormalTexturePath[i] = "C:/Users/dnheu/Downloads/FModel/Output/Exports/Client/Content/Aki/Scene/Assets/Levels/LiNaXiTa/DiSiTaiDi/Rock/Tex/T_Tab_Roc_25A_N.png";
		}
		config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

		_char Text[32] = {};
		sprintf_s(Text, "Object %d###Texture Load", m_iNumObject);
		ImGuiFileDialog::Instance()->OpenDialog(Text, "Import File", nullptr, config);

		if (ImGuiFileDialog::Instance()->Display(Text)) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				_string strFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
				for (const auto& entry : filesystem::recursive_directory_iterator(strFolderPath)) {
					if (entry.is_regular_file()) {
						filesystem::path Filepath = entry.path();
						filesystem::path fileName = Filepath.filename();
						if (fileName.extension() != ".png")
							continue;

						if ((fileName.string().find("_D_") != std::string::npos) || (fileName.string().find("_D") != std::string::npos))
						{
							if ((fileName.string().find("_Doo") != std::string::npos) || (fileName.string().find("_Des") != std::string::npos) || (fileName.string().find("_Dec") != std::string::npos))
							{
								if (fileName.string().find("_N_") != std::string::npos || (fileName.string().find("_N") != std::string::npos))
								{
									m_EntireNormalTextureName.push_back(Filepath.string());
								}
								else if (fileName.string().find("_MA_") != std::string::npos || (fileName.string().find("_M") != std::string::npos))
								{
									m_EntireMaskTextureName.push_back(Filepath.string());
								}
								else
									m_EntireDiffuseTextureName.push_back(Filepath.string());
							}
							else
								m_EntireDiffuseTextureName.push_back(Filepath.string());
						}
						else if (fileName.string().find("_N_") != std::string::npos || (fileName.string().find("_N") != std::string::npos))
						{
							m_EntireNormalTextureName.push_back(Filepath.string());
						}
						else if (fileName.string().find("_MA_") != std::string::npos || (fileName.string().find("_M") != std::string::npos))
						{
							m_EntireMaskTextureName.push_back(Filepath.string());
						}
					}
				}

				m_IsLoaded = true;
				m_IsCustomTexture = !m_IsCustomTexture;
				ImGuiFileDialog::Instance()->Close();
			}
			else
			{
				ImGuiFileDialog::Instance()->Close();
				m_IsCustomTexture = !m_IsCustomTexture;
			}
		}
	}


	if (m_IsLoaded)
	{
		ImGui::Begin("Texture Change");

		_char LOD_Index[10] = {};

		if (ImGui::BeginCombo("Meshes", m_iSelectedMeshName.c_str()))
		{
			for (_uint i = 0; i < m_pModelCom->Get_NumMesh(0); ++i)
            {
                sprintf_s(LOD_Index, "Mesh : %d", i);
                if (ImGui::Selectable(LOD_Index))
                {
                    m_iSelectedMesh = i;
                    m_iSelectedMeshName = LOD_Index;

                }
            }
            ImGui::EndCombo();
        }

        //m_SelectedDiffuseTexturePath[m_iSelectedMesh] = "C:/Users/dnheu/source/repos/Wuthering_Wave_Final/Client/Bin/Resource/Map/The_False_Sovereign/Rock/SM_Tab_Roc_04AH/Mat/Tex/T_Tab_Roc_25A_D.png";

        if (ImGui::BeginCombo("Diffuse", m_SelectedDiffuseName[m_iSelectedMesh].c_str()))
        {
            for (_uint i = 0; i < m_EntireDiffuseTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_EntireDiffuseTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);
                if (ImGui::Selectable(FileName) || ImGui::IsItemHovered())
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedDiffuse = m_EntireDiffuseTextureName[i];


                    _wstring Test(m_SelectedDiffuse.begin(), m_SelectedDiffuse.end());
                    if (m_pDiffuseTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pDiffuseTextureCom[m_iSelectedMesh]);

                    m_pDiffuseTextureCom[m_iSelectedMesh] = (CTexture::Create(m_pDevice, m_pContext, Test.c_str(), 1));
                    m_SelectedDiffuseTexturePath[m_iSelectedMesh] = m_EntireDiffuseTextureName[i].c_str();


                    m_SelectedDiffuseName[m_iSelectedMesh] = FileName;
                    m_iSelectedDiffuseIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Normal", m_SelectedNormalName[m_iSelectedMesh].c_str()))
        {

            for (_uint i = 0; i < m_EntireNormalTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_EntireNormalTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                if (ImGui::Selectable(FileName) || ImGui::IsItemHovered())
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedNormal = m_EntireNormalTextureName[i].c_str();

                    //W?ㅽ듃留곸쑝濡?諛붽퓭.
                    _wstring Test(m_SelectedNormal.begin(), m_SelectedNormal.end());
                    if (m_pNormalTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pNormalTextureCom[m_iSelectedMesh]);

                    m_pNormalTextureCom[m_iSelectedMesh] = (CTexture::Create(m_pDevice, m_pContext, Test.c_str(), 1));
                    m_SelectedNormalTexturePath[m_iSelectedMesh] = m_EntireNormalTextureName[i].c_str();

                    m_SelectedNormalName[m_iSelectedMesh] = FileName;
                    m_iSelectedNormalIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Mask", m_SelectedMaskTextureName[m_iSelectedMesh].c_str()))
        {

            for (_uint i = 0; i < m_EntireMaskTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_EntireMaskTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                if (ImGui::Selectable(FileName) || ImGui::IsItemHovered())
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedMask = m_EntireMaskTextureName[i].c_str();

                    //W��Ʈ������ �ٲ�.
                    _wstring Test(m_SelectedMask.begin(), m_SelectedMask.end());
                    if (m_pMaskTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pMaskTextureCom[m_iSelectedMesh]);

                    m_pMaskTextureCom[m_iSelectedMesh] = (CTexture::Create(m_pDevice, m_pContext, Test.c_str(), 1));
                    m_SelectedMaskTexturePath[m_iSelectedMesh] = m_EntireMaskTextureName[i].c_str();

                    m_SelectedMaskTextureName[m_iSelectedMesh] = FileName;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("X###Mask"))
        {
            if (m_pMaskTextureCom[m_iSelectedMesh])
                Safe_Release(m_pMaskTextureCom[m_iSelectedMesh]);

            m_SelectedMaskTexturePath[m_iSelectedMesh] = "";
            m_SelectedMaskTextureName[m_iSelectedMesh] = "";
        }

        if (ImGui::BeginCombo("MaskDiffuse", m_SelectedMaskDiffuseName[m_iSelectedMesh].c_str()))
        {

            for (_uint i = 0; i < m_EntireDiffuseTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_EntireDiffuseTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                if (ImGui::Selectable(FileName) || ImGui::IsItemHovered())
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedMaskDiffuse = m_EntireDiffuseTextureName[i].c_str();

                    //W��Ʈ������ �ٲ�.
                    if (m_pMaskDiffuseTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pMaskDiffuseTextureCom[m_iSelectedMesh]);

                    m_pMaskDiffuseTextureCom[m_iSelectedMesh] = CTexture::Create(m_pDevice, m_pContext, StringToWString(m_SelectedMaskDiffuse).c_str(), 1);
                    m_SelectedMaskDiffusePath[m_iSelectedMesh] = m_EntireDiffuseTextureName[i].c_str();

                    m_SelectedMaskDiffuseName[m_iSelectedMesh] = FileName;
                    m_iSelectedMaskIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::Button("X###MaskDiffuse"))
        {
            if (m_pMaskDiffuseTextureCom[m_iSelectedMesh])
                Safe_Release(m_pMaskDiffuseTextureCom[m_iSelectedMesh]);

            m_SelectedMaskDiffusePath[m_iSelectedMesh] = "";
            m_SelectedMaskDiffuseName[m_iSelectedMesh] = "";
        }

        if (ImGui::BeginCombo("MaskNormal", m_SelectedMaskNormalName[m_iSelectedMesh].c_str()))
        {

            for (_uint i = 0; i < m_EntireNormalTextureName.size(); ++i)
            {
                _char FileDrive[MAX_PATH] = {};
                _char FileDir[MAX_PATH] = {};
                _char FileName[MAX_PATH] = {};
                _char FileExt[MAX_PATH] = {};
                _splitpath_s(m_EntireNormalTextureName[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                if (ImGui::Selectable(FileName) || ImGui::IsItemHovered())
                {
                    ImGui::SetItemDefaultFocus();
                    m_SelectedMaskNormal = m_EntireNormalTextureName[i].c_str();

                    //W��Ʈ������ �ٲ�.
                    if (m_pMaskNormalTextureCom[m_iSelectedMesh])
                        Safe_Release(m_pMaskNormalTextureCom[m_iSelectedMesh]);

                    m_pMaskNormalTextureCom[m_iSelectedMesh] = CTexture::Create(m_pDevice, m_pContext, StringToWString(m_SelectedMaskNormal).c_str(), 1);
                    m_SelectedMaskNormalPath[m_iSelectedMesh] = m_EntireNormalTextureName[i].c_str();

                    m_SelectedMaskNormalName[m_iSelectedMesh] = FileName;
                    m_iSelectedMaskIndex[m_iSelectedMesh] = i;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::Button("X###MaskNormal"))
        {
            if (m_pMaskNormalTextureCom[m_iSelectedMesh])
                Safe_Release(m_pMaskNormalTextureCom[m_iSelectedMesh]);

            m_SelectedMaskNormalPath[m_iSelectedMesh] = "";
            m_SelectedMaskNormalName[m_iSelectedMesh] = "";
        }

        if (ImGui::Button("Make_Json"))
            m_MakeJson = !m_MakeJson;

        ImGui::SameLine();
        ImGui::Checkbox("Export All LOD Level", &m_ExportAllLOD);

        if (m_MakeJson)
            Export_MaterialData();

        ImGui::Begin("Textures", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

        ImVec2 ImageSize = ImVec2(256.f, 256.f);
        m_pMapInterface->Display_Textures(m_pDiffuseTextureCom[m_iSelectedMesh]);
        ImGui::SameLine();
        m_pMapInterface->Display_Textures(m_pNormalTextureCom[m_iSelectedMesh]);

        m_pMapInterface->Display_Textures(m_pMaskTextureCom[m_iSelectedMesh]);
        ImGui::SameLine();
        m_pMapInterface->Display_Textures(m_pMaskDiffuseTextureCom[m_iSelectedMesh]);
        ImGui::SameLine();
        m_pMapInterface->Display_Textures(m_pMaskNormalTextureCom[m_iSelectedMesh]);

        ImGui::End();

        ImGui::End();
    }

}

void CEdit_MapObject::Copy_MapObject(_bool IsChild, CEdit_MapObject* pParent)
{
	MAP_LOAD Desc{};
	Desc.eObjectType = m_eObjectType;
	Desc.iLevel = m_iLevel;
	Desc.iShaderPassIndex = m_iShaderPassIndex;
	strcpy_s(Desc.ModelName, m_ModelName);
	_float4x4 WorldPos;
	XMStoreFloat4x4(&WorldPos, m_pTransformCom->Get_WorldMatrix());
	Desc.WorldMatrix = &WorldPos;
	Desc.pCopyObject = this;
	Desc.IsChild = IsChild;
	Desc.eObjectType = m_eObjectType;
	Desc.eParkourFlag = m_eParkourFlag;

	if (Desc.IsChild)
		Desc.pParent = pParent;

	m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
		, m_iLevel, TEXT("Layer_MapObject"), &Desc);
}

void CEdit_MapObject::SetCopyData(CEdit_MapObject* pParent)
{
	for (auto& pChild : pParent->m_ChildObjects)
		pChild->Copy_MapObject(true, this);
}

CEdit_MapObject* CEdit_MapObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CEdit_MapObject* pInstance = new CEdit_MapObject(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Create : MapObject");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CEdit_MapObject::Clone(void* pArg)
{
    CEdit_MapObject* pInstance = new CEdit_MapObject(*this);

    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Create : MapObject (Clone)");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CEdit_MapObject::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pRigidbodyCom);

    Safe_Delete(m_iSelectedDiffuseIndex);
    Safe_Delete(m_iSelectedNormalIndex);
    Safe_Delete(m_iSelectedMaskIndex);
    Safe_Delete(m_iSelectedMaskDiffuseIndex);
    Safe_Delete(m_iSelectedMaskNormalIndex);

    m_pParent = nullptr;
    m_pPickedChild = nullptr;
    Safe_Release(m_pMapInterface);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pModelCom);

    for (auto& pTexture : m_pDiffuseTextureCom)
        if (pTexture)
            Safe_Release(pTexture);
    
    for (auto& pTexture : m_pNormalTextureCom)
        if (pTexture)
            Safe_Release(pTexture);

    for (auto& pTexture : m_pMaskTextureCom)
        if (pTexture)
            Safe_Release(pTexture);

    for (auto& pTexture : m_pMaskDiffuseTextureCom)
        if (pTexture)
            Safe_Release(pTexture);

    for (auto& pTexture : m_pMaskNormalTextureCom)
        if (pTexture)
            Safe_Release(pTexture);

    for (auto& pChild : m_ChildObjects)
        pChild = nullptr;
}