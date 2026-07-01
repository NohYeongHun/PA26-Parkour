#include "ClientPch.h"
#include "Loader_Test.h"

#pragma region PLAYER

#pragma endregion


#pragma region OBJECT
//#include "MapObject_Instance.h"
//#include "MapObject.h"
//#include "Trigger_Box.h"
//#include "MapObject_Collaps.h"

#pragma endregion



#include "GameSystem.h"

CLoader_Test::CLoader_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLoader { pDevice, pContext }
{
}

HRESULT CLoader_Test::Initialize()
{
	m_iNumLoadingThread = 18;

	m_pGameInstance->Add_Work([this]() {Load_Texture(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Model(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Shader(); Complete_Load(); });
	m_pGameInstance->Add_Work([this]() {Load_Object(); Complete_Load(); });
    m_pGameInstance->Add_Work([this]() {Load_Player(); Complete_Load(); });
    return S_OK;
}

HRESULT CLoader_Test::Load_Texture()
{
	cout << "Texture" << endl;
    
    return S_OK;
}

HRESULT CLoader_Test::Load_Model()
{

	m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/Parkour/", m_eCurLevel, "Parkour");
	//m_pGameInstance->Load_Resource("../Bin/Resource/Map/The_False_Sovereign/Textures/");
	
	//m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/PLAYER_TEST/", m_eCurLevel, "The_False_Sovereign");

	//m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/Asphodel_Barrens_1102_first/", m_eCurLevel);
	//m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/Total_Map_1102/", m_eCurLevel);
	//m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/The_False_Sovereign_1102_final/", m_eCurLevel);
	//m_pGameSystem->Ready_Prototype_Map("../Bin/Resource/Map/MapData/INSTANCE_TEST/", m_eCurLevel);

    // Prototype_Component_Model_FalseSoverign
    //_fmatrix PreMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
    //if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Model_FalseSoverign"),
    //    CModel::Create(m_pDevice, m_pContext, MODELTYPE::ANIM, PreMatrix, "../Bin/Resource/Model/Player/FalseSovereign/False_SovereignTest1.dat"))))
    //    return E_FAIL;

	cout << "Model" << endl;

    return S_OK;
}

HRESULT CLoader_Test::Load_Shader()
{
	cout << "Shader" << endl;

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl")
            , VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
    {
        CRASH("Failed Load AnimMesh Shader");
        return E_FAIL;
    }


    return S_OK;
}

HRESULT CLoader_Test::Load_Object()
{
 //   m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject"),
 //       CMapObject::Create(m_pDevice, m_pContext));

	//m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Instance"),
	//	CMapObject_Instance::Create(m_pDevice, m_pContext));
	//
	//m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_TriggerBox"),
	//	CTrigger_Box::Create(m_pDevice, m_pContext));

	//m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Collaps"),
	//	CMapObject_Collaps::Create(m_pDevice, m_pContext));

	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_Dummy"),
	//	CDummy::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;

	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_WeaponDummy"),
	//	CWeaponDummy::Create(m_pDevice, m_pContext))))
	//	return E_FAIL;
	////if(FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MonsterTest"), CMonsterTest::Create(m_pDevice, m_pContext))))
	////	return E_FAIL;

	//// Prototype_GameObject_AttackVolume
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::TEST), TEXT("Prototype_GameObject_AttackVolume"),
	//	CAttackVolume::Create(m_pDevice, m_pContext))))
	//	CRASH("AttackVolume Create Failed");

	//m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_MapObject_Throw"),
	//	CMapObject_Throw::Create(m_pDevice, m_pContext));

	//m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_GameObject_Slide_Navigation"),
	//	CSlide_Navigation::Create(m_pDevice, m_pContext));
	//cout << "Object" << endl;

    return S_OK;
}




HRESULT CLoader_Test::Load_Player()
{

    // Controller 초기화
    _wstring wstrControllerTag = L"Prototype_Component_PlayerController";
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), wstrControllerTag,
        CInputController::Create(m_pDevice, m_pContext))))
        CRASH("PlayerInput Controller");
    
    /*_wstring wStrControllerTag = TEXT("Prototype_GameObject_Player");
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel)
        , wStrControllerTag
        , CPlayer::Create(m_pDevice, m_pContext))))
        CRASH("Prototype Create Failed");*/


    return S_OK;
}


CLoader_Test* CLoader_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLoader_Test* pInstance = new CLoader_Test(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Loader_Test");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoader_Test::Free()
{
    __super::Free();
}