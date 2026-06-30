#include"EditorPch.h"
#include "Map_Interface.h"
#include"Edit_PreViewModel.h"
#include"Edit_MapObject.h"
#include"Edit_MapObject_Destruction.h"
#include"Edit_Meteo.h"
#include"Edit_MapObject_Instance.h"
#include"Edit_TriggerBox.h"
#include"Edit_MapObject_Collaps.h"
#include"Edit_LightObject.h"

CMap_Interface::CMap_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CInterface_Edit(pDevice,pContext)
{
}

HRESULT CMap_Interface::Initialize()
{
	m_FilePaths.push_back("../../Client/Bin/Resource/Map/Asphodel_Barrens/");
	m_FilePaths.push_back("../../Client/Bin/Resource/Map/Test/");
	m_FilePaths.push_back("../../Client/Bin/Resource/Map/Logo/");
	m_FilePaths.push_back("../../Client/Bin/Resource/Map/The_False_Sovereign/");
	m_FilePaths.push_back("../../Client/Bin/Resource/Map/");
	m_FilePaths.push_back("../../Client/Bin/Resource/Map/Heaven/");

    return S_OK;
}

_bool CMap_Interface::Set_ShaderPass(CShader* pShader, _uint* ShaderPassIndex)
{
    _bool Result = { false };
    ImGuiID ShaderId = ImGui::GetID("ShaderPass");
    ImGui::BeginChildFrame(ShaderId, ImVec2(100, 200));
    ImGui::Text("ShaderPass");

#ifdef _DEBUG
    for (_uint i = 0; i < pShader->Get_PassCount(); ++i)
    {
        if (!strcmp("DebugRender", pShader->Get_PassName(i)))
            continue;

        if (ImGui::Button(pShader->Get_PassName(i))) {
            *ShaderPassIndex = i;
            Result = true;
            break;
        }
    }
#endif // _DEBUG



    ImGui::EndChildFrame();


    return Result;
}

_bool CMap_Interface::Set_LOD(vector<CModel*>& pModelArray, _uint* iLODIndex)
{
    ImGuiID LODId= ImGui::GetID("LOD_Level");
    ImGui::BeginChildFrame(LODId, ImVec2(100, 200));

    _bool Result = { false };

    _char LOD[10] = {};
    sprintf_s(LOD, "LOD %d", *iLODIndex);
    ImGui::Text(LOD);
    _char LOD_Index[10] = {};

    for (_uint i = 0; i < pModelArray.size(); ++i)
    {
        if (pModelArray[i] == nullptr)
            continue;

        sprintf_s(LOD_Index, "LOD%d", i);
        if (ImGui::Button(LOD_Index))
        {
            *iLODIndex = i;
            Result = true;
            break;
        }
    }
    ImGui::EndChildFrame();

    return Result;
}

_bool CMap_Interface::Set_LOD(_uint* iLODIndex,_uint* iMaxLODIndex)
{
	ImGuiID LODId = ImGui::GetID("LOD_Level_Streaming");
	ImGui::BeginChildFrame(LODId, ImVec2(100, 200));

	_bool Result = { false };

	_char LOD_Index[10] = {};

	for (_uint i = 0; i <= *iMaxLODIndex; ++i)
	{

		sprintf_s(LOD_Index, "LOD%d", i);
		if (ImGui::Button(LOD_Index))
		{
			*iLODIndex = i;
			Result = true;
			break;
		}
	}
	ImGui::EndChildFrame();

	return Result;
}

void CMap_Interface::Set_Transform(CTransform* pTransform)
{
    m_pGameInstance->Use_Gizmo(pTransform);

    _float vScale[3] = {};
    _float vRotation[3] = {};
    _float vTransfrom[3] = {};
    _matrix matrix = pTransform->Get_WorldMatrix();
    ImGuizmo::DecomposeMatrixToComponents(reinterpret_cast<_float*>(&matrix), vTransfrom, vRotation, vScale);

    ImGui::Text("Size");
    {
        ImGui::PushItemWidth(300.0f);
        ImGui::InputFloat3("Scale", vScale);
    }

    ImGui::Text("Turn_Quaternion");
    {
        //로테?�션??계속 ?�데?�트 ?�어??값이 초기?�됨.
        ImGui::PushItemWidth(300.0f);
        //?�그�?각도�?0?�에??360?�까지.
        ImGui::InputFloat3("Rotation", vRotation);
    }

    ImGui::Text("Position");
    {
        ImGui::PushItemWidth(300.0f);
        ImGui::InputFloat3("Translation", vTransfrom);
    }

    ImGui::PopItemWidth();

    ImGuizmo::RecomposeMatrixFromComponents(vTransfrom, vRotation, vScale, reinterpret_cast<_float*>(&matrix));

    pTransform->Set_WorldMatrix(matrix);
}

_bool CMap_Interface::Load_Textures(_uint Origin, vector<_string>* VectorTextures, _string ResearchKeyWord, _string ResearchExt, _bool IsIntoChild, _string TextureFolderPath, _bool IsPng, _string SecondKeyWord)
{
    IGFD::FileDialogConfig config;

    TextureFolderPath.empty() ?
        //Soulution Parent Path
        config.path = filesystem::current_path().parent_path().parent_path().parent_path().string()
        :
        config.path = TextureFolderPath;

    _string FileExt;
    IsPng ?
        FileExt = ".png" :
        FileExt = ".dds";
    config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

    _char Text[32] = {};
    sprintf_s(Text, "%d###Texture File Load", 999);

    //if ResearchExt is empty, It Can Pick Folder
    ResearchExt.empty() ?
        ImGuiFileDialog::Instance()->OpenDialog(Text, "Texture File", nullptr, config)
        :
        ImGuiFileDialog::Instance()->OpenDialog(Text, "Texture File", ResearchExt.c_str(), config);

    if (ImGuiFileDialog::Instance()->Display(Text)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            for (const auto& entry : filesystem::recursive_directory_iterator(strFolderPath)) {
                if (entry.is_regular_file()) {
                    filesystem::path Filepath = entry.path();

                    _bool Condition;

                    SecondKeyWord.empty() ?
                        Condition = Filepath.string().find(ResearchKeyWord) != std::string::npos
                        :
                        Condition = (Filepath.string().find(ResearchKeyWord) != std::string::npos) || (Filepath.string().find(SecondKeyWord) != std::string::npos);

                    if (Condition)
                    {

                        if (Filepath.extension() == FileExt) {
                            {
                                string fileName = Filepath.filename().string();
                                VectorTextures->push_back(Filepath.string());
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

_bool CMap_Interface::Display_Textures(CTexture* pTexture, _uint iTextureNum, _float SizeX, _float SizeY)
{
    ImVec2 ImageSize;

    SizeY == 0.f ?
        ImageSize = ImVec2(SizeX, SizeX)
        :
        ImageSize = ImVec2(SizeX, SizeY);
    if (!pTexture)
        return false;

    ImGui::Image((ImTextureID)pTexture->Get_SRV(iTextureNum), ImageSize);

    return true;
}

_bool CMap_Interface::Initialize_ModelPath(_uint iLevel, _fmatrix PreTransformMatrix)
{
    if (m_IsCreateProto)
        return true;

    m_ModelPaths.clear();
    m_iLevel = iLevel;
    IGFD::FileDialogConfig config;
    config.path = "../../Client/Bin/Resource/Map/";
    config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

    _char Text[32] = {};
    sprintf_s(Text, "Object %d###Model Folder Load", 999);

    ImGuiFileDialog::Instance()->OpenDialog(Text, "Model Folder", nullptr, config);

    if (ImGuiFileDialog::Instance()->Display(Text)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            
            _int version = {};
            _int Lastversion = {};
            _wstring LastVersionName;
            _string LastVersionPath;
            vector<_wstring> m_PrototypeNames;

            m_pPreView = CEdit_PreViewModel::Create(m_pDevice, m_pContext, iLevel);

            _string strFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();

            for (const auto& entry : filesystem::recursive_directory_iterator(strFolderPath)) {
                if (entry.is_regular_file()) {
                    if (entry.path().string().find("MapData") != std::string::npos)
                        continue;

                    if (entry.path().extension() == ".dat") {

                        _char FileDrive[MAX_PATH] = {};
                        _char FileDir[MAX_PATH] = {};
                        _char FileName[MAX_PATH] = {};
                        _char FileExt[MAX_PATH] = {};
                        _splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

                        _wstring baseName = StringToWString(FileName);

                        // LOD �������� ���� ���� ����
                        size_t pos = baseName.find_last_not_of(TEXT("0123456789"));
                        _wstring namePart = baseName.substr(0, pos + 1);
                        _wstring numberPart = baseName.substr(pos + 1);
                        version = stoi(numberPart);

                        _wstring key = L"Prototype_Component_Model_" + namePart;
                        _string VersionPath = FileDir;
                        VersionPath += FileName;
                        VersionPath += ".dat";

                        _wstring PrototypeName = L"Prototype_Component_Model_";
                        PrototypeName += StringToWString(FileName);

                        m_pGameInstance->Add_Work([&, ProtoName = PrototypeName, Path = VersionPath]() {
                            if (FAILED(m_pGameInstance->Add_Prototype(iLevel, ProtoName,
                                CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, Path.c_str()))))
                                CRASH("Prototype Create Failed");
                            });
                        if (lstrcmp(LastVersionName.c_str(), key.c_str()) && !LastVersionName.empty())
                        {
                            m_PrototypeNames.push_back(LastVersionName + to_wstring(version));
                            m_ModelPaths.push_back(LastVersionPath);
                        }

                        Lastversion = version;
                        LastVersionName = key;
                        LastVersionPath = VersionPath;
                    }

                }
            }

            m_pGameInstance->Wait_Thread_End();

            for (_uint i = 0; i < m_PrototypeNames.size(); ++i)
            {
                m_pPreView->Add_Model(m_PrototypeNames[i]);
            }

            m_IsCreateProto = true;
        }
    }
    return m_IsCreateProto;
}

void CMap_Interface::Add_MapObject(_fvector vPos)
{
    if (m_ModelPaths.empty())
        return;

    ImGui::Begin("Create Model Prototype & Clone");

    ImGuiID MapId = ImGui::GetID("Map Model");
    ImGui::BeginChildFrame(MapId, ImVec2(300, 200));
    for (_uint i = 0; i < m_ModelPaths.size(); ++i)
    {
        _char FileDrive[MAX_PATH] = {};
        _char FileDir[MAX_PATH] = {};
        _char FileName[MAX_PATH] = {};
        _char FileExt[MAX_PATH] = {};
        _splitpath_s(m_ModelPaths[i].c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

        if (ImGui::Selectable(FileName))
        {


            //_wstring baseName = StringToWString(FileName);

            // LOD �������� ���� ���� ����
            //size_t pos = baseName.find_last_not_of(TEXT("0123456789"));
            //_wstring namePart = baseName.substr(0, pos + 1);
            //_wstring numberPart = baseName.substr(pos + 1);

            //_wstring PrototypeName = L"Prototype_Component_Model_" + namePart;

            _string ModelPath = FileDir;
            ModelPath += FileName;
            //���⼭ ���� ����
            ModelPath.pop_back();
            ModelPath += to_string(0);
            ModelPath += ".dat";

            //���� ���� ���ڰ� ��. 

            _wstring PrototypeName = TEXT("Prototype_Component_Model_");
            _wstring ModelName = StringToWString(FileName);
            ModelName.pop_back();
            PrototypeName += ModelName;

            _wstring ObjectName;
            CEdit_MapObject::MAP_LOAD Desc{};
            _float4x4 DefaultMatrix{};

            if (vPos.m128_f32[3] == 0.f)
                XMStoreFloat4x4(&DefaultMatrix, XMMatrixTranslationFromVector(vPos));
            else
                XMStoreFloat4x4(&DefaultMatrix, XMMatrixIdentity());

            Desc.WorldMatrix = &DefaultMatrix;
            Desc.iShaderPassIndex = 0;
            strcpy_s(Desc.ModelName, FileName);
            Desc.iLevel = m_iLevel;
            m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
                , m_iLevel, TEXT("Layer_MapObject"), &Desc);
            break;
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::Begin("PreView", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
            _wstring m_szPreViewModelName = StringToWString(FileName);
#ifdef _DEBUG
            ImGui::Image(m_pGameInstance->Get_Debug_RT_Resource(TEXT("RT_Debug")), ImVec2(128, 128));
#endif
            ImGui::End();
            m_pPreView->Late_Update(0.016f, m_szPreViewModelName);
            m_pPreView->Render();
        }
    }
    ImGui::EndChildFrame();

    ImGui::End();
}

void CMap_Interface::Load_Map_GUI()
{
	if (m_IsCreateMap)
		return;

	ImGui::Begin("Map Load");

	IGFD::FileDialogConfig config;

	config.path = "../../Client/Bin/Resource/Map/MapData/";
	config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

	ImGuiFileDialog::Instance()->OpenDialog("Map File Load", "Import File", ".dat", config);
	ImGui::Begin("Select Map");
	
	ImGui::Text(m_szFilePath.c_str());
	for (auto& pFilePath : m_FilePaths)
	{
		if (ImGui::Button(pFilePath))
			m_szFilePath = pFilePath;
	}
	ImGui::End();

	if (ImGuiFileDialog::Instance()->Display("Map File Load")) {

		if (ImGuiFileDialog::Instance()->IsOk()) {
			m_IsCreateMap = true;
			_string DatFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
			m_pGameInstance->Load_Resource(m_szFilePath.c_str());
			Ready_Map_Prototype(m_szFilePath.c_str());
			m_pGameInstance->LoadLastLOD();
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
					if (strFilePath.find("Spawn") != std::string::npos ||
						strFilePath.find("Effect") != std::string::npos || 
						strFilePath.find("SlideBox") != std::string::npos || 
						strFilePath.find("FireFly") != std::string::npos)
						continue;
					else if (strFilePath.find("Meteo") != std::string::npos)
					{
						continue;
						CEdit_Meteo::MAP_LOAD Desc{};

						while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
						{
							memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
							File.read(Desc.ModelName, NameLength);
							_string Name = Desc.ModelName;


							File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
							File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
							_float4x4 Matrix = {};
							File.read(reinterpret_cast<char*>(&Desc.WorldMatrix), sizeof(_float4x4));

							File.read(reinterpret_cast<char*>(&Desc.vSourPos), sizeof(_float4));
							File.read(reinterpret_cast<char*>(&Desc.vDestPos), sizeof(_float4));

							File.read(reinterpret_cast<char*>(&Desc.fDuration), sizeof(_float));
							File.read(reinterpret_cast<char*>(&Desc.fArchY), sizeof(_float));
							File.read(reinterpret_cast<char*>(&Desc.TriggerIndex), sizeof(_uint));
							File.read(reinterpret_cast<char*>(&Desc.TriggerActiveIndex), sizeof(_int));
							_vector Pos = XMLoadFloat4(&Desc.vSourPos);
							//_matrix Mat = XMMatrixTranslationFromVector();
							Desc.iLevel = m_iLevel;
							XMStoreFloat4x4(&Desc.WorldMatrix, XMMatrixTranslationFromVector(Pos));
							m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Meteo")
								, m_iLevel, TEXT("Layer_Meteo"), &Desc);
						}
					}
					else if (strFilePath.find("Instance") != std::string::npos)
					{
						continue;
						_matrix PreTransformMatrix = XMMatrixIdentity();
						_float fSize = 0.01f;
						PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

						CEdit_MapObject_Instance::MAP_LOAD Desc{};

						while (File.read(reinterpret_cast<char*>(&Desc.iSaveIndex), sizeof(_uint)))
						{
							File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));
							memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
							File.read(Desc.ModelName, NameLength);

							File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
							if (Desc.iShaderPassIndex == 2)
								File.read(reinterpret_cast<char*>(&Desc.vDiffuseColor), sizeof(_float4));

							File.read(reinterpret_cast<char*>(&Desc.iNumInstance), sizeof(_uint));

							_float4x4* InstanceMatrix = new _float4x4[Desc.iNumInstance];

							File.read(reinterpret_cast<char*>(InstanceMatrix), sizeof(_float4x4) * Desc.iNumInstance);
							Desc.InstanceWorldMatrix = InstanceMatrix;

							File.read(reinterpret_cast<char*>(&Desc.WorldMatrix), sizeof(_float4x4));

							_float3 vBoundingPos;
							_float3 vBoundingExtends;
							File.read(reinterpret_cast<char*>(&vBoundingPos), sizeof(_float3));
							File.read(reinterpret_cast<char*>(&vBoundingExtends), sizeof(_float3));
							Desc.IsLoaded = true;

							//이거를 프로토타입으로 만든 이후 바로 클론하기.

							CMesh_Instance::MESH_INST_DESC MeshDesc{};
							MeshDesc.iNumInstance = Desc.iNumInstance;
							MeshDesc.pTransformMatrix = InstanceMatrix;
							//파일시스템으로 해당 모델 찾기.
							_string ModelPath = Desc.ModelName;
							ModelPath.pop_back();
							_string m_FolderPath;
							for (const auto& entry : filesystem::recursive_directory_iterator(m_FolderPath)) {
								if (entry.is_regular_file()) {
									if (entry.path().string().find("Foliage") == std::string::npos)
										continue;

									if (entry.path().string().find(ModelPath) == std::string::npos)
										continue;

									if (entry.path().extension() != ".dat")
										continue;

									_char FileDrive[MAX_PATH] = {};
									_char FileDir[MAX_PATH] = {};
									_char FileName[MAX_PATH] = {};
									_char FileExt[MAX_PATH] = {};
									_splitpath_s(entry.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);


									_wstring PrototypeName = L"Prototype_Component_Model_Instance_";
									_wstring ModelName = StringToWString(FileName) + to_wstring(Desc.iSaveIndex);
									PrototypeName += ModelName;

									_string VersionPath = FileDir;
									VersionPath += FileName;
									VersionPath += ".dat";
									if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, PrototypeName,
										CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, VersionPath.c_str(), false, &MeshDesc))))
										CRASH("Prototype Create Failed");

									memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
									strcpy_s(Desc.ModelName, WStringToString(ModelName).c_str());
								}
							}
							m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Instance")
								, m_iLevel, TEXT("Layer_Instance"), &Desc);
							Safe_Delete_Array(InstanceMatrix);
						}
					}
					else if (strFilePath.find("Destruction") != std::string::npos)
					{
						_matrix PreTransformMatrix = XMMatrixIdentity();
						_float fSize = 0.01f;
						PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

						CEdit_MapObject_Destruction::MAP_LOAD Desc{};

						while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
						{
							memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
							File.read(Desc.ModelName, NameLength);
							_string Name = Desc.ModelName;

							File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
							File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
							_float4x4 Matrix = {};
							File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
							Desc.WorldMatrix = &Matrix;
							Desc.iLevel = m_iLevel;

							File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
							File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));

							File.read(reinterpret_cast<char*>(&Desc.m_vImpulsePos), sizeof(_float3));
							File.read(reinterpret_cast<char*>(&Desc.m_vImpulsePower), sizeof(_float3));

							File.read(reinterpret_cast<char*>(&Desc.iTriggerIndex), sizeof(_uint));
							m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Destruction")
								, m_iLevel, TEXT("Layer_Test"), &Desc);
						}
					}
					else if (strFilePath.find("TriggerBox") != std::string::npos)
					{
						_uint iTriggerIndex;
						CEdit_TriggerBox::TRIGGER Desc{};
						while (File.read(reinterpret_cast<char*>(&Desc.iTriggerIndex), sizeof(_uint)))
						{
							File.read(reinterpret_cast<char*>(&Desc.vExtends), sizeof(_float3));
							_float4x4 Matrix = {};
							File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
							Desc.WorldMatrix = &Matrix;
							m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_TriggerBox")
								, m_iLevel, TEXT("Layer_Test"), &Desc);
						}
					}
					else if (strFilePath.find("Collaps") != std::string::npos)
					{
						CEdit_MapObject_Collaps::MAP_LOAD Desc{};
						while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
						{
							memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
							File.read(Desc.ModelName, NameLength);

							File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
							File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
							File.read(reinterpret_cast<char*>(&Desc.vSourWorldMatrix), sizeof(_float4x4));
							File.read(reinterpret_cast<char*>(&Desc.vDestWorldMatrix), sizeof(_float4x4));

							File.read(reinterpret_cast<char*>(&Desc.fDuration), sizeof(_float));
							File.read(reinterpret_cast<char*>(&Desc.TriggerIndex), sizeof(_uint));
							File.read(reinterpret_cast<char*>(&Desc.TriggerActiveIndex), sizeof(_int));

							m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject_Collaps")
								, m_iLevel, TEXT("Layer_MapObject_Collaps"), &Desc);

						}
					}
					else if (strFilePath.find("Object_Light") != std::string::npos)
					{
						LIGHT_DESC ReadDesc{};

						_uint iSize = {};
						File.read(reinterpret_cast<char*>(&iSize), sizeof(_uint));
						for (_uint i = 0; i < iSize; ++i)
						{
							File.read(reinterpret_cast<char*>(&ReadDesc.eType), sizeof(_uint));
							File.read(reinterpret_cast<char*>(&ReadDesc.fRange), sizeof(_float));
							File.read(reinterpret_cast<char*>(&ReadDesc.vAmbient), sizeof(_float4));
							File.read(reinterpret_cast<char*>(&ReadDesc.vDiffuse), sizeof(_float4));
							File.read(reinterpret_cast<char*>(&ReadDesc.vDirection), sizeof(_float4));
							File.read(reinterpret_cast<char*>(&ReadDesc.vPosition), sizeof(_float4));
							File.read(reinterpret_cast<char*>(&ReadDesc.vSpecular), sizeof(_float4));

							CEdit_LightObject::MAP_LOAD Desc{};
							Desc.vWorldPos = ReadDesc.vPosition;
							Desc.CopyDesc = &ReadDesc;
							m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_LightObject")
								, m_iLevel, TEXT("Layer_Light"), &Desc);
						}
					}
					else
					{

						CEdit_MapObject::MAP_LOAD Desc{};

						while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
						{
							memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
							File.read(Desc.ModelName, NameLength);
							_string Name = Desc.ModelName;

							File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
							File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
							_float4x4 Matrix = {};
							File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
							Desc.WorldMatrix = &Matrix;
							File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
							File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));


							m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex, eObjectType = Desc.eObjectType, Matrix = *Desc.WorldMatrix]() mutable {
								CEdit_MapObject::MAP_LOAD pDesc{};
								strcpy_s(pDesc.ModelName, ModelName.c_str());
								pDesc.iShaderPassIndex = ShaderPass;
								pDesc.eObjectType = eObjectType;
								pDesc.WorldMatrix = &Matrix;
								pDesc.iLevel = m_iLevel;

								m_pGameInstance->Add_GameObject_ToLayer(m_iLevel, TEXT("Prototype_GameObject_MapObject")
									, m_iLevel, TEXT("Layer_Test"), &pDesc);
								});

						}
					}
					m_pGameInstance->Wait_Thread_End();
					File.close();

				}
			}
			ImGuiFileDialog::Instance()->Close();
		}
		else
		{
			ImGuiFileDialog::Instance()->Close();
		}
	}
	ImGui::End();
}

void CMap_Interface::Load_Map(const _char* pFilePath)
{
}

void CMap_Interface::Ready_Map_Prototype(const _char* pFilePath)
{
	_matrix PreTransformMatrix = XMMatrixIdentity();
	_float fSize = 0.01f;
	//_float fSize = 0.02f;
	unordered_set<_wstring> m_ProtoNames;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);
	for (const auto& entry : filesystem::recursive_directory_iterator(pFilePath)) {
		if (entry.is_regular_file()) {
			if (entry.path().string().find("MapData") != std::string::npos)
				continue;

			if (entry.path().string().find("Anim") != std::string::npos)
				continue;
			
			if (entry.path().string().find("FireFly") != std::string::npos)
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
					m_pGameInstance->Add_Work([&, ProtoName = PrototypeName, Path = VersionPath]() {
						if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
							CModel::Create(m_pDevice, m_pContext, MODELTYPE::ECO, PreTransformMatrix, Path.c_str()))))
							CRASH("Prototype Create Failed");
						});
					continue;
				}

				_wstring baseName = StringToWString(FileName);

				// LOD 마지막에 붙은 숫자 추출
				size_t pos = baseName.find_last_not_of(TEXT("0123456789"));
				_wstring namePart = baseName.substr(0, pos + 1);

				_wstring numberPart = baseName.substr(pos + 1);

				_wstring key = L"Prototype_Component_Model_" + namePart;


				_wstring StreamName = PrototypeName;

				StreamName.pop_back();
				StreamName.pop_back();
				StreamName.pop_back();
				StreamName.pop_back();
				StreamName.pop_back();

				auto iter = m_ProtoNames.find(StreamName);
				if(iter == m_ProtoNames.end())
				{
					m_ProtoNames.insert(StreamName);
					if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, StreamName,
						CModel_Streaming::Create(m_pDevice, m_pContext, FileDir))))
						CRASH("Prototype Create Failed");
				}

				//m_pGameInstance->Add_Work([&, ProtoName = StreamName, Path = VersionPath]() {
				//	if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
				//		CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, Path.c_str()))))
				//		CRASH("Prototype Create Failed");
				//	});

				if (entry.path().string().find("Foliage") != std::string::npos)
				{
					_wstring ProtoName = TEXT("Prototype_Component_Model_Instance_");
					ProtoName += StringToWString(FileName);

					m_pGameInstance->Add_Work([&, ProtoName = ProtoName, Path = VersionPath]() {
						if (FAILED(m_pGameInstance->Add_Prototype(m_iLevel, ProtoName,
							CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, Path.c_str(), true))))
							CRASH("Prototype Create Failed");
						});
				}
			}
		}
	}
	m_pGameInstance->Wait_Thread_End();
}

void CMap_Interface::SetPrototypes(_uint iLevel)
{
	m_iLevel = iLevel;
	m_pGameInstance->Add_Prototype(iLevel, TEXT("Prototype_Component_Shader_NonAnimMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));

	if (FAILED(m_pGameInstance->Add_Prototype(iLevel, TEXT("Prototype_GameObject_MapObject"),
		CEdit_MapObject::Create(m_pDevice, m_pContext))))
		CRASH("Prototype Create Failed");

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Instance"),
		CEdit_MapObject_Instance::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Meteo"),
		CEdit_Meteo::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Destruction"),
		CEdit_MapObject_Destruction::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_MapObject_Destruction"),
		CEdit_MapObject_Destruction::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(m_iLevel, TEXT("Prototype_GameObject_TriggerBox"),
		CEdit_TriggerBox::Create(m_pDevice, m_pContext));
}

CMap_Interface* CMap_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMap_Interface* pInstance = new CMap_Interface(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Map_Insterface");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CMap_Interface::Free()
{
    __super::Free();
    Safe_Release(m_pPreView);
}
