#include "EditorPch.h"
#include "Level_Effect.h"
#include "Event_Level.h"
#include "Effect_Controller.h"
#include "Particle.h"
#include "Effect_Mesh.h"
#include "ComputeShader.h"
#include "AnimationTool.h"
#include "Trail_Mesh.h"

#include"Edit_MapObject.h"
#include "Effect_Rect.h"
#include "Effect_Decal.h"
#include "Effect_Radial.h"
#include"Map_Interface.h"
#include "TestVA.h"
#include "Effect_Light.h"
#include "Spectrum.h"

CLevel_Effect::CLevel_Effect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Effect::Initialize()
{
    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Prefab"),
        CEffect_Prefab::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_Particle"),
        CParticle::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectMesh"),
        CEffect_Mesh::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_TrailMesh"),
        CTrail_Mesh::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectRect"),
		CEffect_Rect::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Componnent_VIBuffer_FXRect"),
		CVIBuffer_Point::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectDecal"),
		CEffect_Decal::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectRadial"),
		CEffect_Radial::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectVA"),
		CTestVA::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectLight"),
		CEffect_Light::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectSpectrum"),
		CSpectrum::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxInstance_PointParticle"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxInstance_PointParticle.hlsl"), VTXPOINTPARTICLE::Elements, VTXPOINTPARTICLE::iNumElements));

    //�Ž� �׸���� ���̴�
    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxInstance_FXMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxFXMesh_Instance.hlsl"), VTXFXMESHINSTANCE::Elements, VTXFXMESHINSTANCE::iNumElements));

    //�Ϲ� �Ž� �׸���� ���̴� �߰��������.
    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxTrailMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxTrailMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));
	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxFXRect"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxFXRect.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements));
        
    SHADER_MACRO eShaderMacro = {
        {"THREAD_X", "64" }
        ,{"THREAD_Y", "1" }
        ,{"THREAD_Z", "1" }
        , { NULL, NULL }
    };

    string strEntryPoint = "main";

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_ComputeShader_Particle"),
        CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ParticleUpdate_CS.hlsl"), eShaderMacro, strEntryPoint));


    //FX�Ž� ����� ���̴�

    SHADER_MACRO eShaderMacroMesh = {
      {"THREAD_X", "64" }
      ,{"THREAD_Y", "1" }
      ,{"THREAD_Z", "1" }
      , { NULL, NULL }
    };

    string strEntryPointMesh = "main";

    m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Shader_ComputeShader_FXMesh"),
        CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_FXMeshUpdate_CS.hlsl"), eShaderMacroMesh, strEntryPointMesh));

    //����Ʈ ��
    m_pEffect_Controller = CEffect_Controller::Create(m_pDevice, m_pContext);


    //��ƼŬ ������ �� ��ġ���� ������ ���� ���� �÷��̾� ������ �߰���. ���ƿ����� ���� �ִϸ��̼� ��
    m_pAnimation_Tool = CAnimationTool::Create(m_pDevice, m_pContext, LEVEL::EFFECT);
    m_pAnimation_Tool->Set_EffectContorller(m_pEffect_Controller);

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl")
            , VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
    {
        CRASH("Failed Load AnimMesh Shader");
        return E_FAIL;
    }
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Component_Shader_VtxAnimMeshCharacter"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMeshCharacter.hlsl")
			, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
	{
		CRASH("Failed Load AnimMesh Shader");
		return E_FAIL;
	}

    //�ִϸ��̼� ����� ���̴�
    SHADER_MACRO eShaderMacroB = {
    {"THREAD_X", "64" }
    ,{"THREAD_Y", "1" }
    ,{"THREAD_Z", "1" }
    , { NULL, NULL }
    };

    string strEntryPointB = "CSMain";
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Component_Shader_ComputeVtxAnimMesh"),
        CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ComputeVtxAnimMesh.hlsl")
            , eShaderMacroB, strEntryPointB))))
    {
        CRASH("Failed Load AnimMesh Shader");
        return E_FAIL;
    }
	m_pMap_Interface = CMap_Interface::Create(m_pDevice, m_pContext);


	Ready_Map("../../Client/Bin/Resource/Map/MapData/Effect_Map/");


	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;
	LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vDirection = _float4(0.f, -1.f, 0.5f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_CameraNF();

    return S_OK;
}

void CLevel_Effect::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Effect"));

        m_pEffect_Controller->Update();

		if (m_pGameInstance->Get_DIKeyState(DIK_F2) == KEYSTATE::DOWN)
		{
			m_pGameInstance->Set_LightActive(TEXT("Test"), true);
		}
		if (m_pGameInstance->Get_DIKeyState(DIK_F3) == KEYSTATE::DOWN)
		{
			m_pGameInstance->Set_LightActive(TEXT("Test"), false);
		}

}

void CLevel_Effect::Render()
{
    m_pAnimation_Tool->Render();
}

void CLevel_Effect::Ready_Map(const _char* pFilePath)
{
	m_pMap_Interface->SetPrototypes(m_pGameInstance->Get_CurrentLevel());
	m_pGameInstance->Load_Resource("../../Client/Bin/Resource/Map/Asphodel_Barrens/");
	m_pMap_Interface->Ready_Map_Prototype("../../Client/Bin/Resource/Map/Asphodel_Barrens/");
	m_pGameInstance->LoadLastLOD();

	_char FileDrive[MAX_PATH] = {};
	_char FileDir[MAX_PATH] = {};
	_char FileName[MAX_PATH] = {};
	_char FileExt[MAX_PATH] = {};

	_splitpath_s(pFilePath, FileDrive, MAX_PATH, FileDir, MAX_PATH, FileName, MAX_PATH, FileExt, MAX_PATH);

	_string PasingDir = FileDir;

	_string ProjectPath = filesystem::current_path().parent_path().parent_path().string();
	ProjectPath += "/Client/Bin/Resource/Map";
	_float fSize = 0.01f;
	_matrix PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);
	
	_wstring PrototypeName = L"Prototype_Component_Model_";
	_wstring InstancePrototypeName = L"Prototype_Component_Model_Instance_";
	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_MapObject"),
		CEdit_MapObject::Create(m_pDevice, m_pContext));

	m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_Component_Shader_NonAnimMesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));

	//for (const auto& entry : filesystem::directory_iterator(PasingDir)) {
	//	if (!entry.is_regular_file())
	//		continue;
	//	if (entry.path().string().find("Prototype") == std::string::npos)
	//		continue;

	//	_string strFilePath = entry.path().string();
	//	ifstream File(strFilePath, ios::binary);

	//	_uint NameLength = {};

	//	_char Name[MAX_PATH] = {};
	//	while (File.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint)))
	//	{
	//		memset(Name, 0, sizeof(Name));
	//		File.read(reinterpret_cast<_char*>(&Name), NameLength);
	//		//여기서 프로토타입 생성.
	//		_uint ProtoMax = Name[strlen(Name) - 1] - '0' + 1;
	//		_string ModelName = Name;
	//		ModelName.pop_back();

	//		for (const auto& entry2 : filesystem::recursive_directory_iterator(ProjectPath)) {
	//			if (entry2.path().string().find("MapData") != std::string::npos)
	//				continue;
	//			if (entry2.path().string().find("Test") != std::string::npos)
	//				continue;

	//			if (entry2.path().string().find(ModelName) == std::string::npos)
	//				continue;

	//			if (entry2.path().extension() != ".dat")
	//				continue;

	//			_string Path = entry2.path().string();
	//			_string Prototype = entry2.path().stem().string();
	//			//파서 수정중
	//			if (entry2.path().string().find("Instance") == std::string::npos)
	//			{
	//				m_pGameInstance->Add_Work([=, Model = PrototypeName + StringToWString(Prototype), ModelPath = Path]() {
	//					if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), PrototypeName + StringToWString(Prototype),
	//						CModel::Create(m_pDevice, m_pContext, MODELTYPE::MAP, PreTransformMatrix, ModelPath.c_str()))))
	//						CRASH("Prototype Create Failed");
	//					});
	//			}
	//			else if (entry2.path().string().find("Instance") != std::string::npos)
	//			{
	//				m_pGameInstance->Add_Work([=, Model = InstancePrototypeName + StringToWString(Prototype), ModelPath = Path]() {
	//					if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Model,
	//						CModel_Instance::Create(m_pDevice, m_pContext, PreTransformMatrix, ModelPath.c_str()))))
	//						CRASH("Prototype Create Failed");
	//					});
	//			}
	//			//프로토타입 생성
	//		}
	//	}
	//	File.close();
	//}
	//m_pGameInstance->Wait_Thread_End();

	for (const auto& entry : filesystem::recursive_directory_iterator(FileDir)) {
		if (!entry.is_regular_file())
			continue;

		if (entry.path().extension() != ".dat")
			continue;

		if (entry.path().string().find("Prototype") != std::string::npos)
			continue;

		_string strFilePath = entry.path().string();

		ifstream File(strFilePath, ios::binary);

		if (!File.is_open())
		{
			MSG_BOX("Load Failed");
		}

		_uint NameLength;

		_matrix PreTransformMatrix = XMMatrixIdentity();
		_float fSize = 0.01f;
		PreTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);

		if (strFilePath.find("Instance") != std::string::npos)
		{
			return;
		}
		else
		{
			CEdit_MapObject::MAP_LOAD Desc{};

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
				Desc.iLevel = ENUM_CLASS(LEVEL::EFFECT);
				//m_pGameInstance->Add_Work([&, ModelName = string(Desc.ModelName), ShaderPass = Desc.iShaderPassIndex, eObjectType = Desc.eObjectType, Matrix = *Desc.WorldMatrix]() mutable {
				//	CEdit_MapObject::MAP_LOAD pDesc{};
				//	strcpy_s(pDesc.ModelName, ModelName.c_str());
				//	pDesc.iShaderPassIndex = ShaderPass;
				//	pDesc.eObjectType = eObjectType;
				//	pDesc.WorldMatrix = &Matrix;
				//	pDesc.iLevel = ENUM_CLASS(LEVEL::EFFECT);
					m_pGameInstance->Add_GameObject_ToLayer(Desc.iLevel, TEXT("Prototype_GameObject_MapObject"), Desc.iLevel, TEXT("Layer_Map"), &Desc);
					//});
			}
		}
		m_pGameInstance->Wait_Thread_End();
		File.close();
	}
}


CLevel_Effect* CLevel_Effect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Effect* pInstance = new CLevel_Effect(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Effect");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Effect::Free()
{
    __super::Free();

    Safe_Release(m_pEffect_Controller);
	Safe_Release(m_pAnimation_Tool);
	Safe_Release(m_pMap_Interface);
}
