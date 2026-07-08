#include "EditorPch.h"
#include "Level_Map.h"

#include "Event_Level.h"
#include "Edit_MapObject.h"
#include "Edit_MapObject_Instance.h"
#include "Edit_PreViewModel.h"
#include "Edit_LightObject.h"
#include "Edit_Brush.h"
#include "Shader_Interface.h"
#include "AnimationTool.h"
#include "Edit_MapObject_Destruction_Piece.h"
#include "Edit_TriggerBox.h"
#include "Mesh_Instance.h"
#include "Edit_MonsterSpawnor.h"
#include "Edit_Meteo.h"
#include "Model_Streaming.h"
#include "Edit_MapObject_Water.h"
#include "Edit_MapObject_Collaps.h"
#include "Edit_LightManager.h"
#include "Edit_MapEffectCollector.h"
#include "Edit_FireFly_Manager.h"
#include "Edit_SlideZone.h"
#include "Edit_Portal.h"

_float3 CLevel_Map::m_vWorldPos = {};
_float3 CLevel_Map:: m_vWorldDir = {};
_float4 CLevel_Map::m_vPickedPos = _float4(0.f,0.f,0.f,1.f);

CLevel_Map::CLevel_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{

}

HRESULT CLevel_Map::Initialize()
{
	Ready_Event();
	//m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(4000, 4000,4000));
	if (FAILED(Ready_Static_Component()))
		return E_FAIL;
	m_pFlyManager = CEdit_FireFly_Manager::Create(m_pDevice, m_pContext);

	//ImGui::GetIO().DisplayFramebufferScale = ImVec2(1.25f, 1.25f);
	pShaderInterface = CShader_Interface::Create(m_pDevice, m_pContext);

	LEVEL m_eCurLevel = LEVEL::MAP;
	m_pAnimationTool = CAnimationTool::Create(m_pDevice, m_pContext, m_eCurLevel);
	m_pEffectCollector = CEdit_MapEffectCollector::Create();

	SHADER_MACRO eShaderMacro = {
	    {"THREAD_X", "64" }
	    ,{"THREAD_Y", "1" }
	    ,{"THREAD_Z", "1" }
	    , { NULL, NULL }
	};

	string strEntryPoint = "CSMain";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"),
	    CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMesh.hlsl")
	        , eShaderMacro, strEntryPoint))))
	{
	    CRASH("Failed Load AnimMesh Shader");
	    return E_FAIL;
	}


	m_eObjectType = ENUM_CLASS(OBJECTTYPE::DEFAULT);

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_TriggerBox"),
		CEdit_TriggerBox::Create(m_pDevice, m_pContext));

	m_pPickedSpawnor = CEdit_MonsterSpawnor::Create(m_pDevice, m_pContext);

	Safe_AddRef(m_pPickedSpawnor);
	return S_OK;
}

void CLevel_Map::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Get_DIKeyState(DIK_PGUP) == KEYSTATE::DOWN)
		m_FireFly = !m_FireFly;

	if (m_pGameInstance->Get_DIKeyState(DIK_INSERT) == KEYSTATE::DOWN)
		m_Effect = !m_Effect;

    m_fNearDistance = FLT_MAX;
    m_fNearDistance_Instance = FLT_MAX;

    SetWindowText(g_hWnd, TEXT("Map"));
    Menu_Select();
	if (m_Effect)
		m_pEffectCollector->Set_ImGuiOption();
	if (m_FireFly)
		m_pFlyManager->Set_ImGuiOption();

    switch (m_eMenu)
    {
    case Editor::CLevel_Map::MENU_OBJECT:
        Menu_Object();
        break;

    case Editor::CLevel_Map::MENU_RANDSCAPE:
        m_pBrush->Priority_Update(fTimeDelta);
        Menu_RandSacpe();
        m_pBrush->Update(fTimeDelta);
        break;

    case Editor::CLevel_Map::MENU_LIGHT:
        Menu_Light();
        break;

    case Editor::CLevel_Map::MENU_MAPSAVELOAD:
        Menu_Save_Load();
        break;
    case Editor::CLevel_Map::MENU_OBJECTLOAD:
        Menu_Model_Load();
        break;
    case Editor::CLevel_Map::MENU_OBJECTTYPE:
        Menu_Object_Type();
        break;
        
    }

	if (m_pGameInstance->Get_DIKeyState(DIK_U) == KEYSTATE::DOWN)
	{
		_vector Min = XMVectorSet(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
		_vector Max = XMVectorSet(FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN);
		MAP_BOUND event(&Min, &Max);
		m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Calc_Size"), event);

		int a = 0;
	}

    Make_MousePos();
    pShaderInterface->Update_Shadow();
    if (m_pGameInstance->Get_DIKeyState(DIK_GRAVE) == KEYSTATE::PRESS && m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
        m_pPickedObject = nullptr;
}

void CLevel_Map::Render()
{
    m_pAnimationTool->Render();
}

void CLevel_Map::Menu_Select()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::MenuItem("Object")) {
            m_eMenu == MENU_OBJECT ? m_eMenu = END : m_eMenu = MENU_OBJECT;
        }

        if (ImGui::MenuItem("RandScape")) {
            m_eMenu == MENU_RANDSCAPE ? m_eMenu = END : m_eMenu = MENU_RANDSCAPE;
        }

        if (ImGui::MenuItem("Light")) {
            m_eMenu == MENU_LIGHT ? m_eMenu = END : m_eMenu = MENU_LIGHT;
        }
        
        if (ImGui::MenuItem("Map Save & Load")) {
            m_eMenu == MENU_MAPSAVELOAD ? m_eMenu = END : m_eMenu = MENU_MAPSAVELOAD;
        }

        if(ImGui::MenuItem("Object Load")) {
            m_eMenu == MENU_OBJECTLOAD ? m_eMenu = END : m_eMenu = MENU_OBJECTLOAD;
        }
        if (ImGui::MenuItem("Object Type")) {
            m_eMenu == MENU_OBJECTTYPE ? m_eMenu = END : m_eMenu = MENU_OBJECTTYPE;
        }
        _float4 CamPos = *m_pGameInstance->Get_CamPos();
        _char szCamPos[64] = {};
        sprintf_s(szCamPos, "Cam Pos - X : %.1f, Y : %.1f, Z : %.1f", CamPos.x, CamPos.y, CamPos.z);

        ImGui::Text(szCamPos);

        sprintf_s(szCamPos, "Picked Pos - X : %.1f, Y : %.1f, Z : %.1f", m_vPickedPos.x, m_vPickedPos.y, m_vPickedPos.z);
        ImGui::Text(szCamPos);
        ImGui::EndMainMenuBar();
    }
}

//void CLevel_Map::Menu_Object()
//{
//	ImGui::Begin("Menu_Object");
//
//	switch (m_eObjectType)
//	{
//	case static_cast<_uint>(OBJECTTYPE::TRIGGERBOX):
//	{
//		ImGui::Checkbox("Trigger | Slide", &m_ManageTrigger);
//
//		if(m_ManageTrigger)
//		{
//			ImGui::Text("Current Triggers");
//
//			ImGuiID ShaderId = ImGui::GetID("TriggerBox");
//			ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
//
//			for (_uint i = 0; i < m_SaveObjects["Map_Object_TriggerBox"].size(); ++i)
//			{
//				if (ImGui::Button(to_string(i).c_str())) {
//					m_pPickedTriggerBox = dynamic_cast<CEdit_TriggerBox*>(m_SaveObjects["Map_Object_TriggerBox"][i]);
//				}
//			}
//			ImGui::EndChildFrame();
//
//			if (m_pPickedTriggerBox)
//				m_pPickedTriggerBox->Set_ImGuiOption();
//
//			Create_TriggerBox();
//		}
//		else
//		{
//			ImGui::Text("Current Slide");
//
//			ImGuiID ShaderId = ImGui::GetID("Slide");
//			ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
//
//			for (_uint i = 0; i < m_SaveObjects["Map_Object_SlideBox"].size(); ++i)
//			{
//				if (ImGui::Button(to_string(i).c_str())) {
//					m_pPickedSlideBox = dynamic_cast<CEdit_SlideZone*>(m_SaveObjects["Map_Object_SlideBox"][i]);
//				}
//			}
//			ImGui::EndChildFrame();
//
//			if (m_pPickedSlideBox)
//				m_pPickedSlideBox->Set_ImGuiOption();
//
//			Create_SlideBox();
//		}
//	}
//		break;
//	case static_cast<_uint>(OBJECTTYPE::SPAWNOR):
//		if (m_pPickedSpawnor)
//			m_pPickedSpawnor->Set_ImGuiOption();
//		break;
//	case static_cast<_uint>(OBJECTTYPE::METEO):
//		if(m_pPickedMeteo)
//			m_pPickedMeteo->Set_ImGuiOption();
//		break;
//	case static_cast<_uint>(OBJECTTYPE::WATER):
//		if (m_pPickedWater)
//			m_pPickedWater->Set_ImGuiOption();
//		break;
//	case static_cast<_uint>(OBJECTTYPE::COLLAPS):
//		if (m_pPickedCollaps)
//			m_pPickedCollaps->Set_ImGuiOption();
//		break;
//	default:
//		if (m_pPickedObject)
//			m_pPickedObject->Set_ImGuiOption();
//		else if (m_pPickedDestructObject)
//			m_pPickedDestructObject->Set_ImGuiOption();
//		break;
//	}
//    ImGui::End();
//}

void CLevel_Map::Menu_Object()
{
	ImGui::Begin("Menu_Object");
	if (m_pPickedObject)
		m_pPickedObject->Set_ImGuiOption();
	
	ImGui::End();
}

void CLevel_Map::Menu_RandSacpe()
{
    ImGui::Begin("Menu_RandScape");


    Load_Foliage();
    ImGui::Button("Undo");
    if (!m_SaveObjects["Map_Object_Instance"].empty())
    {
        if (ImGui::IsItemHovered() && m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::PRESS)
        {

            CEdit_MapObject_Instance* pObject = dynamic_cast<CEdit_MapObject_Instance*>(m_SaveObjects["Map_Object_Instance"].back());
            pObject->SetActivate(false);
            Safe_Release(pObject);
            m_SaveObjects["Map_Object_Instance"].pop_back();
        }
    }

    if (m_pPickedInstanceObject)
        m_pPickedInstanceObject->Set_ImGuiOption();
    ImGui::End();
}

void CLevel_Map::Menu_Light()
{
	ImGui::Begin("Menu_Light");
	m_pLightManager->Set_ImGuiOption();
	ImGui::End();
}

void CLevel_Map::Menu_Model_Load()
{
    ImGui::Begin("Model Table", nullptr, ImGuiWindowFlags_NoTitleBar);
    if (ImGui::BeginTable("Test", 1, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TableNextColumn();

        for (_uint i = 0; i < m_ModelPaths.size(); ++i)
        {
            _char FileDrive[MAX_PATH] = {};
            _char FileDir[MAX_PATH] = {};

            _char FileName[MAX_PATH] = {};
            _char FileExt[MAX_PATH] = {};
            _splitpath_s(m_ModelPaths[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

			if (ImGui::Selectable(FileName))
			{
				auto iter = m_szPrototypeName.find(FileDir);
				if (iter == m_szPrototypeName.end())
				{
					m_szPrototypeName.insert(FileDir);

					_string NoVersionName = FileName;
					NoVersionName.pop_back();

					//뒤 숫자 떼고 0부터 숫자까지 만들기. 이미 맨 뒤에 .dat 붙어있음.

					_uint V = FileName[strlen(FileName) - 1] - '0' + 1;
					_wstring PrototypeName;

					PrototypeName = L"Prototype_Component_Model_";
					PrototypeName += StringToWString(NoVersionName);

					PrototypeName.pop_back();
					PrototypeName.pop_back();
					PrototypeName.pop_back();
					PrototypeName.pop_back();

					if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, PrototypeName,
						CModel_Streaming::Create(m_pDevice, m_pContext, FileDir))))
						CRASH("Prototype Create Failed");
					m_pGameInstance->LoadLastLOD();
				}

				CEdit_MapObject::MAP_LOAD Desc{};
				_float4x4 DefaultMatrix{};
				XMStoreFloat4x4(&DefaultMatrix, XMMatrixTranslationFromVector(XMLoadFloat4(&m_vPickedPos)));
				Desc.WorldMatrix = &DefaultMatrix;
				strcpy_s(Desc.ModelName, FileName);
				Desc.eObjectType = static_cast<OBJECTTYPE>(m_eObjectType);
				Desc.iLevel = m_iLevel;

				m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
					, m_iLevel, TEXT("Layer_MapObject"), &Desc);
			}

            if (ImGui::IsItemHovered())
            {
                ImGui::Begin("PreView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
                m_szPreViewModelName = StringToWString(FileName);
#ifdef _DEBUG
                ImGui::Image(m_pGameInstance->Get_Debug_RT_Resource(TEXT("RT_Debug")), ImVec2(128, 128));
#endif
                ImGui::End();
                m_pPreViewObject->Late_Update(0.016f, m_szPreViewModelName);
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}

void CLevel_Map::Menu_Object_Type()
{
    ImGui::Begin("Type");

	const _char* pObejceTType[] = { "Default","Parkour"};

    if (ImGui::BeginCombo("Object_Type", pObejceTType[m_eObjectType]))
    {
        for (_uint i = 0; i < ENUM_CLASS(OBJECTTYPE::END); ++i)
        {
            if (ImGui::Selectable(pObejceTType[i]))
            {
                m_eObjectType = static_cast<_uint>(i);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::End();
}

void CLevel_Map::Menu_Save_Load()
{
    IGFD::FileDialogConfig config;

    config.path = "../../Client/Bin/Resource/Map/MapData/";
    config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

    static _char exportText[128] = "";
	// 폴더명을 입력하고 Save_All을 해야 해당 폴더에 .dat 파일이 저장된다.
    ImGui::InputText("<Input>/<Input>_<Category>.dat", exportText, IM_ARRAYSIZE(exportText));

    if (ImGui::BeginMenu("Save"))
    {
        for (auto& Pair : m_SaveObjects)
            if (ImGui::MenuItem(Pair.first.c_str()))
            {
                string MapName = config.path;
                MapName += exportText;
                MapName += "/";
                if (filesystem::exists(MapName))
                    filesystem::create_directories(MapName);
                MapName += exportText;
                MapName += "_";
                MapName += Pair.first;
                MapName += ".dat";
                unordered_set<_string> Test;
                ofstream File(MapName, ios::binary);

                MAP_SAVE event(File, Test);
                File.close();
            }

        ImGui::EndMenu();
    }
	if (ImGui::BeginMenu("Save All"))
	{
		if (ImGui::MenuItem("Save All Check"))
		{
			unordered_set<_string> UsingPrototypeNames;
			for (auto& Pair : m_SaveObjects)
			{
				string MapName = config.path;
				MapName += exportText;
				MapName += "/";
				if (!filesystem::exists(MapName))
					filesystem::create_directories(MapName);
				MapName += exportText;
				MapName += "_";
				MapName += Pair.first;
				MapName += ".dat";
				ofstream File(MapName, ios::binary);

				MAP_SAVE event(File, UsingPrototypeNames);
				m_pGameInstance->Publish(ENUM_CLASS(LEVEL::STATIC), TEXT("Save_Map"), event);
				File.flush();
				File.close();
			}
			_string PrototypeSave = config.path;
			PrototypeSave += exportText;
			PrototypeSave += "/";
			if (!filesystem::exists(PrototypeSave))
				filesystem::create_directories(PrototypeSave);
			PrototypeSave += exportText;
			PrototypeSave += "_";
			_string PrototypeSaveNames = PrototypeSave;
			PrototypeSaveNames += ".txt";
			PrototypeSave += "_Prototype.dat";
			ofstream File2(PrototypeSave, ios::binary);
			ofstream File3(PrototypeSaveNames, ios::trunc);

			_uint i = 0;
			for (const auto& Data : UsingPrototypeNames)
			{
				_uint StrSize = static_cast<_uint>(strlen(Data.c_str()));
				File2.write(reinterpret_cast<const _char*>(&StrSize), sizeof(_uint));
				File2.write(reinterpret_cast<const _char*>(Data.c_str()), StrSize);
				File3 << Data.c_str() << endl;
				i++;
			}
			File2.flush();
			File2.close();
			File3 << i << endl;
			File3.close();
		}
		ImGui::EndMenu();
	}


    ImGui::MenuItem("Load", nullptr, &m_LoadMenu);
    if (m_LoadMenu)
    {
        ImGui::Begin("Map Save & Load");

        ImGuiFileDialog::Instance()->OpenDialog("Map File Load", "Import File", ".dat", config);

        if (ImGuiFileDialog::Instance()->Display("Map File Load")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
				Ready_Map_Load_Prototype();
                _string DatFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
                for (const auto& entry : filesystem::recursive_directory_iterator(DatFolderPath)) {
                    if (entry.is_regular_file())
                    {
						_uint NameLength = {};

                        _string strFilePath = entry.path().string();
                        if (strFilePath.find("Prototype") != std::string::npos)
                            continue;
                        if (strFilePath.find(".txt") != std::string::npos)
                            continue;
                        ifstream File(strFilePath, ios::binary);

                        if (!File.is_open())
                        {
                            MSG_BOX("Load Failed");
                        }
                        else
                        {
                            CEdit_MapObject::MAP_LOAD Desc{};
							//경로 돌면서 하나하나 일일히 찾아서 경로 찾은 다음에 있는 거랑 비교한 후 없으면 프로토타입 만들기.

							while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
							{
								memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
								File.read(Desc.ModelName, NameLength);

								File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
								File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
								File.read(reinterpret_cast<char*>(&Desc.eParkourFlag), sizeof(PARKOUR_FLAG));
								_float4x4 Matrix = {};
								File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
								Desc.WorldMatrix = &Matrix;
								File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
								File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));
								_string Name = Desc.ModelName;
								ShaderChange(Name, &Desc.iShaderPassIndex);
								/*if (Desc.eObjectType == OBJECTTYPE::INTERACTION)
									int a = 0;*/

								m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
									, m_iLevel, TEXT("Layer_MapObject"), &Desc);

								//바운딩박스 누수문제로 주석.

								/*m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex, eObjectType = Desc.eObjectType, Matrix = *Desc.WorldMatrix]() mutable {
									CEdit_MapObject::MAP_LOAD pDesc{};
									strcpy_s(pDesc.ModelName, ModelName.c_str());
									pDesc.iShaderPassIndex = ShaderPass;
									pDesc.eObjectType = eObjectType;
									pDesc.WorldMatrix = &Matrix;
									pDesc.iLevel = m_iLevel;

									m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
										, m_iLevel, TEXT("Layer_MapObject"), &pDesc);
									});*/

							}
                        }
						m_pGameInstance->Wait_Thread_End();
						File.close();
                    }
                }
                m_LoadMenu = !m_LoadMenu;
                ImGuiFileDialog::Instance()->Close();
            }
            else
            {
                m_LoadMenu = !m_LoadMenu;
                ImGuiFileDialog::Instance()->Close();
            }
        }
        ImGui::End();
    }
}

// 시작과 동시에 경로 상의 오브젝트들을 가져옵니다.
void CLevel_Map::Load_Objects()
{
    m_ModelPaths.clear();

    m_pPreViewObject = CEdit_PreViewModel::Create(m_pDevice, m_pContext);
	//m_FolderPath = "../../Client/Bin/Resource/Map/Asphodel_Barrens/";
	//m_FolderPath = "../../Client/Bin/Resource/Map/Test/";
	//m_FolderPath= "../../Client/Bin/Resource/Map/Logo/";
	//m_FolderPath = "../../Client/Bin/Resource/Map/The_False_Sovereign/";
	//m_FolderPath = "../../Client/Bin/Resource/Map/Test/Heaven_Deco/";
	//m_FolderPath = "../../Client/Bin/Resource/Map/Test/Heaven/";
	//m_FolderPath = "../../Client/Bin/Resource/Map/Heaven/";
	//m_FolderPath = "../../Client/Bin/Resource/Map/Test/Heaven_Interaction/";
	//m_FolderPath = "../../Client/Bin/Resource/Map/Test/Heaven_Foliage/";
	m_FolderPath = "../../Client/Bin/Resource/Map/Parkour/";

    vector<_wstring> m_PrototypeNames;
    vector<_wstring> m_FoliageNames;

    _matrix PreTransformMatrix = XMMatrixIdentity();
    _float fSize = 0.01f;
    //_float fSize = 0.02f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

    _int version={};
    _int Lastversion = {};
    _wstring LastVersionName;
    _string LastVersionPath;

	m_pGameInstance->Load_Resource(m_FolderPath.c_str());
    //마지막 폴더 못읽음. 프로토타입 안생김.
	for (const auto& entry : filesystem::recursive_directory_iterator(m_FolderPath)) {
		if (entry.is_regular_file()) {
			if (entry.path().string().find("MapData") != std::string::npos)
				continue;

			if (entry.path().string().find("Anim") != std::string::npos)
				continue;

			if (entry.path().extension() == ".dat") {

				_char FileDrive[MAX_PATH] = {};
				_char FileDir[MAX_PATH] = {};
				_char FileName[MAX_PATH] = {};
				_char FileExt[MAX_PATH] = {};
				_splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

				_wstring PrototypeName = L"Prototype_Component_Model_";
				PrototypeName += StringToWString(FileName);

				_string VersionPath = FileDir;
				VersionPath += FileName;
				VersionPath += ".dat";

				if (entry.path().string().find("_Bone") != std::string::npos)
				{
					int a = 0;
					m_pGameInstance->Add_Work([&, ProtoName = PrototypeName, Path = VersionPath]() {
						if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
							CModel::Create(m_pDevice, m_pContext, MODELTYPE::ECO, PreTransformMatrix, Path.c_str()))))
							CRASH("Prototype Create Failed");
						});
					continue;
				}
				else if (entry.path().string().find("Bones") != std::string::npos && entry.path().string().find("_Bone") == std::string::npos)
				{
					/*m_pGameInstance->Add_Work([&, ProtoName = PrototypeName, Path = FileDir]() {
						if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
							CModel_Streaming::Create(m_pDevice, m_pContext, Path))))
							CRASH("Prototype Create Failed");*/
						if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, PrototypeName,
							CModel_Streaming::Create(m_pDevice, m_pContext, FileDir))))
							CRASH("Prototype Create Failed");
						//});
					continue;
				}
				_wstring baseName = StringToWString(FileName);

				// LOD 마지막에 붙은 숫자 추출
				size_t pos = baseName.find_last_not_of(TEXT("0123456789"));
				_wstring namePart = baseName.substr(0, pos + 1);

				_wstring numberPart = baseName.substr(pos + 1);
				if (numberPart.empty())
					continue;
				version = stoi(numberPart);

				_wstring key = L"Prototype_Component_Model_" + namePart;

				if (entry.path().string().find("Foliage") != std::string::npos)
				{
					_wstring ProtoName = TEXT("Prototype_Component_Model_Instance_");
					ProtoName += StringToWString(FileName);
					auto iter = m_szPrototypeName.find(WStringToString(ProtoName));
					if (iter != m_szPrototypeName.end())
						continue;
					m_szPrototypeName.insert(FileDir);
					m_pGameInstance->Add_Work([&, ProtoName = ProtoName, Path = VersionPath]() {
						if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
							CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, Path.c_str(),true))))
							CRASH("Prototype Create Failed");
						});
				}
				if (lstrcmp(LastVersionName.c_str(), key.c_str()) && !LastVersionName.empty())
				{
					// ✅ (수정됨) '이전' 경로를 검사합니다.
					if (LastVersionPath.find("Foliage") != std::string::npos)
					{
						// ✅ (수정됨) '이전' 버전을 사용합니다.
						m_FoliageNames.push_back(LastVersionName + to_wstring(Lastversion));
						m_FoliagePaths.push_back(LastVersionPath);
					}
					else
					{
						m_PrototypeNames.push_back(LastVersionName + to_wstring(Lastversion));
						m_ModelPaths.push_back(LastVersionPath);
					}
				}
				Lastversion = version;
				LastVersionName = key;
				LastVersionPath = VersionPath;
			}
		}
	}

	if (!LastVersionName.empty())
	{
		// ✅ (수정됨) '이전' 경로를 검사합니다.
		if (LastVersionPath.find("Foliage") != std::string::npos)
		{
			// ✅ (수정됨) '이전' 버전을 사용합니다.
			m_FoliageNames.push_back(LastVersionName + to_wstring(Lastversion));
			m_FoliagePaths.push_back(LastVersionPath);
		}
		else
		{
			m_PrototypeNames.push_back(LastVersionName + to_wstring(Lastversion));
			m_ModelPaths.push_back(LastVersionPath);
		}
	}

    m_pGameInstance->Wait_Thread_End();
    m_pGameInstance->Wait_Thread_End();

}

void CLevel_Map::Create_TriggerBox()
{
	ImGui::Text("TriggerBox Info");
	ImGui::InputFloat3("TriggerBox Pos", reinterpret_cast<_float*>(&m_vPickedPos), "%.1f");

	ImGui::InputFloat3("TriggerBox Extends", m_TriggerBoxExtends);
	if (ImGui::Button("Create"))
	{
		CEdit_TriggerBox::TRIGGER Tri;
		Tri.iLevel = m_iLevel;
		Tri.vExtends = _float3(m_TriggerBoxExtends[0], m_TriggerBoxExtends[1], m_TriggerBoxExtends[2]);
		_matrix Mat = XMMatrixTranslationFromVector(XMVectorSet(m_vPickedPos.x, m_vPickedPos.y, m_vPickedPos.z, 1.f));
		_float4x4 TT;
		XMStoreFloat4x4(&TT, Mat);
		Tri.WorldMatrix = &TT;
		m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_TriggerBox"), m_iLevel, TEXT("Layer_Trigger"), &Tri);
	}
}

void CLevel_Map::Create_SlideBox()
{
	ImGui::Text("Slide Info");
	ImGui::InputFloat3("SlidePos", reinterpret_cast<_float*>(&m_vPickedPos), "%.1f");

	ImGui::InputFloat3("Slide Extends", m_TriggerBoxExtends);
	if (ImGui::Button("Create"))
	{
		CEdit_SlideZone::SLIDE_DESC SlideDesc;
		SlideDesc.iLevel = m_iLevel;
		SlideDesc.vExtends = _float3(m_TriggerBoxExtends[0], m_TriggerBoxExtends[1], m_TriggerBoxExtends[2]);
		_matrix Mat = XMMatrixTranslationFromVector(XMVectorSet(m_vPickedPos.x, m_vPickedPos.y, m_vPickedPos.z, 1.f));
		_float4x4 TT;
		XMStoreFloat4x4(&TT, Mat);
		SlideDesc.WorldMatrix = &TT;
		m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_SlideBox"), m_iLevel, TEXT("Layer_Slide"), &SlideDesc);
	}
}

void CLevel_Map::Ready_Map_Load_Prototype()
{
	unordered_set<_string> m_Test;
	for (const auto& entry : filesystem::recursive_directory_iterator(m_FolderPath))
	{
		if (!entry.is_regular_file())
			continue;
		if (entry.path().extension() != ".dat")
			continue;
		if (entry.path().string().find("Bone") != string::npos)
			continue;
		if (entry.path().string().find("FireFly") != string::npos)
			continue;
		_char FileDrive[MAX_PATH] = {};
		_char FileDir[MAX_PATH] = {};

		_char FileName[MAX_PATH] = {};
		_char FileExt[MAX_PATH] = {};
		_splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

		auto iter2 = m_Test.find(FileDir);
		if (iter2 != m_Test.end())
			continue;
		m_Test.insert(FileDir);

		auto iter = m_szPrototypeName.find(FileDir);
		if (iter == m_szPrototypeName.end())
		{
			m_szPrototypeName.insert(FileDir);
			_string VersionName = FileName;

			_string NoVersionName = FileName;
			NoVersionName.pop_back();

			_uint V = FileName[strlen(FileName) - 1] - '0' + 1;
			_wstring PrototypeName;

			PrototypeName = L"Prototype_Component_Model_";
			PrototypeName += StringToWString(NoVersionName);

			PrototypeName.pop_back();
			PrototypeName.pop_back();
			PrototypeName.pop_back();
			PrototypeName.pop_back();
			if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, PrototypeName,
				CModel_Streaming::Create(m_pDevice, m_pContext, FileDir))))
				CRASH("Prototype Create Failed");
		}
	}
	m_pGameInstance->Wait_Thread_End();
	m_pGameInstance->LoadLastLOD();
}

void CLevel_Map::Ready_Debris_Prototype(const _char* pModelName)
{
	_string Name = pModelName;
	Name.pop_back();
	Name.pop_back();
	Name.pop_back();
	Name.pop_back();
	Name.pop_back();

	for (const auto& entry : filesystem::recursive_directory_iterator(m_FolderPath))
	{
		if (!entry.is_regular_file())
			continue;
		if (entry.path().extension() != ".dat")
			continue;
		if (entry.path().string().find(Name + "_") == string::npos)
			continue;

		auto iter = m_szPrototypeName.find(entry.path().string());
		if (iter == m_szPrototypeName.end())
		{
			m_szPrototypeName.insert(entry.path().string());

			_char FileDrive[MAX_PATH] = {};
			_char FileDir[MAX_PATH] = {};

			_char FileName[MAX_PATH] = {};
			_char FileExt[MAX_PATH] = {};
			_splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

			_string VersionName = FileName;

			_string NoVersionName = FileName;
			NoVersionName.pop_back();

			//뒤 숫자 떼고 0부터 숫자까지 만들기. 이미 맨 뒤에 .dat 붙어있음.

			_uint V = FileName[strlen(FileName) - 1] - '0' + 1;

			for (_uint i = 0; i < V; ++i)
			{
				_wstring PrototypeName = L"Prototype_Component_Model_";
				PrototypeName += StringToWString(NoVersionName);
				PrototypeName += to_wstring(i);

				_string VersionPath = FileDir;
				VersionPath += NoVersionName;
				VersionPath += to_string(i);
				VersionPath += ".dat";

				m_pGameInstance->Add_Work([=, Name = PrototypeName, Path = VersionPath]() {
					if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, Name,
						CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, XMMatrixScalingFromVector(XMVectorSet(0.01f, 0.01f, 0.01f, 1.f)), Path.c_str()))))
						CRASH("Prototype Create Failed");
					});
			}
		}
	}
	m_pGameInstance->Wait_Thread_End();
}

_bool CLevel_Map::NameCheck(const _string& ModelName, const _string& Name)
{
	return ModelName.find(Name) != string::npos;
}

void CLevel_Map::ShaderChange(const _string& ModelName, _uint* pShaderIndex)
{
	return;

#pragma region MyRegion


	if (NameCheck(ModelName, "SM_Lau_Mou_01AH"))
		*pShaderIndex = 9;
	else if (NameCheck(ModelName, "SM_Sev_Roc_21AL") ||
		NameCheck(ModelName, "SM_Sev_Roc_31AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_41AS") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_13BH") ||
		NameCheck(ModelName, "SM_Sev_Roc_27AM") ||
		NameCheck(ModelName, "SM_Sev_Roc_23AM") ||
		NameCheck(ModelName, "SM_Sev_Roc_37AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_28AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_04AM") ||
		NameCheck(ModelName, "SM_Sev_Roc_19AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_15AM") ||
		NameCheck(ModelName, "SM_Sev_Roc_20AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_38AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_16AM") ||
		NameCheck(ModelName, "SM_Sev_Roc_08AL") ||
		NameCheck(ModelName, "SM_Sev_Roc_14AM") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_15AM") ||
		NameCheck(ModelName, "SM_Sev_Roc_12AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_09AL") ||
		NameCheck(ModelName, "SM_Sev_Roc_51AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_34AL") ||
		NameCheck(ModelName, "SM_Sev_Roc_41BS") ||
		NameCheck(ModelName, "SM_Sev_Roc_10AL") ||
		NameCheck(ModelName, "SM_Sev_Roc_05AL") ||
		NameCheck(ModelName, "SM_Sev_Roc_07AL") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_07AH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_01AH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_19AM") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_10AH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_04AH") ||
		NameCheck(ModelName, "SM_Sev_Roc_11AL") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_08AH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_18BL") ||
		NameCheck(ModelName, "SM_Sev_Roc_39AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_13AM") ||
		NameCheck(ModelName, "SM_Sev_Roc_18AL") ||
		NameCheck(ModelName, "SM_Sev_Roc_50AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_44AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_03AL") ||
		NameCheck(ModelName, "SM_Sev_Roc_24BS") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_12AL") ||
		NameCheck(ModelName, "SM_Sev_Roc_36AL") ||
		NameCheck(ModelName, "SM_Sev_Roc_24AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_32AL") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_01DH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_03AH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_02BH") ||
		NameCheck(ModelName, "SM_Sev_Roc_48AS") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_23AL") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_02AH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_09AH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_01BH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_01CH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_05AH") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_10BH") ||
		NameCheck(ModelName, "SM_Sev_Roc_47AS") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_06AH") ||
		NameCheck(ModelName, "SM_Sev_Roc_40AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_46AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_49AS") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_14AM") ||
		NameCheck(ModelName, "SM_Sev_Tab_NonSonoro_Roc_15AM") ||
		NameCheck(ModelName, "SM_Sev_Tab_NonSonoro_Roc_21AL") ||
		NameCheck(ModelName, "SM_Sev_Tab_NonSonoro_Roc_38AS") ||
		NameCheck(ModelName, "SM_Tab_NonSonoro_Roc_07AH"))
	{
		*pShaderIndex = 10;
	}
	else if (NameCheck(ModelName, "SM_Com2_Roc_APD_16AM") ||
		NameCheck(ModelName, "SM_Com2_Roc_APD_16AM") ||
		NameCheck(ModelName, "SM_Com2_Roc_APD_21AM") ||
		NameCheck(ModelName, "SM_Com2_Roc_APD_06AL") ||
		NameCheck(ModelName, "SM_Com2_Roc_APD_35AS") ||
		NameCheck(ModelName, "SM_Sev_Roc_54AS") ||
		NameCheck(ModelName, "SM_Tab_NonSonoro_Roc_17AS") ||
		NameCheck(ModelName, "SM_Tab_APD_Roc_17AS") ||
		NameCheck(ModelName, "SM_Com2_NonSonoro_Roc_14AM") ||
		NameCheck(ModelName, "SM_Sev_Roc_49AS")
		)
	{
		*pShaderIndex = 11;
	}
	else if (NameCheck(ModelName, "SM_Com2_Roc_APD_27AM") ||
		NameCheck(ModelName, "SM_Com2_NonSonoro_Roc_27AM") ||
		NameCheck(ModelName, "SM_Sev_Roc_58AS"))
	{
		*pShaderIndex = 12;
	}
	else if (NameCheck(ModelName, "SM_Sev_Roc_02AL"))
	{
		*pShaderIndex = 13;
	}
	else if (NameCheck(ModelName, "SM_Com2_Roc_APD_10AM") ||
		NameCheck(ModelName, "SM_Com2_Roc_APD_39AX"))
	{
		*pShaderIndex = 14;
	}
	else if (NameCheck(ModelName, "SM_Com2_Roc_APD_10AM") ||
		NameCheck(ModelName, "SM_Tab_Roc_23AL") ||
		NameCheck(ModelName, "SM_Tab_Roc_13BH") ||
		NameCheck(ModelName, "SM_Com2_Roc_14AM") ||
		NameCheck(ModelName, "SM_Com2_Roc_39AX") ||
		NameCheck(ModelName, "SM_Tab_Roc_24AL") ||
		NameCheck(ModelName, "SM_Tab_Roc_07AH") ||
		NameCheck(ModelName, "SM_Tab_Roc_19AM") ||
		NameCheck(ModelName, "SM_Tab_Roc_28AS"))
	{
		*pShaderIndex = 15;
	}
	else if (NameCheck(ModelName, "SM_Tab_Roc_08AH") ||
		NameCheck(ModelName, "SM_Tab_Roc_04AH") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_32AL") ||
		NameCheck(ModelName, "SM_Com2_Roc_37BS") ||
		NameCheck(ModelName, "SM_Com2_Roc_38CS") ||
		NameCheck(ModelName, "SM_Tab_Roc_20BM") ||
		NameCheck(ModelName, "SM_Tab_Roc_10AH") ||
		NameCheck(ModelName, "SM_Tab_Roc_20AM") ||
		NameCheck(ModelName, "SM_Tab_Roc_17AS") ||
		NameCheck(ModelName, "SM_Tab_Roc_15AM") ||
		NameCheck(ModelName, "SM_Tab_Roc_02AH") ||
		NameCheck(ModelName, "SM_Tab_Roc_13AH") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_38AS") ||
		NameCheck(ModelName, "SM_Tab_Roc_18BL") ||
		NameCheck(ModelName, "SM_Tab_Roc_19BM") ||
		NameCheck(ModelName, "SM_Tab_Roc_22AL") ||
		NameCheck(ModelName, "SM_Tab_Roc_06AH") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_22AM") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_28AS") ||
		NameCheck(ModelName, "SM_Com2_Roc_38BS") ||
		NameCheck(ModelName, "SM_Tab_Roc_12AL") ||
		NameCheck(ModelName, "SM_Com2_Roc_03AL") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_23AM") ||
		NameCheck(ModelName, "SM_Com2_Roc_27AM") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_15AM") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_21AL") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_31AS") ||
		NameCheck(ModelName, "SM_Com2_Roc_20AS") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_12AS") ||
		NameCheck(ModelName, "SM_Com2_Roc_05AL") ||
		NameCheck(ModelName, "SM_Com2_Roc_28AS") ||
		NameCheck(ModelName, "SM_Com2_Roc_39BX") ||
		NameCheck(ModelName, "SM_Tab_Roc_22BL") ||
		NameCheck(ModelName, "SM_Tab_Roc_01EH") ||
		NameCheck(ModelName, "SM_Tab_Roc_18AL") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_05AL") ||
		NameCheck(ModelName, "SM_Com2_Roc_17AL") ||
		NameCheck(ModelName, "SM_Com2_Roc_24AS") ||
		NameCheck(ModelName, "SM_Tab_Roc_09BH") ||
		NameCheck(ModelName, "SM_Tab_Roc_09AH") ||
		NameCheck(ModelName, "SM_Tab_Roc_05AH") ||
		NameCheck(ModelName, "SM_Tab_Roc_14AM") ||
		NameCheck(ModelName, "SM_Com2_Roc_39CX") ||
		NameCheck(ModelName, "SM_Sev_Tab_Roc_37AS"))
	{
		*pShaderIndex = 16;
	}
	else if (NameCheck(ModelName, "Doo_") ||
		NameCheck(ModelName, "Cru_Bui_09DH") || 
		NameCheck(ModelName, "Cru_Bui_09CH") || 
		NameCheck(ModelName, "Cru_Bui_41AH") ||
		NameCheck(ModelName, "Cru_Bui_11CH") ||
		NameCheck(ModelName, "Cru_Bui_13AH") ||
		NameCheck(ModelName, "Cru_Bui_21") ||
		NameCheck(ModelName, "Cru_Bui_42") ||
		NameCheck(ModelName, "Cru_Bui_11BH"))
		*pShaderIndex = 17;
	//else if(NameCheck(ModelName, "SM_Sev_Bui_02"))
	//	*pShaderIndex = 0;
	//else if (NameCheck(ModelName, "Lig_"))
	//{
	//	if (ModelName.find("11BS") == string::npos)
	//		*pShaderIndex = 4;
	//	if (ModelName.find("04AS") != string::npos || ModelName.find("13") != string::npos)
	//		*pShaderIndex = 0;
	//}

#pragma endregion

}

//void CLevel_Map::Logo_Test()
//{
//	_wstring wStrModelTag = L"Prototype_Component_Model_MaleRover";
//	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Logo/Male/LogoMaleRover.dat";
//	_matrix	PreTransformMatrix = XMMatrixIdentity();
//	_float fSize = 0.01f;
//	//_float fSize = 0.0001f;
//	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(90.f));// * XMMatrixRotationY(XMConvertToRadians(180.f));
//
//	// 1. 모델 초기화.
//	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, wStrModelTag,
//		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
//		CRASH("Prototype Create Failed");
//
//
//	// 2. StateMachine 초기화
//	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_MaleRover";
//	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, wStrStateMachineTag,
//		CStateMachine::Create(m_pDevice, m_pContext))))
//		CRASH("PlayerState Machine");
//
//
//	// 3. 객체 초기화
//	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_LogoMaleRover");
//	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel
//		, wStrActorTag
//		, CLogoMaleRover::Create(m_pDevice, m_pContext))))
//		CRASH("Prototype Create Failed");
//
//	_wstring wStrModelTag = L"Prototype_Component_Model_FemaleRover";
//	_string strFilePath = "../../Client/Bin/Resource/Model/Player/Logo/Female/LogoFemaleRover.dat";
//	_matrix	PreTransformMatrix = XMMatrixIdentity();
//	_float fSize = 0.01f;
//	//_float fSize = 0.0001f;
//	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(90.f));// * XMMatrixRotationZ(XMConvertToRadians(90.f));// *  XMMatrixRotationY(XMConvertToRadians(180.f));
//
//	// 1. 모델 초기화.
//	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, wStrModelTag,
//		CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreTransformMatrix, strFilePath.c_str()))))
//		CRASH("Prototype Create Failed");
//
//
//	// 2. StateMachine 초기화
//	_wstring wStrStateMachineTag = L"Prototype_Component_StateMachine_FemaleRover";
//	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, wStrStateMachineTag,
//		CStateMachine::Create(m_pDevice, m_pContext))))
//		CRASH("PlayerState Machine");
//
//
//	// 3. 객체 초기화
//	_wstring wStrActorTag = TEXT("Prototype_GameObject_Actor_LogoFemaleRover");
//	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel
//		, wStrActorTag
//		, CLogoFemaleRover::Create(m_pDevice, m_pContext))))
//		CRASH("Prototype Create Failed");
//
//	cout << "Logo FeMale Rover" << endl;
//}

HRESULT CLevel_Map::Ready_Static_Component()
{
    _matrix PreTransformMatrix = XMMatrixIdentity();
    _float fSize = 0.001f;
    PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize) * XMMatrixRotationY(XMConvertToRadians(180.0f));

	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;

	LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);
	LightDesc.vDiffuse = _float4(0.5f, 0.55f, 0.85f, 1.f);
	LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);

	m_pLightManager = CEdit_LightManager::Create();

    //?쇰컲 紐⑤뜽

    m_pGameInstance->Add_Work([&]() {

        m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Instance"),
            CEdit_MapObject_Instance::Create(m_pDevice, m_pContext));
        });

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));

	// DeferredShader_Map
	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_DeferredShader_Map"),
		CDeferredShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements, TEXT("Shader_Map")))))
		CRASH("DeferredShader_Map");

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh_Instance"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh_Instance.hlsl"), VTXMESHINSTANCE::Elements, VTXMESHINSTANCE::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_Brush"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxPoint.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh_Water"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh_Water.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_Component_VIBuffer_Point"),
        CVIBuffer_Point::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject"),
        CEdit_MapObject::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_LightObject"),
        CEdit_LightObject::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_Destruction_Peice"),
		CEdit_MapObject_Destruction_Piece::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Meteo"),
		CEdit_Meteo::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Water"),
		CEdit_MapObject_Water::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Collaps"),
		CEdit_MapObject_Collaps::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_SlideBox"),
		CEdit_SlideZone::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_Potal"),
		CEdit_Portal::Create(m_pDevice, m_pContext));

    Load_Objects();
    m_pBrush = CEdit_Brush::Create(m_pDevice, m_pContext);

	m_pGameInstance->LoadLastLOD();

    return S_OK;
}

void CLevel_Map::Ready_Event()
{
	m_pGameInstance->Subscribe<MAP_PICK>(ENUM_CLASS(LEVEL::STATIC), TEXT("ObjectPick"), [this](const MAP_PICK& event) {
		if (event.fDistance <= m_fNearDistance)
		{
			m_fNearDistance = event.fDistance;
			XMStoreFloat4(&m_vPickedPos, XMVectorSetW(XMLoadFloat3(&m_vWorldPos) + m_fNearDistance * XMLoadFloat3(&m_vWorldDir), 1.f));
		}


		switch (m_eMenu)
		{
		case Editor::CLevel_Map::MENU_OBJECT:
		{
			if (event.fDistance <= m_fNearDistance)
			{
				CGameObject* pObject = reinterpret_cast<CGameObject*>(event.pObject);
				if (m_pPickedObject = dynamic_cast<CEdit_MapObject*>(pObject))
				{

					if (m_pPickedDestructObject)
					{
						m_pPickedDestructObject = nullptr;
					}

					if (m_pChildObject)
					{
						m_pPickedObject->Add_Child(m_pChildObject);
						m_pChildObject = nullptr;
					}
				}
			}
		}
		break;

		case Editor::CLevel_Map::MENU_RANDSCAPE:
			if (event.fDistance <= m_fNearDistance_Instance)
			{
				m_fNearDistance_Instance = event.fDistance;
				m_pPickedInstanceObject = dynamic_cast<CEdit_MapObject_Instance*>(reinterpret_cast<CGameObject*>(event.pObject));
			}
			break;

		case Editor::CLevel_Map::MENU_LIGHT:
			int a = 0;
			break;
		}

		});
	m_pGameInstance->Subscribe<MAP_CREATE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Create_Object"), [this](const MAP_CREATE& event) {
		CGameObject* pObject = reinterpret_cast<CGameObject*>(event.pObject);
		{
			lock_guard<mutex> lock(m_Mutex);
			if (m_pPickedObject = dynamic_cast<CEdit_MapObject*>(pObject))
			{

				m_SaveObjects["Map_Object"].push_back(m_pPickedObject);
				Safe_AddRef(m_pPickedObject);
			}
		}

		});

	m_pGameInstance->Subscribe<MAP_CREATE>(ENUM_CLASS(LEVEL::STATIC), TEXT("Set_Parent"), [this](const MAP_CREATE& event) {
		if (!m_pChildObject)
			m_pChildObject = reinterpret_cast<CEdit_MapObject*>(event.pObject);
		else
			m_pChildObject = nullptr;
		});
}

void CLevel_Map::Make_MousePos()
{
    POINT ptMousePos = m_pGameInstance->Get_MousePoint();
	
    _float3 vMousePos{};
    vMousePos.x = ptMousePos.x / (g_iWinSizeX * 0.5f) - 1.f;
    vMousePos.y = -1 * ptMousePos.y / (g_iWinSizeY * 0.5f) + 1.f;
    vMousePos.z = 0.f;
    
    XMStoreFloat3(&vMousePos, XMVector3TransformCoord(XMLoadFloat3(&vMousePos), m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ)));
    
    m_vWorldPos = {};
    m_vWorldDir = vMousePos;

    XMStoreFloat3(&m_vWorldPos, XMVector3TransformCoord(XMLoadFloat3(&m_vWorldPos), m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW)));
    XMStoreFloat3(&m_vWorldDir, XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&m_vWorldDir), m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW))));
}

void CLevel_Map::Container_Info()
{
    ImGuiID ShaderId = ImGui::GetID("Container");
    ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
    ImGui::Text("Current Container");

    for (auto& pContainer : m_ContainerObjects)
        if (ImGui::Button(pContainer.second->Get_ModelName())) {
            int a = 0;
        }
    ImGui::EndChildFrame();
}

void CLevel_Map::Load_Foliage()
{
    ImGui::Text("Foliage Models");

    ImGuiID Models = ImGui::GetID("Foliage Models");
    
    ImGui::Text(WStringToString(m_szPreViewModelName).c_str());

    ImGui::BeginChildFrame(Models, ImVec2(100, 200));
    for (_uint i = 0; i < m_FoliagePaths.size(); ++i)
    {
        _char FileDrive[MAX_PATH] = {};
        _char FileDir[MAX_PATH] = {};
        _char FileName[MAX_PATH] = {};
        _char FileExt[MAX_PATH] = {};
        _splitpath_s(m_FoliagePaths[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);
		m_szPreViewModelName = StringToWString(FileName);
		if (ImGui::Selectable(FileName))
        {
            //m_szPreViewModelName = StringToWString(FileName);
            _wstring ProtoName = TEXT("Prototype_Component_Model_Instance_");
            ProtoName += StringToWString(FileName);
			m_pBrush->Set_ModelName(StringToWString(FileName));
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::Begin("PreView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
#ifdef _DEBUG
            ImGui::Image(m_pGameInstance->Get_Debug_RT_Resource(TEXT("RT_Debug")), ImVec2(128, 128));
#endif
            ImGui::End();
            m_pPreViewObject->Late_Update(0.016f, m_szPreViewModelName);
        }
    }


    ImGui::EndChildFrame();
}
CLevel_Map* CLevel_Map::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Map* pInstance = new CLevel_Map(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Map");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Map::Free()
{
    __super::Free();
    m_pPickedObject = nullptr;
    m_pPickedInstanceObject = nullptr;
    m_pPickedLightObject = nullptr;
	m_pPickedDestructObject = nullptr;
	m_pPickedTriggerBox = nullptr;
	m_pPickedWater = nullptr;

    Safe_Release(m_pPreViewObject);
	Safe_Release(m_pBrush);
	Safe_Release(m_pPickedSpawnor);
	Safe_Release(m_pLightManager);
	Safe_Release(m_pEffectCollector);
	Safe_Release(m_pFlyManager);
	
    for (auto& Pair : m_SaveObjects)
    {
        for (auto& pGameObject : Pair.second)
            Safe_Release(pGameObject);
        Pair.second.clear();
    }
    Safe_Release(pShaderInterface);
    Safe_Release(m_pAnimationTool);
    
    m_SaveObjects.clear();
}
