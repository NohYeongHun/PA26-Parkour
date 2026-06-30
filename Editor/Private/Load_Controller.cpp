#include "EditorPch.h"
#include "Load_Controller.h"

CLoad_Controller::CLoad_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CLoad_Controller::Initialize(LEVEL eCurrentLevel)
{
    m_eCurrentLevel = eCurrentLevel;

    if (eCurrentLevel == LEVEL::EFFECT)
        return S_OK;

    //다른 레벨에서 컨트롤러 불러오면 이펙트에 필요한 파일들 미리 읽어둬야함.

    m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurrentLevel), TEXT("Prototype_GameObject_Prefab"),
        CEffect_Prefab::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurrentLevel), TEXT("Prototype_GameObject_Particle"),
        CParticle::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurrentLevel), TEXT("Prototype_GameObject_EffectMesh"),
        CEffect_Mesh::Create(m_pDevice, m_pContext));

    m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurrentLevel), TEXT("Prototype_GameObject_TrailMesh"),
        CTrail_Mesh::Create(m_pDevice, m_pContext));

    //파티클 그리기용 셰이더
    m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurrentLevel), TEXT("Prototype_Shader_VtxInstance_PointParticle"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxInstance_PointParticle.hlsl"), VTXPOINTPARTICLE::Elements, VTXPOINTPARTICLE::iNumElements));

    //매쉬 그리기용 셰이더
    m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurrentLevel), TEXT("Prototype_Shader_VtxInstance_FXMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxFXMesh_Instance.hlsl"), VTXFXMESHINSTANCE::Elements, VTXFXMESHINSTANCE::iNumElements));

    //일반 매쉬 그리기용 셰이더 추가해줘야함.
    m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurrentLevel), TEXT("Prototype_Shader_VtxTrailMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxTrailMesh.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements));

    //파티클 연산용 셰이더
    SHADER_MACRO eShaderMacro = {
        {"THREAD_X", "64" }
        ,{"THREAD_Y", "1" }
        ,{"THREAD_Z", "1" }
        , { NULL, NULL }
    };

    string strEntryPoint = "main";

    m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurrentLevel), TEXT("Prototype_Shader_ComputeShader_Particle"),
        CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_ParticleUpdate_CS.hlsl"), eShaderMacro, strEntryPoint));

    //FX매쉬 연산용 셰이더
    SHADER_MACRO eShaderMacroMesh = {
      {"THREAD_X", "64" }
      ,{"THREAD_Y", "1" }
      ,{"THREAD_Z", "1" }
      , { NULL, NULL }
    };

    string strEntryPointMesh = "main";

    m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurrentLevel), TEXT("Prototype_Shader_ComputeShader_FXMesh"),
        CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_FXMeshUpdate_CS.hlsl"), eShaderMacroMesh, strEntryPointMesh));

    return S_OK;
}

void CLoad_Controller::Update()
{

}

void CLoad_Controller::Render()
{

}

void CLoad_Controller::Load_TrailMesh_AllTextureFromFolder(const _string& strFolderPath)
{
}

void CLoad_Controller::Load_TrailMesh_AllMeshDatFromFolder(const _string& strFolderPath)
{
}

void CLoad_Controller::Load_TrailMesh_AllColorTextureFormFolder(const _string& strFolderPath)
{
}

void CLoad_Controller::Prefab_Load_Tab(_bool* IsLoad)
{
    if (ImGui::Button("Load Prefab"))
    {
        IGFD::FileDialogConfig config;
        config.path = "../../Client/Bin/Resource/Effect/Prefab";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        ImGuiFileDialog::Instance()->OpenDialog("Load Prefab", "Import", ".json", config);
    }

    if (ImGuiFileDialog::Instance()->Display("Load Prefab", ImGuiWindowFlags_NoCollapse))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            _string strFilePath = {};
            _string strFolderPath = {};

            strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

            size_t lastSlashPos = strFilePath.find_last_of("\\");

            //마지막 문자열 빼고 폴더 경로만 가져오기. 
            if (lastSlashPos != string::npos) {
                strFolderPath += strFilePath.substr(0, lastSlashPos);
            }

            Load_Prefab_FromJson(strFilePath, strFolderPath);

            if(m_eCurrentLevel == LEVEL::EFFECT)
                *IsLoad = true;


        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void CLoad_Controller::Load_Prefab_FromJson(const _string& strFilePath, const _string& strFolderPath)
{
    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json PrefabJson;
    JsonStream >> PrefabJson;
    JsonStream.close();

    CEffect_Prefab::PREFAB_DESC PrefabDesc = {};
    
    if (PrefabJson.contains("Prefab_Name"))
        PrefabDesc.strPrefabTag = StringToWString(PrefabJson["Prefab_Name"].get<string>());

    if(PrefabJson.contains("Children_Number"))
        PrefabDesc.ChildrenCount = PrefabJson["Children_Number"].get<_int>();

    if(PrefabJson.contains("Bone_Name"))
        PrefabDesc.strBoneTag = PrefabJson["Bone_Name"].get<string>();

	if (PrefabJson.contains("Loop"))
		PrefabDesc.IsLoop = PrefabJson["Loop"].get<_bool>();

    if (PrefabJson.contains("Prefab_LifeTime") && PrefabJson["Prefab_LifeTime"].is_array())
    {
        json LifeTime = PrefabJson["Prefab_LifeTime"];

        PrefabDesc.vLifeTime.x = LifeTime[0].get<_float>();
        PrefabDesc.vLifeTime.y = LifeTime[1].get<_float>();
    }

    if (PrefabJson.contains("Frames") && PrefabJson["Frames"].is_array())
    {
        for (size_t i = 0; i < PrefabDesc.ChildrenCount; i++)
        {
            CEffect_Prefab::FRAME_DESC FrameDesc = {};

            json Frame = PrefabJson["Frames"][i];

            if (Frame.contains("Children_Name"))
                FrameDesc.strChildrenTag = StringToWString(Frame["Children_Name"].get<string>());

            if (Frame.contains("Children_Type"))
                FrameDesc.eChildrenType = static_cast<EFFECT_TYPE>(Frame["Children_Type"].get<double>());

            if (Frame.contains("Activate_Time"))
                FrameDesc.fActivateTime = Frame["Activate_Time"].get<double>();

            if (Frame.contains("Offset_Size") && Frame["Offset_Size"].is_array())
            {
                json Size = Frame["Offset_Size"];

                FrameDesc.vOffsetSize.x = Size[0].get<_float>();
                FrameDesc.vOffsetSize.y = Size[1].get<_float>();
                FrameDesc.vOffsetSize.z = Size[2].get<_float>();
            }

            if (Frame.contains("Offset_Position") && Frame["Offset_Position"].is_array())
            {
                json Pos = Frame["Offset_Position"];

                FrameDesc.vOffsetPos.x = Pos[0].get<_float>();
                FrameDesc.vOffsetPos.y = Pos[1].get<_float>();
                FrameDesc.vOffsetPos.z = Pos[2].get<_float>();
            }

            if (Frame.contains("Offset_Rotation") && Frame["Offset_Rotation"].is_array())
            {
                json Rot = Frame["Offset_Rotation"];

                FrameDesc.vOffsetRot.x = Rot[0].get<_float>();
                FrameDesc.vOffsetRot.y = Rot[1].get<_float>();
                FrameDesc.vOffsetRot.z = Rot[2].get<_float>();
            }

            FrameDesc.bActivated = false;          //처음엔 기본적으로 비활성화

            PrefabDesc.FrameDesc.push_back(FrameDesc);

            //이펙트 레벨에선 Desc 필요해서 저장 따로 처리
            if (m_eCurrentLevel == LEVEL::EFFECT)
                m_tPrefabDesc = PrefabDesc;

            //프리팹 정보 다 읽었고, 읽은 정보 토대로 자식 Save파일 읽기.
            if (FrameDesc.eChildrenType == EFFECT_TYPE::PARTICLE)
            {
                //VB저장
                _string ParticleVBPath = strFolderPath;
                ParticleVBPath += "/ParticleVB/";
                ParticleVBPath += WStringToString(FrameDesc.strChildrenTag);
                ParticleVBPath += ".json";

                Load_Particle_VB_FromJson(ParticleVBPath, FrameDesc.strChildrenTag);

                //OB저장
                _string ParticlePath = strFolderPath;
                ParticlePath += "/Particle/";
                ParticlePath += WStringToString(FrameDesc.strChildrenTag);
                ParticlePath += ".json";

                Load_Particle_OB_FromJson(ParticlePath, FrameDesc.strChildrenTag);
            }

            if (FrameDesc.eChildrenType == EFFECT_TYPE::MESH)
            {
                //VB저장
                _string FXMeshVBPath = strFolderPath;
                FXMeshVBPath += "/MeshVB/";
                FXMeshVBPath += WStringToString(FrameDesc.strChildrenTag);
                FXMeshVBPath += ".json";
                
                Load_FXMesh_VB_FromJson(FXMeshVBPath, FrameDesc.strChildrenTag);

                //OB저장
                _string FXMehsPath = strFolderPath;
                FXMehsPath += "/Mesh/";
                FXMehsPath += WStringToString(FrameDesc.strChildrenTag);
                FXMehsPath += ".json";

                Load_FXMesh_OB_FromJson(FXMehsPath, FrameDesc.strChildrenTag);

            }

            if (FrameDesc.eChildrenType == EFFECT_TYPE::TRAIL)
            {
                _string TrailMeshPath = strFolderPath;
                TrailMeshPath += "/TrailMesh/";
                TrailMeshPath += WStringToString(FrameDesc.strChildrenTag);
                TrailMeshPath += ".json";

                Load_TrailMesh_FromJson(TrailMeshPath, FrameDesc.strChildrenTag);
            }

			if (FrameDesc.eChildrenType == EFFECT_TYPE::RECT)
			{
				_string RectPath = strFolderPath;
				RectPath += "/FXRect/";
				RectPath += WStringToString(FrameDesc.strChildrenTag);
				RectPath += ".json";

				Load_FXRect_FromJson(RectPath, FrameDesc.strChildrenTag);
			}

			if (FrameDesc.eChildrenType == EFFECT_TYPE::DECAL)
			{
				_string DecalPath = strFolderPath;
				DecalPath += "/FXDecal/";
				DecalPath += WStringToString(FrameDesc.strChildrenTag);
				DecalPath += ".json";


				Load_FXDecal_FromJson(DecalPath, FrameDesc.strChildrenTag);
			}

			if (FrameDesc.eChildrenType == EFFECT_TYPE::RADIAL)
			{
				_string RadialPath = strFolderPath;
				RadialPath += "/FXRadial/";
				RadialPath += WStringToString(FrameDesc.strChildrenTag);
				RadialPath += ".json";

				Load_FXRadial_FromJson(RadialPath, FrameDesc.strChildrenTag);
			}

			if (FrameDesc.eChildrenType == EFFECT_TYPE::VA)
			{
				_string VAPath = strFolderPath;
				VAPath += "/FXVA/";
				VAPath += WStringToString(FrameDesc.strChildrenTag);
				VAPath += ".json";

				Load_FXVA_FromJson(VAPath, FrameDesc.strChildrenTag);
			}

			if (FrameDesc.eChildrenType == EFFECT_TYPE::LIGHT)
			{
				_string LightPath = strFolderPath;
				LightPath += "/FXLight/";
				LightPath += WStringToString(FrameDesc.strChildrenTag);
				LightPath += ".json";

				Load_FXLight_FromJson(LightPath, FrameDesc.strChildrenTag);
			}
        }
    }


    //프리팹 정보 다 읽었음. 자식들 다 찾아서 Desc 각각 생성 완료 한 후
    // 현재 이펙트 레벨일때 -> 이펙트 컨트롤러 통해서 자식들 세팅 해주고, 프리팹 Desc 저장해줘야함
    // 또한 자식들도 각각 맞는 컨트롤러에 Desc 저장해줘야. 추후 수정했을 때 저장가능.

    // 현재 다른 레벨 일때 -> 그냥 레이어에 추가해서 재생만 되게 하면 됨.

}

void CLoad_Controller::Load_Particle_VB_FromJson(const _string& strFilePath, const _wstring& ParticleTag)
{
    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json ParticleVBJson;
    JsonStream >> ParticleVBJson;
    JsonStream.close();

    CVIBuffer_Point_Instance::POINT_INSTANCE_DESC Desc = {};

    if (ParticleVBJson.contains("NumInstance"))
        Desc.iNumInstance = ParticleVBJson["NumInstance"].get<_int>();

    if (ParticleVBJson.contains("Center") && ParticleVBJson["Center"].is_array())
    {
        json CenterJson = ParticleVBJson["Center"];
        Desc.vCenter.x = CenterJson[0].get<_float>();
        Desc.vCenter.y = CenterJson[1].get<_float>();
        Desc.vCenter.z = CenterJson[2].get<_float>();
    }

    if (ParticleVBJson.contains("Range") && ParticleVBJson["Range"].is_array())
    {
        json RangeJson = ParticleVBJson["Range"];
        Desc.vRange.x = RangeJson[0].get<_float>();
        Desc.vRange.y = RangeJson[1].get<_float>();
        Desc.vRange.z = RangeJson[2].get<_float>();
    }

    if (ParticleVBJson.contains("Size") && ParticleVBJson["Size"].is_array())
    {
        json SizeJson = ParticleVBJson["Size"];
        Desc.vSize.x = SizeJson[0].get<_float>();
        Desc.vSize.y = SizeJson[1].get<_float>();
    }

    if (ParticleVBJson.contains("Pivot") && ParticleVBJson["Pivot"].is_array())
    {
        json PivotJson = ParticleVBJson["Pivot"];
        Desc.vPivot.x = PivotJson[0].get<_float>();
        Desc.vPivot.y = PivotJson[1].get<_float>();
        Desc.vPivot.z = PivotJson[2].get<_float>();
    }

    if (ParticleVBJson.contains("Speed") && ParticleVBJson["Speed"].is_array())
    {
        json SpeedJson = ParticleVBJson["Speed"];
        Desc.vSpeed.x = SpeedJson[0].get<_float>();
        Desc.vSpeed.y = SpeedJson[1].get<_float>();
    }
  
    if (ParticleVBJson.contains("LifeTime") && ParticleVBJson["LifeTime"].is_array())
    {
        json LifeTimeJson = ParticleVBJson["LifeTime"];
        Desc.vLifeTime.x = LifeTimeJson[0].get<_float>();
        Desc.vLifeTime.y = LifeTimeJson[1].get<_float>();
    }

    if (ParticleVBJson.contains("Loop"))
        Desc.IsLoop = ParticleVBJson["Loop"].get<_bool>();

	if (ParticleVBJson.contains("SpawnBox"))
		Desc.IsSpawnBox = ParticleVBJson["SpawnBox"].get<_bool>();

	if (ParticleVBJson.contains("SpawnRing"))
		Desc.IsSpawnRing = ParticleVBJson["SpawnRing"].get<_bool>();

	if (ParticleVBJson.contains("RingAngle"))
		Desc.IsRingAngle = ParticleVBJson["RingAngle"].get<_bool>();

	if (ParticleVBJson.contains("RingAngle_Min"))
		Desc.fRmin = ParticleVBJson["RingAngle_Min"].get<_float>();

	if (ParticleVBJson.contains("RingAngle_Max"))
		Desc.fRmax = ParticleVBJson["RingAngle_Max"].get<_float>();

	if (ParticleVBJson.contains("DegreeAngle") && ParticleVBJson["DegreeAngle"].is_array())
	{
		json DegreeJson = ParticleVBJson["DegreeAngle"];
		Desc.fDegreeAngle.x = DegreeJson[0].get<_float>();
		Desc.fDegreeAngle.y = DegreeJson[1].get<_float>();
	}

    if (ParticleVBJson.contains("Stretch"))
        Desc.IsStretch = ParticleVBJson["Stretch"].get<_bool>();

    if (ParticleVBJson.contains("Stretch_Weight"))
        Desc.fStretchWeight = ParticleVBJson["Stretch_Weight"].get<_float>();

    if (ParticleVBJson.contains("Stretch_Range") && ParticleVBJson["Stretch_Range"].is_array())
    {
        json StrerchRangJson = ParticleVBJson["Stretch_Range"];
        Desc.fStretchRange.x = StrerchRangJson[0].get<_float>();
        Desc.fStretchRange.y = StrerchRangJson[1].get<_float>();
    }

    if (ParticleVBJson.contains("Sprite"))
        Desc.IsSprite = ParticleVBJson["Sprite"].get<_bool>();

    if (ParticleVBJson.contains("Sprite_Weight"))
        Desc.fSpriteWeight = ParticleVBJson["Sprite_Weight"].get<_float>();

    if (ParticleVBJson.contains("Sprite_DefulatSpeed"))
        Desc.fDefualtSpeed = ParticleVBJson["Sprite_DefulatSpeed"].get<_float>();

    if (ParticleVBJson.contains("Delay"))
        Desc.IsDelay = ParticleVBJson["Delay"].get<_bool>();

    if (ParticleVBJson.contains("Delay_Time") && ParticleVBJson["Delay_Time"].is_array())
    {
        json DelayTimeJson = ParticleVBJson["Delay_Time"];
        Desc.fDelay.x = DelayTimeJson[0].get<_float>();
        Desc.fDelay.y = DelayTimeJson[1].get<_float>();
    }

    if (ParticleVBJson.contains("SpreadWeight"))
        Desc.fSpreadWeight = ParticleVBJson["SpreadWeight"].get<_float>();

    if (ParticleVBJson.contains("DropWeight"))
        Desc.fDropWeight = ParticleVBJson["DropWeight"].get<_float>();

    if (ParticleVBJson.contains("RotationWeight"))
        Desc.fRotationWeight = ParticleVBJson["RotationWeight"].get<_float>();

    if (ParticleVBJson.contains("Gravity"))
        Desc.fGravity = ParticleVBJson["Gravity"].get<_float>();

    m_tParticleVBDesc.emplace(ParticleTag, Desc);
}

void CLoad_Controller::Load_Particle_OB_FromJson(const _string& strFilePath, const _wstring& ParticleTag)
{
    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json ParticleJson;
    JsonStream >> ParticleJson;
    JsonStream.close();

    CParticle::PARTICLE_DESC Desc = {};

    if (ParticleJson.contains("MyTag"))
        Desc.strMyTag = StringToWString(ParticleJson["MyTag"].get<_string>());

    if (ParticleJson.contains("MyType"))
        Desc.eMyType = static_cast<EFFECT_TYPE>(ParticleJson["MyType"].get<double>());

    if (ParticleJson.contains("Root"))
        Desc.IsRootOn = ParticleJson["Root"].get<_bool>();

	if (ParticleJson.contains("Pivot"))
		Desc.IsPivot = ParticleJson["Pivot"].get<_bool>();

	if (ParticleJson.contains("Loop"))
		Desc.IsLoop = ParticleJson["Loop"].get<_bool>();

    if (ParticleJson.contains("TextureTag"))
        Desc.strTextureTag = StringToWString(ParticleJson["TextureTag"].get<_string>());

    if(ParticleJson.contains("VIBufferTag"))
        Desc.strVIBufferTag = StringToWString(ParticleJson["VIBufferTag"].get<_string>());

    if (ParticleJson.contains("ShaderPass"))
        Desc.fShaderPass = ParticleJson["ShaderPass"].get<_int>();

	if (ParticleJson.contains("MaskFlag"))
		Desc.iMaskFlag = ParticleJson["MaskFlag"].get<_int>();

    if (ParticleJson.contains("Size") && ParticleJson["Size"].is_array())
    {
        json SizeJson = ParticleJson["Size"];
        Desc.vSize.x = SizeJson[0].get<_float>();
        Desc.vSize.y = SizeJson[1].get<_float>();
        Desc.vSize.z = SizeJson[2].get<_float>();
    }

    if (ParticleJson.contains("Position") && ParticleJson["Position"].is_array())
    {
        json PosJson = ParticleJson["Position"];
        Desc.vPos.x = PosJson[0].get<_float>();
        Desc.vPos.y = PosJson[1].get<_float>();
        Desc.vPos.z = PosJson[2].get<_float>();
    }

    if (ParticleJson.contains("Color") && ParticleJson["Color"].is_array())
    {
        json ColorJson = ParticleJson["Color"];
        Desc.vColor.x = ColorJson[0].get<_float>();
        Desc.vColor.y = ColorJson[1].get<_float>();
        Desc.vColor.z = ColorJson[2].get<_float>();
        Desc.vColor.w = ColorJson[3].get<_float>();
    }

    if (ParticleJson.contains("LifeTime") && ParticleJson["LifeTime"].is_array())
    {
        json LifeTimeJson = ParticleJson["LifeTime"];
        Desc.vLifeTime.x = LifeTimeJson[0].get<_float>();
        Desc.vLifeTime.y = LifeTimeJson[1].get<_float>();
    }

    if (ParticleJson.contains("Sprite"))
        Desc.IsSprite = ParticleJson["Sprite"].get<_bool>();

    if (ParticleJson.contains("Row"))
        Desc.iRows = ParticleJson["Row"].get<_int>();

    if (ParticleJson.contains("Col"))
        Desc.iCols = ParticleJson["Col"].get<_int>();

    m_tParticleDesc.emplace(ParticleTag, Desc);
}

void CLoad_Controller::Load_FXMesh_VB_FromJson(const _string& strFilePath, const _wstring& MeshTag)
{
    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json FXMeshVBJson;
    JsonStream >> FXMeshVBJson;
    JsonStream.close();
    
    CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC Desc = {};

    if (FXMeshVBJson.contains("NumInstance"))
        Desc.iNumInstance = FXMeshVBJson["NumInstance"].get<_int>();

    if (FXMeshVBJson.contains("Center") && FXMeshVBJson["Center"].is_array())
    {
        json CenterJson = FXMeshVBJson["Center"];
        Desc.vCenter.x = CenterJson[0].get<_float>();
        Desc.vCenter.y = CenterJson[1].get<_float>();
        Desc.vCenter.z = CenterJson[2].get<_float>();
    }

    if (FXMeshVBJson.contains("Range") && FXMeshVBJson["Range"].is_array())
    {
        json RangeJson = FXMeshVBJson["Range"];
        Desc.vRange.x = RangeJson[0].get<_float>();
        Desc.vRange.y = RangeJson[1].get<_float>();
        Desc.vRange.z = RangeJson[2].get<_float>();
    }

    if (FXMeshVBJson.contains("Size") && FXMeshVBJson["Size"].is_array())
    {
        json SizeJson = FXMeshVBJson["Size"];
        Desc.vSize.x = SizeJson[0].get<_float>();
        Desc.vSize.y = SizeJson[1].get<_float>();
    }

    if (FXMeshVBJson.contains("DatPath"))
        strcpy_s(Desc.DatFilePath, MAX_PATH, FXMeshVBJson["DatPath"].get<_string>().c_str());

    if (FXMeshVBJson.contains("Pivot") && FXMeshVBJson["Pivot"].is_array())
    {
        json PivotJson = FXMeshVBJson["Pivot"];
        Desc.vPivot.x = PivotJson[0].get<_float>();
        Desc.vPivot.y = PivotJson[1].get<_float>();
        Desc.vPivot.z = PivotJson[2].get<_float>();
    }

    if (FXMeshVBJson.contains("Speed") && FXMeshVBJson["Speed"].is_array())
    {
        json SpeedJson = FXMeshVBJson["Speed"];
        Desc.vSpeed.x = SpeedJson[0].get<_float>();
        Desc.vSpeed.y = SpeedJson[1].get<_float>();
    }

    if (FXMeshVBJson.contains("LifeTime"))
        Desc.fLifeTime = FXMeshVBJson["LifeTime"].get<_float>();

    if (FXMeshVBJson.contains("Loop"))
        Desc.IsLoop = FXMeshVBJson["Loop"].get<_bool>();

    if (FXMeshVBJson.contains("SpawnBox"))
        Desc.IsSpawnBox = FXMeshVBJson["SpawnBox"].get<_bool>();

    if (FXMeshVBJson.contains("SpawnRing"))
        Desc.IsSpawnRing = FXMeshVBJson["SpawnRing"].get<_bool>();

    if (FXMeshVBJson.contains("RingAngle"))
        Desc.IsRingAngle = FXMeshVBJson["RingAngle"].get<_bool>();

    if (FXMeshVBJson.contains("RingAngle_Min"))
        Desc.fRmin = FXMeshVBJson["RingAngle_Min"].get<_float>();

    if (FXMeshVBJson.contains("RingAngle_max"))
        Desc.fRmax = FXMeshVBJson["RingAngle_max"].get<_float>();

    if (FXMeshVBJson.contains("DegreeAngle") && FXMeshVBJson["DegreeAngle"].is_array())
    {
        json DegreeJson = FXMeshVBJson["DegreeAngle"];
        Desc.fDegreeAngle.x = DegreeJson[0].get<_float>();
        Desc.fDegreeAngle.y = DegreeJson[1].get<_float>();
    }

    if (FXMeshVBJson.contains("InWard"))
        Desc.IsInWard = FXMeshVBJson["InWard"].get<_bool>();

    if (FXMeshVBJson.contains("OutWard"))
        Desc.IsOutWard = FXMeshVBJson["OutWard"].get<_bool>();

    if (FXMeshVBJson.contains("Pitch"))
        Desc.fPitch = FXMeshVBJson["Pitch"].get<_float>();

    if (FXMeshVBJson.contains("SpreadWeight"))
        Desc.fSpreadWeight = FXMeshVBJson["SpreadWeight"].get<_float>();

    if (FXMeshVBJson.contains("DropWeight"))
        Desc.fDropWeight = FXMeshVBJson["DropWeight"].get<_float>();

    if (FXMeshVBJson.contains("RotationWeight"))
        Desc.fRotationWeight = FXMeshVBJson["RotationWeight"].get<_float>();

    m_tMeshVBDesc.emplace(MeshTag, Desc);
}

void CLoad_Controller::Load_FXMesh_OB_FromJson(const _string& strFilePath, const _wstring& MeshTag)
{
    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json FXMeshJson;
    JsonStream >> FXMeshJson;
    JsonStream.close();

    CEffect_Mesh::EFFECTMESH_DESC Desc = { };

    if (FXMeshJson.contains("MyTag"))
        Desc.strMyTag = StringToWString(FXMeshJson["MyTag"].get<_string>());

    if (FXMeshJson.contains("MyType"))
        Desc.eMyType = static_cast<EFFECT_TYPE>(FXMeshJson["MyType"].get<double>());

    if (FXMeshJson.contains("Root"))
        Desc.IsRootOn = FXMeshJson["Root"].get<_bool>();

    if (FXMeshJson.contains("TextureTag"))
        Desc.strTextureTag = StringToWString(FXMeshJson["TextureTag"].get<_string>());

    if (FXMeshJson.contains("VIBufferTag"))
        Desc.strVIBufferTag = StringToWString(FXMeshJson["VIBufferTag"].get<_string>());

    if (FXMeshJson.contains("ShaderPass"))
        Desc.fShaderPass = FXMeshJson["ShaderPass"].get<_int>();

    if (FXMeshJson.contains("Size") && FXMeshJson["Size"].is_array())
    {
        json SizeJson = FXMeshJson["Size"];
        Desc.vSize.x = SizeJson[0].get<_float>();
        Desc.vSize.y = SizeJson[1].get<_float>();
        Desc.vSize.z = SizeJson[2].get<_float>();
    }

    if (FXMeshJson.contains("Position") && FXMeshJson["Position"].is_array())
    {
        json PosJson = FXMeshJson["Position"];
        Desc.vPos.x = PosJson[0].get<_float>();
        Desc.vPos.y = PosJson[1].get<_float>();
        Desc.vPos.z = PosJson[2].get<_float>();
    }

    if (FXMeshJson.contains("Color") && FXMeshJson["Color"].is_array())
    {
        json ColorJson = FXMeshJson["Color"];
        Desc.vColor.x = ColorJson[0].get<_float>();
        Desc.vColor.y = ColorJson[1].get<_float>();
        Desc.vColor.z = ColorJson[2].get<_float>();
    }

    if (FXMeshJson.contains("LifeTime") && FXMeshJson["LifeTime"].is_array())
    {
        json LifeTimeJson = FXMeshJson["LifeTime"];
        Desc.vLifeTime.x = LifeTimeJson[0].get<_float>();
        Desc.vLifeTime.y = LifeTimeJson[1].get<_float>();
    }

    m_tEffectMeshDesc.emplace(MeshTag,Desc);
}

void CLoad_Controller::Load_TrailMesh_FromJson(const _string& strFilePath, const _wstring& TrailMeshTag)
{
    ifstream JsonStream(strFilePath.c_str());

    if (!JsonStream.is_open())
        return;

    json TrailMeshJson;
    JsonStream >> TrailMeshJson;
    JsonStream.close();

    CTrail_Mesh::TRAILMESH_DESC Desc = {};

    if (TrailMeshJson.contains("MyTag"))
        Desc.strMyTag = StringToWString(TrailMeshJson["MyTag"].get<_string>());

    if (TrailMeshJson.contains("MyType"))
        Desc.eMyType = static_cast<EFFECT_TYPE>(TrailMeshJson["MyType"].get<double>());

    if (TrailMeshJson.contains("Root"))
        Desc.IsRootOn = TrailMeshJson["Root"].get<_bool>();

	if (TrailMeshJson.contains("Loop"))
		Desc.IsLoop = TrailMeshJson["Loop"].get<_bool>();

    if (TrailMeshJson.contains("TextureTag"))
        Desc.strTextureTag = StringToWString(TrailMeshJson["TextureTag"].get<_string>());

    if (TrailMeshJson.contains("ColorTextureTag"))
        Desc.strColorTextureTag = StringToWString(TrailMeshJson["ColorTextureTag"].get<_string>());

	if (TrailMeshJson.contains("DlssolveTextureTag"))
		Desc.strDissolveTextureTag = StringToWString(TrailMeshJson["DlssolveTextureTag"].get<_string>());

	if (TrailMeshJson.contains("DistortionTextureTag"))
		Desc.strDistortionTextureTag = StringToWString(TrailMeshJson["DistortionTextureTag"].get<_string>());

    if (TrailMeshJson.contains("VIBufferTag"))
        Desc.strVIBufferTag = StringToWString(TrailMeshJson["VIBufferTag"].get<_string>());

    if (TrailMeshJson.contains("ShaderPass"))
        Desc.iShaderPass = TrailMeshJson["ShaderPass"].get<_int>();

    if (TrailMeshJson.contains("SweepSpeed"))
        Desc.fSweep = TrailMeshJson["SweepSpeed"].get<_float>();

    if (TrailMeshJson.contains("SweepWitdh"))
        Desc.fSweepWitdh = TrailMeshJson["SweepWitdh"].get<_float>();

	if (TrailMeshJson.contains("SweepSoft"))
		Desc.fSoft = TrailMeshJson["SweepSoft"].get<_float>();

    if (TrailMeshJson.contains("DirFlag"))
        Desc.iDirFlag = TrailMeshJson["DirFlag"].get<_int>();

	if (TrailMeshJson.contains("MaskFloag"))		//오타있음
		Desc.iMaskFlag = TrailMeshJson["MaskFloag"].get<_int>();

	if (TrailMeshJson.contains("DissolveFlag"))
		Desc.IsDissolve = TrailMeshJson["DissolveFlag"].get<_bool>();

	if (TrailMeshJson.contains("DistortionFlag"))
		Desc.IsDistortion = TrailMeshJson["DistortionFlag"].get<_bool>();

	if (TrailMeshJson.contains("DistortionWeight"))
		Desc.fDistortionWeight = TrailMeshJson["DistortionWeight"].get<_float>();

	if (TrailMeshJson.contains("ColorSpeed"))
		Desc.fColorSpeed = TrailMeshJson["ColorSpeed"].get<_float>();

	if (TrailMeshJson.contains("MaskSpeed"))
		Desc.fMaskSpeed = TrailMeshJson["MaskSpeed"].get<_float>();

	if (TrailMeshJson.contains("Alpha"))
		Desc.fAlpha = TrailMeshJson["Alpha"].get<_float>();

	if (TrailMeshJson.contains("ColorGain"))
		Desc.fColorGain = TrailMeshJson["ColorGain"].get<_float>();

	if (TrailMeshJson.contains("ColorGamma"))
		Desc.fColorGamma = TrailMeshJson["ColorGamma"].get<_float>();


    if (TrailMeshJson.contains("Size") && TrailMeshJson["Size"].is_array())
    {
        json SizeJson = TrailMeshJson["Size"];
        Desc.vSize.x = SizeJson[0].get<_float>();
        Desc.vSize.y = SizeJson[1].get<_float>();
        Desc.vSize.z = SizeJson[2].get<_float>();
    }

    if (TrailMeshJson.contains("Position") && TrailMeshJson["Position"].is_array())
    {
        json PosJson = TrailMeshJson["Position"];
        Desc.vPos.x = PosJson[0].get<_float>();
        Desc.vPos.y = PosJson[1].get<_float>();
        Desc.vPos.z = PosJson[2].get<_float>();
    }

    if (TrailMeshJson.contains("LifeTime") && TrailMeshJson["LifeTime"].is_array())
    {
        json LifeTimeJson = TrailMeshJson["LifeTime"];
        Desc.vLifeTime.x = LifeTimeJson[0].get<_float>();
        Desc.vLifeTime.y = LifeTimeJson[1].get<_float>();
    }

    m_tTrailMeshDesc.emplace(TrailMeshTag, Desc);
}

void CLoad_Controller::Load_FXRect_FromJson(const _string& strFilePath, const _wstring& RectTag)
{
	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json RectJson;
	JsonStream >> RectJson;
	JsonStream.close();

	CEffect_Rect::FXRECT_DESC Desc = {};

	if (RectJson.contains("MyTag"))
		Desc.strMyTag = StringToWString(RectJson["MyTag"].get<_string>());

	if (RectJson.contains("MyType"))
		Desc.eMyType = static_cast<EFFECT_TYPE>(RectJson["MyType"].get<double>());

	if (RectJson.contains("Root"))
		Desc.IsRootOn = RectJson["Root"].get<_bool>();

	if (RectJson.contains("Loop"))
		Desc.IsLoop = RectJson["Loop"].get<_bool>();

	if (RectJson.contains("TextureTag"))
		Desc.strTextureTag = StringToWString(RectJson["TextureTag"].get<_string>());

	if (RectJson.contains("ShaderPass"))
		Desc.iShaderPass = RectJson["ShaderPass"].get<_int>();

	if (RectJson.contains("MaskFlag"))
		Desc.iMaskFlag = RectJson["MaskFlag"].get<_int>();

	if (RectJson.contains("ColorFlag"))
		Desc.iColorFlag = RectJson["ColorFlag"].get<_int>();

	if (RectJson.contains("Sprite"))
		Desc.IsSprite = RectJson["Sprite"].get<_bool>();

	if (RectJson.contains("Row"))
		Desc.iRows = RectJson["Row"].get<_int>();

	if (RectJson.contains("Col"))
		Desc.iCols = RectJson["Col"].get<_int>();

	if (RectJson.contains("SweepSpeed"))
		Desc.fSweepSpeed = RectJson["SweepSpeed"].get<_float>();

	if (RectJson.contains("SweepSoft"))
		Desc.fSoft = RectJson["SweepSoft"].get<_float>();

	if (RectJson.contains("SizeX"))
		Desc.fXSize = RectJson["SizeX"].get<_float>();

	if (RectJson.contains("SizeY"))
		Desc.fYSize = RectJson["SizeY"].get<_float>();

	if (RectJson.contains("Position") && RectJson["Position"].is_array())
	{
		json PosJson = RectJson["Position"];
		Desc.vPos.x = PosJson[0].get<_float>();
		Desc.vPos.y = PosJson[1].get<_float>();
		Desc.vPos.z = PosJson[2].get<_float>();
	}

	if (RectJson.contains("LifeTime") && RectJson["LifeTime"].is_array())
	{
		json LifeTimeJson = RectJson["LifeTime"];
		Desc.vLifeTime.x = LifeTimeJson[0].get<_float>();
		Desc.vLifeTime.y = LifeTimeJson[1].get<_float>();
	}

	if (RectJson.contains("Color") && RectJson["Color"].is_array())
	{
		json ColorJson = RectJson["Color"];
		Desc.vColor.x = ColorJson[0].get<_float>();
		Desc.vColor.y = ColorJson[1].get<_float>();
		Desc.vColor.z = ColorJson[2].get<_float>();
		Desc.vColor.w = ColorJson[3].get<_float>();
	}

	m_tRectDesc.emplace(RectTag, Desc);
}

void CLoad_Controller::Load_FXDecal_FromJson(const _string& strFilePath, const _wstring& DecalTag)
{
	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json DecalJson;
	JsonStream >> DecalJson;
	JsonStream.close();

	CEffect_Decal::DECAL_DESC Desc = {};

	if (DecalJson.contains("MyTag"))
		Desc.strMyTag = StringToWString(DecalJson["MyTag"].get<_string>());

	if (DecalJson.contains("MyType"))
		Desc.eMyType = static_cast<EFFECT_TYPE>(DecalJson["MyType"].get<double>());

	if (DecalJson.contains("DecalTag"))
		Desc.wstrDecalTag = StringToWString(DecalJson["DecalTag"].get<_string>());

	if (DecalJson.contains("LifeTime"))
		Desc.LifeTime = DecalJson["LifeTime"].get<_float>();

	if (DecalJson.contains("BlendTime"))
		Desc.fBlendTime = DecalJson["BlendTime"].get<_float>();

	if (DecalJson.contains("EmissiveIntensity"))
		Desc.fEmissiveIntensity = DecalJson["EmissiveIntensity"].get<_float>();

	if (DecalJson.contains("Color") && DecalJson["Color"].is_array())
	{
		json ColorJson = DecalJson["Color"];
		Desc.vColor.x = ColorJson[0].get<_float>();
		Desc.vColor.y = ColorJson[1].get<_float>();
		Desc.vColor.z = ColorJson[2].get<_float>();
		Desc.vColor.w = ColorJson[3].get<_float>();
	}

	m_tDecalDesc.emplace(DecalTag, Desc);
}

void CLoad_Controller::Load_FXRadial_FromJson(const _string& strFilePath, const _wstring& RadialTag)
{
	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json RadialJson;
	JsonStream >> RadialJson;
	JsonStream.close();

	CEffect_Radial::RADIAL_DESC Desc = {};

	if (RadialJson.contains("MyTag"))
		Desc.strMyTag = StringToWString(RadialJson["MyTag"].get<_string>());

	if (RadialJson.contains("MyType"))
		Desc.eMyType = static_cast<EFFECT_TYPE>(RadialJson["MyType"].get<double>());

	if (RadialJson.contains("PositionFlag"))
		Desc.PositionFlag = RadialJson["PositionFlag"].get<_bool>();

	if(RadialJson.contains("LifeTime"))
		Desc.fLifeTime = RadialJson["LifeTime"].get<_float>();

	if (RadialJson.contains("IntensityRange"))
		Desc.IntensityRange = RadialJson["IntensityRange"].get<_float>();

	if (RadialJson.contains("Center") && RadialJson["Center"].is_array())
	{
		json CenterJson = RadialJson["Center"];
		Desc.Center.x = CenterJson[0].get<_float>();
		Desc.Center.y = CenterJson[1].get<_float>();
	}

	if (RadialJson.contains("DistanceRange") && RadialJson["DistanceRange"].is_array())
	{
		json DistanceRangeJson = RadialJson["DistanceRange"];
		Desc.DistanceRange.x = DistanceRangeJson[0].get<_float>();
		Desc.DistanceRange.y = DistanceRangeJson[1].get<_float>();
	}

	m_tRadialDesc.emplace(RadialTag, Desc);
}

void CLoad_Controller::Load_FXVA_FromJson(const _string& strFilePath, const _wstring& VATag)
{
	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json VAJson;
	JsonStream >> VAJson;
	JsonStream.close();

	CTestVA::VA_DESC Desc = {};

	if (VAJson.contains("MyTag"))
		Desc.strMyTag = StringToWString(VAJson["MyTag"].get<_string>());

	if (VAJson.contains("MyType"))
		Desc.eMyType = static_cast<EFFECT_TYPE>(VAJson["MyType"].get<double>());

	if (VAJson.contains("MaskTextureTag"))
		Desc.strTextureTag = StringToWString(VAJson["MaskTextureTag"].get<_string>());

	if (VAJson.contains("ColorTextureTag"))
		Desc.strColorTextureTag = StringToWString(VAJson["ColorTextureTag"].get<_string>());

	if (VAJson.contains("MeshTag"))
		Desc.strMeshTag = StringToWString(VAJson["MeshTag"].get<_string>());

	if (VAJson.contains("AnimSpeed"))
		Desc.fAnimSpeed = VAJson["AnimSpeed"].get<_float>();

	if (VAJson.contains("MovementScale"))
		Desc.fMovementScale = VAJson["MovementScale"].get<_float>();

	if (VAJson.contains("ShaderPass"))
		Desc.iShaderPass = VAJson["ShaderPass"].get<_int>();

	m_tVADesc.emplace(VATag, Desc);
}

void CLoad_Controller::Load_FXLight_FromJson(const _string& strFilePath, const _wstring& LightTag)
{
	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json LightJson;
	JsonStream >> LightJson;
	JsonStream.close();

	CEffect_Light::LIGHT_DESC Desc = {};

	if (LightJson.contains("MyTag"))
		Desc.strMyTag = StringToWString(LightJson["MyTag"].get<_string>());

	if (LightJson.contains("MyType"))
		Desc.eMyType = static_cast<EFFECT_TYPE>(LightJson["MyType"].get<double>());

	if (LightJson.contains("LightTag"))
		Desc.wstrLightTag = StringToWString(LightJson["LightTag"].get<_string>());

	if (LightJson.contains("Speed"))
		Desc.fSpeed = LightJson["Speed"].get<_float>();

	if (LightJson.contains("Ambient"))
		Desc.fAmbient = LightJson["Ambient"].get<_float>();

	if (LightJson.contains("Color") && LightJson["Color"].is_array())
	{
		json Color = LightJson["Color"];
		Desc.vColor.x = Color[0].get<_float>();
		Desc.vColor.y = Color[1].get<_float>();
		Desc.vColor.z = Color[2].get<_float>();
		Desc.vColor.w = Color[3].get<_float>();
	}

	if (LightJson.contains("LifeTime") && LightJson["LifeTime"].is_array())
	{
		json LifeTime = LightJson["LifeTime"];
		Desc.vLifeTime.x = LifeTime[0].get<_float>();
		Desc.vLifeTime.y = LifeTime[1].get<_float>();
	}

	if (LightJson.contains("Range") && LightJson["Range"].is_array())
	{
		json Range = LightJson["Range"];
		Desc.vRange.x = Range[0].get<_float>();
		Desc.vRange.y = Range[1].get<_float>();
	}

	m_tLightDesc.emplace(LightTag, Desc);
}

void CLoad_Controller::Get_Prefab_Desc(CEffect_Prefab::PREFAB_DESC& PrefabDesc)
{
    PrefabDesc = m_tPrefabDesc;
}

void CLoad_Controller::Get_Particle_VB_Desc(const _wstring& ParticleTag, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC& ParticleVBDesc)
{
    auto iter = m_tParticleVBDesc.find(ParticleTag);

    if (iter != m_tParticleVBDesc.end())
        ParticleVBDesc = iter->second;
}

void CLoad_Controller::Get_Particle_OB_Desc(const _wstring& ParticleTag, CParticle::PARTICLE_DESC& ParticleDesc)
{
    auto iter = m_tParticleDesc.find(ParticleTag);

    if (iter != m_tParticleDesc.end())
        ParticleDesc = iter->second;
}

void CLoad_Controller::Get_FXMesh_VB_Desc(const _wstring& FXMeshTag, CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC& MeshVBDesc)
{
    auto iter = m_tMeshVBDesc.find(FXMeshTag);

    if (iter != m_tMeshVBDesc.end())
        MeshVBDesc = iter->second;
}

void CLoad_Controller::Get_FXMesh_OB_Desc(const _wstring& FXMeshTag, CEffect_Mesh::EFFECTMESH_DESC& MeshDesc)
{
    auto iter = m_tEffectMeshDesc.find(FXMeshTag);

    if (iter != m_tEffectMeshDesc.end())
        MeshDesc = iter->second;
}

void CLoad_Controller::Get_TrailMesh_Desc(const _wstring& TrailMeshTag, CTrail_Mesh::TRAILMESH_DESC& TrailMesh)
{
    auto iter = m_tTrailMeshDesc.find(TrailMeshTag);

    if (iter != m_tTrailMeshDesc.end())
        TrailMesh = iter->second;
}

void CLoad_Controller::Get_FXRect_Desc(const _wstring& RectTag, CEffect_Rect::FXRECT_DESC& RectDesc)
{
	auto iter = m_tRectDesc.find(RectTag);

	if (iter != m_tRectDesc.end())
		RectDesc = iter->second;
}

void CLoad_Controller::Get_FXDecal_Desc(const _wstring& DecalTag, CEffect_Decal::DECAL_DESC& DecalDesc)
{
	auto iter = m_tDecalDesc.find(DecalTag);

	if (iter != m_tDecalDesc.end())
		DecalDesc = iter->second;
}

void CLoad_Controller::Get_FXRadial_Desc(const _wstring& RadialTag, CEffect_Radial::RADIAL_DESC& RadialDesc)
{
	auto iter = m_tRadialDesc.find(RadialTag);

	if (iter != m_tRadialDesc.end())
		RadialDesc = iter->second;
}

void CLoad_Controller::Get_FXVA_Desc(const _wstring& VATag, CTestVA::VA_DESC& VADesc)
{
	auto iter = m_tVADesc.find(VATag);

	if (iter != m_tVADesc.end())
		VADesc = iter->second;
}

void CLoad_Controller::Get_FXLight_Desc(const _wstring& LightTag, CEffect_Light::LIGHT_DESC& LightDesc)
{
	auto iter = m_tLightDesc.find(LightTag);

	if (iter != m_tLightDesc.end())
		LightDesc = iter->second;
}

void CLoad_Controller::Reset_Load()
{
    m_tEffectMeshDesc.clear();
    m_tMeshVBDesc.clear();
    m_tParticleDesc.clear();
    m_tParticleVBDesc.clear();
    m_tTrailMeshDesc.clear();
	m_tRectDesc.clear();
	m_tDecalDesc.clear();
	m_tRadialDesc.clear();
	m_tVADesc.clear();
	m_tLightDesc.clear();

    CEffect_Prefab::PREFAB_DESC Desc = {};
    m_tPrefabDesc = Desc;
}

void CLoad_Controller::Add_CurrentLevel_Effect()
{
	//
}

CLoad_Controller* CLoad_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eCurrentLevel)
{
    CLoad_Controller* pInstance = new CLoad_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eCurrentLevel)))
    {
        MSG_BOX("Failed to Create : CLoad_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLoad_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
}