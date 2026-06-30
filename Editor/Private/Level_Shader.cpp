#include "EditorPch.h"
#include "Level_Shader.h"

#include "Event_Level.h"
#include "Shader_Interface.h"

//Dummy
#include "EditDummy_Wolf.h"
#include "EditDummy_Augusta.h"
#include "EditDummy_Map.h"
#include "TestVA.h"

CLevel_Shader::CLevel_Shader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Shader::Initialize()
{
	m_pGameInstance->Load_Resource("../../Client/Bin/Resource/Map/Asphodel_Barrens/");

	if (FAILED(Ready_Light()))
        CRASH("Failed Light");

    if (FAILED(Ready_Interface()))
        CRASH("Failed Interface");

    if(FAILED(Ready_TestObjects()))
        CRASH("Failed TestObject");

	CEditDummy_Augusta::DUMMY_AUGU_DESC AuguDesc = {};
	_matrix PreTransformationMatrix = XMMatrixScalingFromVector(XMVectorSet(0.01f, 0.01f, 0.01f, 1.f));
	
	//if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::SHADER), TEXT("Prototype_Test_VAMesh"),
	//	CVAMesh::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Effect/VA/Augusta_Light_20005.dat"), PreTransformationMatrix, 1))))
	//	return E_FAIL;

	/*CTestVA::VA_DESC Desc = {};

	Desc.strMeshTag = TEXT("Prototype_Test_VAMesh");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::SHADER), TEXT("Prototype_TestVA"), CTestVA::Create(m_pDevice, m_pContext))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_PoolingObject(ENUM_CLASS(LEVEL::SHADER), TEXT("Prototype_TestVA"), ENUM_CLASS(LEVEL::SHADER), TEXT("Layer_TEST"),
		TEXT("Poolling_Test"), 1)))
		return E_FAIL;*/

	m_pGameInstance->SettingFog(0);

    return S_OK;
}

void CLevel_Shader::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Shader"));
    m_pShader_Interface->Setting_Shader();

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD8) == KEYSTATE::DOWN)
	{
		m_pGameInstance->Set_LightActive(TEXT("Test"), false);
		//m_pGameInstance->Spawn_PoolingObject(TEXT("Poolling_Test"), XMMatrixIdentity());
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD9) == KEYSTATE::DOWN)
	{
		m_pGameInstance->Set_LightActive(TEXT("Test"), true);
		//m_pGameInstance->Spawn_PoolingObject(TEXT("Poolling_Test"), XMMatrixIdentity());
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD7) == KEYSTATE::DOWN)
	{
		m_TestLight.fRange = max(m_TestLight.fRange - 1.f, 1.f);
		m_pGameInstance->Update_LightDesc(TEXT("Test1"), m_TestLight);
		//m_pGameInstance->Spawn_PoolingObject(TEXT("Poolling_Test"), XMMatrixIdentity());
	}
}

void CLevel_Shader::Render()
{

}

HRESULT CLevel_Shader::Ready_Light()
{
    LIGHT_DESC LightDesc{};
    LightDesc.eType = LIGHT_DESC::DIRECTION;
    LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
	LightDesc.vDiffuse = _float4(0.8f, 0.2f, 0.2f, 1.f);//_float4(0.8f, 0.7f, 0.12f, 1.f);
	LightDesc.vDirection = _float4(0.f, -0.5f, 0.5f, 0.f);
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

    m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
    m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
    m_pGameInstance->SetUp_CameraNF();

	m_TestLight.eType = LIGHT_DESC::POINT;
	m_TestLight.fRange = 10.f;
	m_TestLight.vPosition = _float4(0.f, 0.f, 0.f, 1.f);
	m_TestLight.vDiffuse = _float4(1.f, 0.f, 0.f, 1.f);
	m_TestLight.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	m_TestLight.vAmbient = _float4(0.1f, 0.1f, 0.1f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test1"), m_TestLight);

	/*LIGHT_DESC PointLight = {};
	PointLight.eType = LIGHT_DESC::POINT;
	PointLight.vAmbient = _float4(0.8f, 0.8f, 0.8f, 1.f);
	PointLight.vDiffuse = _float4(0.f, 0.f, 0.7f, 1.f);
	PointLight.fRange = 1500.f;
	PointLight.vPosition = _float4(340.f, 230.f, 500.f, 1.f);

	LightDesc.vDirection = _float4(1.f, -0.5f, -1.f, 0.f);
	PointLight.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test1"), PointLight);*/

	//PointLight.vDiffuse = _float4(0.f, 0.7f, 0.f, 1.f);
	//PointLight.vPosition = _float4(-340.f, 230.f, 500.f, 1.f);
	//m_pGameInstance->Add_Light(TEXT("Test2"), PointLight);

	return S_OK;
}

HRESULT CLevel_Shader::Ready_Interface()
{
    m_pShader_Interface = CShader_Interface::Create(m_pDevice, m_pContext);
    ASSERT_CRASH(m_pShader_Interface);

    return S_OK;
}

HRESULT CLevel_Shader::Ready_TestObjects()
{
    CEditDummy_Augusta::DUMMY_AUGU_DESC AuguDesc = {};
    _matrix PreTransformationMatrix = XMMatrixScalingFromVector(XMVectorSet(0.0001f, 0.0001f, 0.0001f, 1.f)) * XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(180.f), 0.f));
    AuguDesc.PreTransformMatrix = PreTransformationMatrix;

    if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Augu"),
                                                       ENUM_CLASS(LEVEL::SHADER), TEXT("Layer_Dummy"), &AuguDesc)))
        CRASH("Failed Clone Dummy Wolf");

    //AuguDesc.vPosition = XMVectorSet(0.f, -120.f, 0.f, 1.f);
    //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Augu"),
    //                                                   ENUM_CLASS(LEVEL::SHADER), TEXT("Layer_Dummy"), &AuguDesc)))
    //    CRASH("Failed Clone Dummy Wolf");

    //CEditDummy_Map::DUMMY_MAP_DESC MapDesc = {};
    //PreTransformationMatrix = XMMatrixScalingFromVector(XMVectorSet(0.01f, 0.01f, 0.01f, 1.f)) * XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(0.f, XMConvertToRadians(180.f), 0.f))
    //    * XMMatrixTranslationFromVector(XMVectorSet(0.f, -10.f, 0.f, 1.f));
    //MapDesc.PreTransformMatrix = PreTransformationMatrix;

    //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Map"),
    //                                                   ENUM_CLASS(LEVEL::SHADER), TEXT("Layer_Dummy"), &MapDesc)))
    //    CRASH("Failed Clone Dummy Wolf");

    return S_OK;
}

CLevel_Shader* CLevel_Shader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Shader* pInstance = new CLevel_Shader(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Shader");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Shader::Free()
{
    __super::Free();

    Safe_Release(m_pShader_Interface);
}
