#include "ClientPch.h"
#include "Parser.h"

#pragma region MAP
#include "MapObject.h"

#include "Trigger_Box.h"
#include "MapObject_Destruction.h"
#include "MapObject_Instance.h"
#include "MapObject_Meteo.h"
#include "MapObject_Collaps.h"
#include "MapObject_FireFly.h"
#include "Slide_Navigation.h"

#include "Spawner.h"
#pragma endregion

CParser::CParser(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pGameInstance{ CGameInstance::GetInstance() },
	m_pDevice{ pDevice }, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

void CParser::Ready_Prototype_Map(const _char* pFilePath, LEVEL eLevel, const _char* pModelFilePath)
{
	_char FileDrive[MAX_PATH] = {};
	_char FileDir[MAX_PATH] = {};
	_char FileName[MAX_PATH] = {};
	_char FileExt[MAX_PATH] = {};

	_splitpath_s(pFilePath, FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

	_string PasingDir = FileDir;
	m_pGameInstance->Model_Manager_Change_Level(ENUM_CLASS(eLevel));
	Read_Map_Prototype(PasingDir, eLevel, pModelFilePath);
	m_LoadingMap[eLevel].push_back(pFilePath);
}

void CParser::Read_Map_Prototype(const _string pDataFilePath, LEVEL eLevel, const _char* pModelFilePath)
{
	//넘어오는 건 폴더 경로.
	_string ProjectPath = filesystem::current_path().parent_path().parent_path().string();
	ProjectPath += "/Client/Bin/Resource/Map/";
	ProjectPath += pModelFilePath;
	_float fSize = 0.01f;
	_matrix PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

	_wstring PrototypeName = L"Prototype_Component_Model_";
	_wstring InstancePrototypeName = L"Prototype_Component_Model_Instance_";

	_string FilePath;
	for (const auto& entry : filesystem::directory_iterator(pDataFilePath)) {
		if (!entry.is_regular_file())
			continue;
		FilePath = entry.path().string();
		if (FilePath.find("Prototype") == std::string::npos && FilePath.find("Instance") == std::string::npos
			&& FilePath.find("MonsterSpawnor") == std::string::npos && FilePath.find("FireFly") == std::string::npos)
			continue;

		_string strFilePath = FilePath;
		ifstream File(strFilePath, ios::binary);

		_uint NameLength = {};

		_char Name[MAX_PATH] = {};

		if (FilePath.find("Instance") != std::string::npos)
		{
			CMapObject_Instance::MAP_LOAD Desc{};
			unordered_set<_wstring> m_Names;
			while (File.read(reinterpret_cast<char*>(&Desc.iSaveIndex), sizeof(_uint)))
			{
				CMesh_Instance::MESH_INST_DESC MeshDesc{};

				File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));
				memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
				File.read(Desc.ModelName, NameLength);

				File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
				File.read(reinterpret_cast<char*>(&Desc.vDiffuseColor), sizeof(_float4));

				File.read(reinterpret_cast<char*>(&MeshDesc.iNumInstance), sizeof(_uint));
				MeshDesc.pTransformMatrix = new _float4x4[MeshDesc.iNumInstance];

				File.read(reinterpret_cast<char*>(MeshDesc.pTransformMatrix), sizeof(_float4x4) * MeshDesc.iNumInstance);

				File.read(reinterpret_cast<char*>(&Desc.WorldMatrix), sizeof(_float4x4));

				File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
				File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));
				File.read(reinterpret_cast<char*>(&Desc.eInstanceType), sizeof(INSTANCETYPE));

				Desc.iLevel = ENUM_CLASS(eLevel);

				_string ModelOrigin = Desc.ModelName;
				ModelOrigin.pop_back();
				for (const auto& entry2 : filesystem::recursive_directory_iterator(ProjectPath)) {
					if (entry2.path().string().find("Foliage") == std::string::npos)
						continue;

					if (entry2.path().string().find("Test") != std::string::npos)
						continue;

					//지금 LOD단계 다 만드는 게 아니라 하나만 만드는 거 같음.
					if (entry2.path().string().find(ModelOrigin) == std::string::npos)
						continue;
					if (entry2.path().extension() != ".dat")
						continue;

					_char FileDrive[MAX_PATH] = {};
					_char FileDir[MAX_PATH] = {};
					_char FileName[MAX_PATH] = {};
					_char FileExt[MAX_PATH] = {};
					_splitpath_s(entry2.path().string().c_str(), FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);
					//LOD단계별로 하고있어서 0, 1, 2  해야하는데 20,21,22, 이런 식으로 됨.
					_wstring PrototypeName = L"Prototype_Component_Model_Instance_";
					_wstring ModelName = StringToWString(FileName) + to_wstring(Desc.iSaveIndex);

					PrototypeName += ModelName;
					strcpy_s(Desc.ModelName, WStringToString(ModelName).c_str());
					_string VersionPath = FileDir;
					VersionPath += FileName;
					VersionPath += ".dat";

					if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), PrototypeName,
						CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, VersionPath.c_str(), false, &MeshDesc))))
						CRASH("Prototype Create Failed");

				}
				m_MapInstanceData.push_back(Desc);
				Safe_Delete_Array(MeshDesc.pTransformMatrix);
			}
		}
		else if (FilePath.find("MonsterSpawnor") != std::string::npos)
		{
			SPAWN_DESC Desc;
			while (File.read(reinterpret_cast<char*>(&Desc.vMonsterSpawnorPos), sizeof(_float4)))
			{
				memset(Desc.szMonsterName1, 0, sizeof(Desc.szMonsterName1));
				memset(Desc.szMonsterName2, 0, sizeof(Desc.szMonsterName2));
				memset(Desc.szMonsterName3, 0, sizeof(Desc.szMonsterName3));

				File.read(reinterpret_cast<char*>(&Desc.vMonsterPos1), sizeof(_float4));

				File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));
				File.read(Desc.szMonsterName1, NameLength);

				File.read(reinterpret_cast<char*>(&Desc.vMonsterPos2), sizeof(_float4));
				File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));
				File.read(Desc.szMonsterName2, NameLength);

				File.read(reinterpret_cast<char*>(&Desc.vMonsterPos3), sizeof(_float4));
				File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));
				File.read(Desc.szMonsterName3, NameLength);
				m_MonsterDesc[eLevel].push_back(Desc);
			}
		}
		else if (FilePath.find("FireFly") != std::string::npos)
		{
			CMesh_Instance_FireFly::MESH_INST_DESC MeshDesc{};
			CMapObject_FireFly::MAP_LOAD MapDesc{};
			_float FlySize = 0.00005f;
			_string FireFlyFilePath = "../Bin/Resource/Map/Asphodel_Barrens/FireFly/FireFly.dat";
			_matrix FireFlyPreTransformMatrix = XMMatrixScaling(FlySize, FlySize, FlySize);
			_uint i = 0;

			while (File.read(reinterpret_cast<char*>(&MeshDesc.iNumInstance), sizeof(_uint)))
			{
				File.read(reinterpret_cast<char*>(&MapDesc.iShaderPassIndex), sizeof(_uint));
				File.read(reinterpret_cast<char*>(&MapDesc.WorldMatrix), sizeof(_float4x4));

				_float4x4* pInstanceTransform = new _float4x4[MeshDesc.iNumInstance];
				for (_uint i = 0; i < MeshDesc.iNumInstance; ++i)
					pInstanceTransform[i] = MapDesc.WorldMatrix;
				MeshDesc.pTransformMatrix = pInstanceTransform;
				File.read(reinterpret_cast<char*>(&MeshDesc.vRange), sizeof(_float2));
				File.read(reinterpret_cast<char*>(&MeshDesc.vPerMoveSin), sizeof(_float2));
				File.read(reinterpret_cast<char*>(&MeshDesc.vPerMoveCos), sizeof(_float2));
				File.read(reinterpret_cast<char*>(&MeshDesc.vPerMoveSin2), sizeof(_float2));
				strcpy_s(MapDesc.ModelName, "Fly");
				strcat_s(MapDesc.ModelName, to_string(i++).c_str());
				MapDesc.iLevel = ENUM_CLASS(eLevel);
				m_FireFlyData.push_back(MapDesc);
				if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), StringToWString(MapDesc.ModelName),
					CModel_Instance_FireFly::Create(m_pDevice, m_pContext, FireFlyPreTransformMatrix, FireFlyFilePath.c_str(), false, &MeshDesc))))
					CRASH("Prototype Create Failed");

				Safe_Delete_Array(pInstanceTransform);
			}
		}
		else if (FilePath.find("Prototype") != std::string::npos)
		{
			while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
			{
				memset(Name, 0, sizeof(Name));
				File.read(reinterpret_cast<_char*>(&Name), NameLength);
				//여기서 프로토타입 생성.
				_uint ProtoMax = Name[strlen(Name) - 1] - '0' + 1;
				_string ModelName = Name;
				ModelName.pop_back();


				for (const auto& entry2 : filesystem::recursive_directory_iterator(ProjectPath)) {
					if (entry2.path().string().find("MapData") != std::string::npos)
						continue;

					if (entry2.path().string().find("Test") != std::string::npos)
						continue;

					if (entry2.path().string().find(ModelName) == std::string::npos)
						continue;

					if (entry2.path().extension() != ".dat")
						continue;

					if (entry2.path().string().find("Anim") != std::string::npos)
						continue;

					_string Path = entry2.path().string();
					_string Prototype = entry2.path().stem().string();
					//파서 수정중
					if (entry2.path().string().find("_Bone") != std::string::npos)
					{
						m_pGameInstance->Add_Work([=, Model = PrototypeName + StringToWString(Prototype), ModelPath = Path]() {
							if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), PrototypeName + StringToWString(Prototype),
								CModel::Create(m_pDevice, m_pContext, MODELTYPE::ECO, PreTransformMatrix, ModelPath.c_str()))))
								CRASH("Prototype Create Failed");
						});
					}
					else if (entry2.path().string().find("Instance") != std::string::npos)
					{
						m_pGameInstance->Add_Work([=, Model = InstancePrototypeName + StringToWString(Prototype), ModelPath = Path]() {
							if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), Model,
								CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, ModelPath.c_str()))))
								CRASH("Prototype Create Failed");
						});
					}
					else
					{
						if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), PrototypeName + StringToWString(entry2.path().parent_path().stem().string()),
							CModel_Streaming::Create(m_pDevice, m_pContext, entry2.path().parent_path().string().c_str()))))
							CRASH("Prototype Create Failed");
						break;
					}
				}
			}
		}
		File.close();
	}
}

void CParser::Clone_MapObjects(LEVEL eLevel)
{
	m_pGameInstance->LoadLastLOD();

	if (m_LoadingMap[eLevel].empty())
		MSG_BOX("Map Clone Failed");

	_char FileDrive[MAX_PATH] = {};
	_char FileDir[MAX_PATH] = {};
	_char FileName[MAX_PATH] = {};
	_char FileExt[MAX_PATH] = {};

	for (auto& FilePath : m_LoadingMap[eLevel])
	{
		_splitpath_s(FilePath, FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

		for (const auto& entry : filesystem::recursive_directory_iterator(FileDir)) {
			if (!entry.is_regular_file())
				continue;

			if (entry.path().extension() != ".dat")
				continue;

			if (entry.path().string().find("Prototype") != std::string::npos)
				continue;

			// 카테고리별로 바이너리 포맷이 달라서, Read_Map_Dat이 파싱 가능한
			// "_Map_Object" 접미사 파일만 통과시킨다. (Instance/TriggerBox/... 제외)
			_string Stem = entry.path().stem().string();
			const _string SuffixMapObject = "_Map_Object";
			if (Stem.size() < SuffixMapObject.size() ||
				Stem.compare(Stem.size() - SuffixMapObject.size(), SuffixMapObject.size(), SuffixMapObject) != 0)
				continue;

			_string strFilePath = entry.path().string();

			Read_Map_Dat(eLevel, strFilePath);
		}
	}
	m_pGameInstance->Destroy_RigidData();
}


void CParser::Read_Map_Dat(LEVEL eLevel, const _string pFilePath)
{
	if (pFilePath.find("Spawn") != _string::npos)
		return;

	ifstream File(pFilePath, ios::binary);

	if (!File.is_open())
	{
		MSG_BOX("Load Failed");
	}

	_uint NameLength;

	_matrix PreTransformMatrix = XMMatrixIdentity();
	_float fSize = 0.01f;
	PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

	if (pFilePath.find("Instance") != std::string::npos)
	{
		for (_uint i = 0; i < m_MapInstanceData.size(); ++i)
		{
			m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Instance")
				, ENUM_CLASS(eLevel), TEXT("Layer_Instance"), &m_MapInstanceData[i]);
		}
		m_MapInstanceData.clear();
	}
	else if (pFilePath.find("Destruction") != std::string::npos)
	{
		_uint NameLength;

		_matrix PreTransformMatrix = XMMatrixIdentity();
		_float fSize = 0.01f;
		PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

		CMapObject_Destruction::MAP_LOAD Desc{};

		while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
		{
			memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
			File.read(Desc.ModelName, NameLength);
			_string Name = Desc.ModelName;
			Name.pop_back();
			Name.pop_back();
			Name.pop_back();
			Name.pop_back();
			Name.pop_back();
			strcpy_s(Desc.ModelName, Name.c_str());
			OBJECTTYPE Type;
			File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
			File.read(reinterpret_cast<char*>(&Type), sizeof(OBJECTTYPE));
			_float4x4 Matrix = {};
			File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
			Desc.WorldMatrix = &Matrix;
			Desc.iLevel = ENUM_CLASS(eLevel);
			File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
			File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));

			File.read(reinterpret_cast<char*>(&Desc.m_vImpulsePos), sizeof(_float3));
			File.read(reinterpret_cast<char*>(&Desc.m_vImpulsePower), sizeof(_float3));
			File.read(reinterpret_cast<char*>(&Desc.iTriggerIndex), sizeof(_uint));

			m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Destruction")
				, ENUM_CLASS(eLevel), TEXT("Layer_Meteo"), &Desc);
		}
	}
	else if (pFilePath.find("Meteo") != std::string::npos)
	{
		CMapObject_Meteo::MAP_LOAD Desc{};

		while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
		{
			memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
			File.read(Desc.ModelName, NameLength);
			_string Name = Desc.ModelName;

			OBJECTTYPE Type;
			File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
			File.read(reinterpret_cast<char*>(&Type), sizeof(OBJECTTYPE));
			File.read(reinterpret_cast<char*>(&Desc.WorldMatrix), sizeof(_float4x4));

			File.read(reinterpret_cast<char*>(&Desc.vSourPos), sizeof(_float4));
			File.read(reinterpret_cast<char*>(&Desc.vDestPos), sizeof(_float4));

			File.read(reinterpret_cast<char*>(&Desc.fDuration), sizeof(_float));
			File.read(reinterpret_cast<char*>(&Desc.fArchY), sizeof(_float));


			File.read(reinterpret_cast<char*>(&Desc.TriggerIndex), sizeof(_uint));

			File.read(reinterpret_cast<char*>(&Desc.TriggerActiveIndex), sizeof(_int));
			//XMStoreFloat4x4(&Desc.WorldMatrix, XMMatrixTranslationFromVector(XMLoadFloat4(&Desc.vSourPos)));
			m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Meteo")
				, ENUM_CLASS(eLevel), TEXT("Layer_Meteo"), &Desc);
		}
	}
	else if (pFilePath.find("TriggerBox") != std::string::npos)
	{
		//_uint iTriggerIndex;
		CTrigger_Box::TRIGGER Desc{};
		while (File.read(reinterpret_cast<char*>(&Desc.iTriggerIndex), sizeof(_uint)))
		{
			File.read(reinterpret_cast<char*>(&Desc.vExtends), sizeof(_float3));
			_float4x4 Matrix = {};
			File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
			Desc.WorldMatrix = &Matrix;
			m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_TriggerBox")
				, ENUM_CLASS(eLevel), TEXT("Layer_Trigger"), &Desc);
		}
	}
	else if (pFilePath.find("Collaps") != std::string::npos)
	{
		CMapObject_Collaps::MAP_LOAD Desc{};
		OBJECTTYPE Type;
		while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
		{
			memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
			File.read(Desc.ModelName, NameLength);

			File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
			File.read(reinterpret_cast<char*>(&Type), sizeof(OBJECTTYPE));
			File.read(reinterpret_cast<char*>(&Desc.vSourWorldMatrix), sizeof(_float4x4));
			File.read(reinterpret_cast<char*>(&Desc.vDestWorldMatrix), sizeof(_float4x4));

			File.read(reinterpret_cast<char*>(&Desc.fDuration), sizeof(_float));
			File.read(reinterpret_cast<char*>(&Desc.TriggerIndex), sizeof(_uint));
			File.read(reinterpret_cast<char*>(&Desc.TriggerActiveIndex), sizeof(_int));
			Desc.iLevel = ENUM_CLASS(eLevel);
			m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Collaps")
				, ENUM_CLASS(eLevel), TEXT("Layer_Collaps"), &Desc);
		}
	}
	else if (pFilePath.find("Light") != std::string::npos)
	{
		LIGHT_DESC ReadDesc{};
		_uint iLightIndex = { 0 };
		File.read(reinterpret_cast<char*>(&iLightIndex), sizeof(_uint));
		for (_uint i = 0; i < iLightIndex; ++i)
		{
			File.read(reinterpret_cast<char*>(&ReadDesc.eType), sizeof(_uint));
			File.read(reinterpret_cast<char*>(&ReadDesc.fRange), sizeof(_float));
			File.read(reinterpret_cast<char*>(&ReadDesc.vAmbient), sizeof(_float4));
			File.read(reinterpret_cast<char*>(&ReadDesc.vDiffuse), sizeof(_float4));
			File.read(reinterpret_cast<char*>(&ReadDesc.vDirection), sizeof(_float4));
			File.read(reinterpret_cast<char*>(&ReadDesc.vPosition), sizeof(_float4));
			File.read(reinterpret_cast<char*>(&ReadDesc.vSpecular), sizeof(_float4));
			m_pGameInstance->Add_Light(to_wstring(i), ReadDesc);
		}
	}
	else if (pFilePath.find("Effect") != std::string::npos)
	{
		MAPEFFECT Effect{};

		while (File.read(reinterpret_cast<char*>(&Effect.iEffectTag), sizeof(_uint)))
		{
			File.read(reinterpret_cast<char*>(&Effect.vEffectPos), sizeof(_float4));
			m_MapEffects.push_back(Effect);
		}
	}
	else if (pFilePath.find("FireFly") != std::string::npos)
	{
		for (auto& pData : m_FireFlyData)
		{
			m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_MapObejct_FireFly"),
				ENUM_CLASS(eLevel), TEXT("Layer_Fly"), &pData);
		}
		m_FireFlyData.clear();
	}
	else if (pFilePath.find("Slide") != std::string::npos)
	{
		CSlide_Navigation::SLIDE_NAVIGATION_DESC Desc{};

		while (File.read(reinterpret_cast<char*>(&Desc.IsStart), sizeof(_bool)))
		{
			File.read(reinterpret_cast<char*>(&Desc.vExtends), sizeof(_float3));
			File.read(reinterpret_cast<char*>(&Desc.WorldMat), sizeof(_float4x4));
			File.read(reinterpret_cast<char*>(&Desc.iPathSize), sizeof(_uint));
			_float4* pPath = new _float4[Desc.iPathSize];
			for (_uint i = 0; i < Desc.iPathSize; ++i)
				File.read(reinterpret_cast<char*>(&pPath[i]), sizeof(_float4));
			Desc.pPath = pPath;
			m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_Slide_Navigation")
				, ENUM_CLASS(eLevel), TEXT("Layer_Collaps"), &Desc);

			Safe_Delete_Array(pPath);
		}
	}
	else
	{
		CMapObject::MAP_LOAD Desc{};

		_wstring PrototypeName = TEXT("Prototype_Component_Model_");

		while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
		{
			memset(Desc.ModelName, 0, sizeof(Desc.ModelName));
			File.read(Desc.ModelName, NameLength);

			File.read(reinterpret_cast<char*>(&Desc.iShaderPassIndex), sizeof(_uint));
			File.read(reinterpret_cast<char*>(&Desc.eObjectType), sizeof(OBJECTTYPE));
			_float4x4 Matrix = {};
			File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
			Desc.WorldMatrix = &Matrix;
			File.read(reinterpret_cast<char*>(&Desc.vBoundingPos), sizeof(_float3));
			File.read(reinterpret_cast<char*>(&Desc.vBoundingExtends), sizeof(_float3));

			//프로토타입은 제일 큰 놈으로 들어옴. => 0번까지 계속 생성.
			_wstring ModelName = StringToWString(Desc.ModelName);

			m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex, eObjectType = Desc.eObjectType,
				Matrix = *Desc.WorldMatrix, BoundingPos = Desc.vBoundingPos, BoundingExtends = Desc.vBoundingExtends]() mutable {
				CMapObject::MAP_LOAD pDesc{};
				strcpy_s(pDesc.ModelName, ModelName.c_str());
				pDesc.iShaderPassIndex = ShaderPass;
				pDesc.eObjectType = eObjectType;
				pDesc.WorldMatrix = &Matrix;
				pDesc.iLevel = ENUM_CLASS(eLevel);
				pDesc.vBoundingPos = BoundingPos;
				pDesc.vBoundingExtends = BoundingExtends;

				switch (pDesc.eObjectType)
				{
				case OBJECTTYPE::SONORA:
					m_pGameInstance->Clone_Prototype(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject_Sonoro")
						, PROTOTYPE::GAMEOBJECT, &pDesc);
					break;

				case OBJECTTYPE::NONSONORA:
					m_pGameInstance->Clone_Prototype(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject_NonSonoro")
						, PROTOTYPE::GAMEOBJECT, &pDesc);
					break;

				case OBJECTTYPE::NONSONORA_FLOOR:
					m_pGameInstance->Clone_Prototype(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject_NonSonoro")
						, PROTOTYPE::GAMEOBJECT, &pDesc);
					break;
				case OBJECTTYPE::WATER:
					//return;
					m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Water")
						, ENUM_CLASS(eLevel), TEXT("Layer_Water"), &pDesc);
					break;
				case OBJECTTYPE::THROW:
					m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Throw")
						, ENUM_CLASS(eLevel), TEXT("Layer_Throw"), &pDesc);
					break;
				case OBJECTTYPE::BURN:
					m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Burn")
						, ENUM_CLASS(eLevel), TEXT("Layer_Burn"), &pDesc);
					break;
				case OBJECTTYPE::DOME:
					m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Dome")
						, ENUM_CLASS(eLevel), TEXT("Layer_Dome"), &pDesc);
					break;
				case OBJECTTYPE::TURN:
					m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Turn")
						, ENUM_CLASS(eLevel), TEXT("Layer_Turn"), &pDesc);
					break;
				default:
					if (ModelName.find("_Wat_") != string::npos)
						m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject_Water")
							, ENUM_CLASS(eLevel), TEXT("Layer_Water"), &pDesc);
					else
						m_pGameInstance->Clone_Prototype(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject")
							, PROTOTYPE::GAMEOBJECT, &pDesc);
					break;
				}
			});
		}
		m_pGameInstance->Wait_Thread_End();
	}
	File.close();
}

HRESULT CParser::Initialize()
{
	return S_OK;
}

CParser* CParser::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CParser* pInstance = new CParser(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Parser");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CParser::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}