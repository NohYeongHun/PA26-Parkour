#include "EditorPch.h"
#include "Level_Camera.h"

#include "SpringCamera_Edit.h"
#include "EditDummy_Wolf.h"
#include "EditDummy_Map.h"
#include "EditDummy_Target.h"

#include	"Map_Interface.h"
#include	"Camera_Interface.h"
#include "AnimationTool.h"

#include "Sequencer.h"

#include "SQ_Camera_Edit.h"

CLevel_Camera::CLevel_Camera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Camera::Initialize()
{
	//Ready_Camera();
	Ready_Prototype();
	Ready_Light();
	//Ready_Dummy();
	//Ready_Ground();

	m_pMapInterface = CMap_Interface::Create(m_pDevice, m_pContext);
	m_pCameraInterface = CCamera_Interface::Create(m_pDevice, m_pContext);
	m_pAnimationTool = CAnimationTool::Create(m_pDevice, m_pContext, LEVEL::CAMERA);
	m_pMapInterface->SetPrototypes(m_pGameInstance->Get_CurrentLevel());
	m_pSequencer = CSequencer::Create();

    return S_OK;
}

void CLevel_Camera::Update(_float fTimeDelta)
{
	SetWindowText(g_hWnd, TEXT("Camera"));

	ImGui::Begin("Camera Edit");

	if (ImGui::Button("MapInterface"))
		m_isMapInterface = !m_isMapInterface;

	if(true == m_isMapInterface)
	{
		/*if (m_pMapInterface->Initialize_ModelPath(ENUM_CLASS(LEVEL::CAMERA), XMMatrixScalingFromVector(XMVectorSet(0.1f, 0.1f, 0.1f, 1.f))))
			m_pMapInterface->Add_MapObject();*/
		m_pMapInterface->Load_Map_GUI();
		//임시로 여기 추가. 버튼 누르면 다른 맵 부르거나, 맵 안부르고 바로 끌 수 있음.(그냥 맵 선택 창 키고 끄는 용도)
		if (ImGui::Button("Change Map Mode? "))
			m_pMapInterface->Load_Another_Map();
		//if(ImGui::Button("Load Map"))
		//	m_pMapInterface->Load_Map_GUI();
	}

	ImGui::End();

	m_pSequencer->Update(fTimeDelta);
}

void CLevel_Camera::Render()
{
	m_pAnimationTool->Render();
}

void CLevel_Camera::Ready_Prototype()
{
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CAMERA), TEXT("Prototype_Component_Shader_VtxAnimMeshCharacter"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMeshCharacter.hlsl")
			, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
	{
		CRASH("Failed Load AnimMesh Shader");
	}

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CAMERA), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl")
			, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
	{
		CRASH("Failed Load AnimMesh Shader");
	}

	SHADER_MACRO eShaderMacro = {
		{"THREAD_X", "64" }
		,{"THREAD_Y", "1" }
		,{"THREAD_Z", "1" }
		, { NULL, NULL }
	};

	string strEntryPoint = "CSMain";
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CAMERA), TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"),
		CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMesh.hlsl")
			, eShaderMacro, strEntryPoint))))
	{
		CRASH("Failed Load AnimMesh Shader");
	}

	// Sequence Item
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CAMERA), TEXT("Prototype_GameObject_SceneCamera"),
		CSQ_Camera_Edit::Create(m_pDevice, m_pContext))))
		CRASH("Camera");
}

void CLevel_Camera::Ready_Light()
{
	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;
	LightDesc.vAmbient = _float4(0.7f, 0.7f, 0.7f, 1.f);
	LightDesc.vDiffuse = _float4(0.8f, 0.8f, 1.f, 1.f);
	LightDesc.vDirection = _float4(1.f, -0.5f, -1.f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_CameraNF();
}

void CLevel_Camera::Ready_Dummy()
{
	CEditDummy_Wolf::DUMMY_WOLF_DESC WolfDesc = {};
	_matrix PreTransformationMatrix = XMMatrixScalingFromVector(XMVectorSet(0.005f, 0.005f, 0.005f, 1.f));
	WolfDesc.PreTransformMatrix = PreTransformationMatrix;

	WolfDesc.fSpeedPerSec = 100.f;
	WolfDesc.vPosition = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Wolf"),
		ENUM_CLASS(LEVEL::CAMERA), TEXT("Layer_Dummy"), &WolfDesc)))
		CRASH("Failed Clone Dummy Wolf");

	//CEditDummy_Target::DUMMY_TARGET_DESC TargetDesc = {};
	//TargetDesc.PreTransformMatrix = PreTransformationMatrix;
	//TargetDesc.vPosition = XMVectorSet(40.f, 0.f, 0.f, 1.f);
	//if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Target"),
	//	ENUM_CLASS(LEVEL::CAMERA), TEXT("Layer_Dummy"), &TargetDesc)))
	//	CRASH("Failed Clone Dummy Target");

	// Dummy Map
	//PreTransformationMatrix = XMMatrixScalingFromVector(XMVectorSet(0.05f, 0.05f, 0.05f, 1.f));
	//CEditDummy_Map::DUMMY_MAP_DESC MapDesc = {};
	//MapDesc.PreTransformMatrix = PreTransformationMatrix;
	//MapDesc.vPosition = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	//if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Dummy_Map"),
	//	ENUM_CLASS(LEVEL::CAMERA), TEXT("Layer_Dummy"), &MapDesc)))
	//	CRASH("Failed Clone Dummy Map");
}

void CLevel_Camera::Ready_Ground()
{
	CRigidbody::BOXBODY_DESC BoxBodyDesc = {};
	BoxBodyDesc.eShape = SHAPE::BOX;
	BoxBodyDesc.vPos = _float3(0.f, -50.f, 0.f);
	BoxBodyDesc.vExtent = _float3(1000.f, 10.f, 1000.f);
	BoxBodyDesc.eType = EMotionType::Static;
	BoxBodyDesc.iLayer = ENUM_CLASS(COLLISIONLAYER::MAP);
	
	m_pGround = CRigidbody::Create(m_pDevice, m_pContext);
	m_pGround->Initialize_Clone(&BoxBodyDesc);
}

CLevel_Camera* CLevel_Camera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Camera* pInstance = new CLevel_Camera(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Level_Camera");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Camera::Free()
{
	__super::Free();

	Safe_Release(m_pGround);

	Safe_Release(m_pMapInterface);
	Safe_Release(m_pCameraInterface);
	Safe_Release(m_pAnimationTool);

	Safe_Release(m_pSequencer);
}
