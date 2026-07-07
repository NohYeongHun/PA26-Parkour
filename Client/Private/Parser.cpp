#include "ClientPch.h"
#include "Parser.h"

#pragma region MAP
#include "MapObject.h"

#include "Trigger_Box.h"
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
	_string ProjectPath = filesystem::current_path().parent_path().parent_path().string();
	ProjectPath += "/Client/Bin/Resource/Map/";
	ProjectPath += pModelFilePath;
	_float fSize = 0.01f;
	_matrix PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

	_wstring PrototypeName = L"Prototype_Component_Model_";
	_wstring InstancePrototypeName = L"Prototype_Component_Model_Instance_";

	struct FileCandidate {
		_string FullPath;
		_string ParentStem;
	};
	vector<FileCandidate> Candidates;
	for (const auto& entry2 : filesystem::recursive_directory_iterator(ProjectPath)) {
		const auto& p = entry2.path();
		if (p.extension() != ".dat")                                   continue;
		if (p.string().find("MapData") != _string::npos)              continue;
		if (p.string().find("Test") != _string::npos)              continue;
		if (p.string().find("Anim") != _string::npos)              continue;
		Candidates.push_back({ p.string(), p.parent_path().stem().string() });
	}

	_string FilePath;
	for (const auto& entry : filesystem::directory_iterator(pDataFilePath)) {
		if (!entry.is_regular_file())
			continue;
		FilePath = entry.path().string();
		if (FilePath.find("Prototype") == _string::npos && FilePath.find("Instance") == _string::npos
			&& FilePath.find("MonsterSpawnor") == _string::npos && FilePath.find("FireFly") == _string::npos)
			continue;

		_string strFilePath = FilePath;
		ifstream File(strFilePath, ios::binary);

		_uint NameLength = {};
		_char Name[MAX_PATH] = {};
		if (FilePath.find("Prototype") != _string::npos)
		{
			while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
			{
				memset(Name, 0, sizeof(Name));
				File.read(reinterpret_cast<_char*>(&Name), NameLength);

				_uint ProtoMax = Name[strlen(Name) - 1] - '0' + 1;
				_string ModelName = Name;
				ModelName.pop_back();

				// ★ 디스크 I/O 없이 메모리 목록에서만 검색
				for (const auto& Candidate : Candidates) {
					if (Candidate.FullPath.find(ModelName) == _string::npos)
						continue;

					if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel),
						PrototypeName + StringToWString(Candidate.ParentStem),
						CModel_Streaming::Create(m_pDevice, m_pContext,
							filesystem::path(Candidate.FullPath).parent_path().string().c_str()))))
						CRASH("Prototype Create Failed");
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

	CMapObject::MAP_LOAD Desc{};

	_wstring PrototypeName = TEXT("Prototype_Component_Model_");

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

		//프로토타입은 제일 큰 놈으로 들어옴. => 0번까지 계속 생성.
		_wstring ModelName = StringToWString(Desc.ModelName);

		m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex, eObjectType = Desc.eObjectType,
			eParkourFlag = Desc.eParkourFlag, Matrix = *Desc.WorldMatrix, BoundingPos = Desc.vBoundingPos, BoundingExtends = Desc.vBoundingExtends]() mutable {
			CMapObject::MAP_LOAD pDesc{};
			strcpy_s(pDesc.ModelName, ModelName.c_str());
			pDesc.iShaderPassIndex = ShaderPass;
			pDesc.eObjectType = eObjectType;
			pDesc.eParkourFlag = eParkourFlag;
			pDesc.WorldMatrix = &Matrix;
			pDesc.iLevel = ENUM_CLASS(eLevel);
			pDesc.vBoundingPos = BoundingPos;
			pDesc.vBoundingExtends = BoundingExtends;

			switch (pDesc.eObjectType)
			{
			case OBJECTTYPE::PARKOUR:
				if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(eLevel), TEXT("Prototype_GameObject_MapObject"),
					ENUM_CLASS(eLevel), TEXT("Layer_Parkour"), &pDesc)))
					CRASH("Failed Ready Parkour Object");
				/*m_pGameInstance->Clone_Prototype(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject")
					, PROTOTYPE::GAMEOBJECT, &pDesc);*/
				break;

			default: // OBJECTTYPE::DEFAULT
				m_pGameInstance->Clone_Prototype(pDesc.iLevel, TEXT("Prototype_GameObject_MapObject")
					, PROTOTYPE::GAMEOBJECT, &pDesc);
				break;

			}
		});
	}
	m_pGameInstance->Wait_Thread_End();
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