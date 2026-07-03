#include "ClientPch.h"
#include "GameSystem.h"
#include "Level_Test.h"
#include "MapObject.h"
#include "Traceur.h"

//#include "Player.h"


CLevel_Test::CLevel_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :	CLevel(pDevice,pContext), m_pGameSystem { CGameSystem::GetInstance() }
{
	Safe_AddRef(m_pGameSystem);
}

HRESULT CLevel_Test::Initialize()
{
	// SetUp OctoTree
	m_pGameInstance->SetUp_OctoTree(_float3(0.f, 0.f, 0.f), _float3(1024, 1024, 1024));

	//로더에서 부른 것과 같은 거 부르기.
	m_pGameSystem->Clone_MapObjects(m_eCurLevel);

    Ready_Layer_Player();

    /*LIGHT_DESC LightDesc{};
    LightDesc.eType = LIGHT_DESC::DIRECTION;
    LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(1.f, -0.5f, -1.f, 0.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

    m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
    m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
    m_pGameInstance->SetUp_CameraNF();*/

	// Test
	_uint iLevel = m_pGameInstance->Get_CurrentLevel(); 

    return S_OK;
}

void CLevel_Test::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("Test"));
    
#ifdef _DEBUG
	if (m_pGameInstance->Get_DIKeyState(DIK_F2) == KEYSTATE::DOWN)
	{
		m_pGameInstance->Set_LightActive(TEXT("Test"), true);
	}

#endif
	
}

void CLevel_Test::Render()
{
}


//void CLevel_Test::Ready_Skybox()
//{
//	CSkyBox::SKYBOX_DESC SkyboxDesc = {};
//	SkyboxDesc.iNumModel = 2;
//	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_Dome"));
//	SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_Background"));
//	//SkyboxDesc.strModelTags.push_back(TEXT("Prototype_Component_Model_Skybox_FX2"));
//
//	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Skybox"), ENUM_CLASS(m_eCurLevel),
//		TEXT("Layer_BackGround"), &SkyboxDesc)))
//		CRASH("Skybox");
//}



HRESULT CLevel_Test::Ready_Layer_Map(const _char* pFilePath)
{
    _char FileDrive[MAX_PATH] = {};
    _char FileDir[MAX_PATH] = {};
    _char FileName[MAX_PATH] = {};
    _char FileExt[MAX_PATH] = {};

    _splitpath_s(pFilePath, FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

    _string PasingDir = FileDir;
    if (strlen(FileName) > 0)
    {
        PasingDir += FileName;
        PasingDir += FileExt;
        Read_Map_Dat(pFilePath);
    }
    else
    {
        for (const auto& entry : filesystem::recursive_directory_iterator(FileDir)) {
            if (!entry.is_regular_file())
                continue;

            if (entry.path().string().find("Prototype") != std::string::npos)
                continue;

            if (entry.path().extension() != ".dat")
                continue;

            _string strFilePath = entry.path().string();
            Read_Map_Dat(strFilePath);
        }
    }
    return S_OK;
}

void CLevel_Test::Read_Map_Dat(const _string pFilePath)
{
    ifstream File(pFilePath, ios::binary);

    if (!File.is_open())
    {
        MSG_BOX("Load Failed");
    }
    if (pFilePath.find("Instance") != std::string::npos)
    {
        return;


    }
    else
    {
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
			_float4x4 Matrix = {};
			File.read(reinterpret_cast<char*>(&Matrix), sizeof(_float4x4));
			Desc.WorldMatrix = &Matrix;

			//프로토타입은 제일 큰 놈으로 들어옴. => 0번까지 계속 생성.
			_wstring ModelName = StringToWString(Desc.ModelName);

			m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex, eObjectType = Desc.eObjectType, Matrix = *Desc.WorldMatrix]() mutable {
				CMapObject::MAP_LOAD pDesc{};
				strcpy_s(pDesc.ModelName, ModelName.c_str());
				pDesc.iShaderPassIndex = ShaderPass;
				pDesc.eObjectType = eObjectType;
				pDesc.WorldMatrix = &Matrix;

				CMapObject* pMapObject = static_cast<CMapObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject")
					, PROTOTYPE::GAMEOBJECT, &pDesc));
				
				});
        }
    }
    m_pGameInstance->Wait_Thread_End();
    File.close();
}

void CLevel_Test::Ready_Layer_Player()
{
	_float3 vScale{}, vRotation{}, vPosition{};
	vScale = { 1.f, 1.f, 1.f };
	vRotation = { 0.f, 0.f, 0.f };
	vPosition = { 3.f, 1.f, 3.f };

	CCharacter::CHARACTER_DESC Desc{};
	Desc.eCurLevel = m_eCurLevel;
	Desc.modelData = make_pair(m_eCurLevel, TEXT("Prototype_Component_Model_Traceur"));
	Desc.shaderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"));
	Desc.colliderData = make_pair(LEVEL::STATIC, TEXT("Prototype_Component_Collider"));
	Desc.inputControllerData = make_pair(m_eCurLevel, TEXT("Prototype_Component_PlayerController"));

	Desc.fRotationPerSec = XMConvertToRadians(90.f);
	Desc.fSpeedPerSec = 10.f;
	Desc.vScale = vScale;
	Desc.vRotation = vRotation;
	Desc.vPosition = vPosition;

	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Traceur"),
		ENUM_CLASS(m_eCurLevel), TEXT("Layer_Traceur"), &Desc)))
		CRASH("Failed Ready Player");
}

CLevel_Test* CLevel_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Test* pInstance = new CLevel_Test(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Test");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Test::Free()
{
    __super::Free();
    Safe_Release(m_pGameSystem);
	
}
