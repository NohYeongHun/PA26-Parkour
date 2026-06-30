#include "EditorPch.h"
#include "Mesh_Controller.h"

CMesh_Controller::CMesh_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CMesh_Controller::Initialize()
{
    Load_AllTextureFromFolder("../../Client/Bin/Resource/Effect/EffectMesh/Texture");
    Load_AllMeshDatFromFolder("../../Client/Bin/Resource/Effect/EffectMesh/Dat");

    return S_OK;
}

void CMesh_Controller::Update()
{
    EffectMesh_Tab();
}

void CMesh_Controller::Render()
{

}

void CMesh_Controller::Load_AllTextureFromFolder(const _string& strFolderPath)
{
    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            _string filePath = entry.path().string();
            _string fileName = entry.path().filename().string();
            _string extension = entry.path().extension().string();

            if (extension == ".png")
            {
                MESH_TEXTURE Desc{};
                CTexture* pTexture = {};

                // Ȯ���� ������ ���ϸ�
                _string strTextureTag = entry.path().stem().string();

                //���ϸ����� �ؽ�ó �̸� ����
                strcpy_s(Desc.szName, sizeof(Desc.szName), strTextureTag.c_str());

                //���ϸ����� �ؽ�ó ������Ʈ �̸� ����
                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strTextureTag, MAX_PATH);

                //���ϰ�� wstring ��ȯ
                _wstring wstrFilePath = StringToWString(filePath);
    
                //�ؽ�ó ������Ʈ ����
                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
                    pTexture = CTexture::Create(m_pDevice, m_pContext, wstrFilePath.c_str(), 1));

                //������ �ؽ�ó �ּ� ���, �̸����� ������ �ּҷ� SRV�����;��ؼ� �����������.
                Desc.pTexture = pTexture;
                //Safe_AddRef(pTexture);

                m_Textures.push_back(Desc);
            }
        }
    }
}

//�̸��� �о ����Ʈ�ڽ��� �̸� ���� �뵵�θ� �������. 
void CMesh_Controller::Load_AllMeshDatFromFolder(const _string& strFolderPath)
{
    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            _string filePath = entry.path().string();
            _string fileName = entry.path().filename().string();
            _string extension = entry.path().extension().string();

            if (extension == ".Dat" || extension == ".dat")
            {
                MESH_TAG Desc = {};
                CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC FXMeshDesc = {};

                // Ȯ���� ������ ���ϸ�
                _string strMeshTag = entry.path().stem().string();

                //���ϸ����� �Ž� �̸� ����
                strcpy_s(Desc.szName, sizeof(Desc.szName), strMeshTag.c_str());

                //���ϸ����� �Ž����� ������Ʈ �̸� ����
                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_EffectMesh_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strMeshTag, MAX_PATH);

                _string DefaultPath = "/Resource/Effect/EffectMesh/Dat/";

                DefaultPath += fileName;

                strcpy_s(Desc.szDatPath, sizeof(Desc.szDatPath), DefaultPath.c_str());

                m_MeshVBTag.push_back(Desc);
            }
        }
    }
}

void CMesh_Controller::EffectMesh_Tab()
{
    if (m_bSelectedMesh)
    {
        if (ImGui::Begin("FXMesh Info"))
        {

            //��ƼŬ ������ VIBuffer
            if (ImGui::CollapsingHeader("VIBuffer", ImGuiTreeNodeFlags_DefaultOpen))
            {
                /////////////////////////////////////// üũ�ڽ�
                ImGui::Checkbox("Loop", &(m_pSelectedVBFXDesc->IsLoop));

                if (ImGui::Checkbox("SpawnRing", &(m_pSelectedVBFXDesc->IsSpawnRing)))
                {
                    //���õ����� �ٸ� ��� ������
                    m_pSelectedVBFXDesc->IsSpawnBox = false;
                }

                ImGui::Checkbox("RingAngle", &(m_pSelectedVBFXDesc->IsRingAngle));

                if (ImGui::Checkbox("SpawnBox", &(m_pSelectedVBFXDesc->IsSpawnBox)))
                {
                    m_pSelectedVBFXDesc->IsSpawnRing = false;
                }
                ImGui::Separator();
                ///////////////////////////////////////

                ImGui::Text("NumInstance");
                ImGui::PushItemWidth(100);
                ImGui::DragInt("##NumInstance", (int*)&(m_pSelectedVBFXDesc->iNumInstance));
                ImGui::PopItemWidth();

                ImGui::PushItemWidth(200);

                /////////////////////////////////////// ����ġ ����
                ImGui::Separator();
                ImGui::Text("SpreadWeight");
                ImGui::SameLine();
                ImGui::DragFloat("##SpreadW", &(m_pSelectedVBFXDesc->fSpreadWeight), 0.1f, 0.f, 1.f);

                ImGui::Text("DropWeight");
                ImGui::SameLine();
                ImGui::DragFloat("##DorpW", &(m_pSelectedVBFXDesc->fDropWeight), 0.1f, 0.f, 1.f);

                ImGui::Text("RotationWeight");
                ImGui::SameLine();
                ImGui::DragFloat("##RotationW", &(m_pSelectedVBFXDesc->fRotationWeight), 0.1f, 0.f, 1.f);

                ImGui::PopItemWidth();
                ImGui::Separator();
                ///////////////////////////////////////
         
                if (m_pSelectedVBFXDesc->IsSpawnRing)
                {
                    /////////////////////////////////////// �� ������ ������
                    ImGui::Text("RMin/RMax");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##RMin", &(m_pSelectedVBFXDesc->fRmin));
                    ImGui::SameLine();
                    ImGui::InputFloat("##RMax", &(m_pSelectedVBFXDesc->fRmax));
                    ImGui::PopItemWidth();

                    ImGui::Text("Degree Start/End");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##DegreeX", &(m_pSelectedVBFXDesc->fDegreeAngle.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##DegreeY", &(m_pSelectedVBFXDesc->fDegreeAngle.y));
                    ImGui::PopItemWidth();
                    ImGui::Separator();
                    ///////////////////////////////////////
                }

                ImGui::Checkbox("InWard", &(m_pSelectedVBFXDesc->IsInWard));
                /////////////////////////////////////// ���� ����

                if (m_pSelectedVBFXDesc->IsInWard)
                {
                    ImGui::Text("Pitch");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##Pitch", &(m_pSelectedVBFXDesc->fPitch));
                    ImGui::PopItemWidth();
                    ImGui::Separator();
                }
                ///////////////////////////////////////
        
                ImGui::Text("Center");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##CenterX", &(m_pSelectedVBFXDesc->vCenter.x));
                ImGui::SameLine();
                ImGui::InputFloat("##CenterY", &(m_pSelectedVBFXDesc->vCenter.y));
                ImGui::SameLine();
                ImGui::InputFloat("##CenterZ", &(m_pSelectedVBFXDesc->vCenter.z));
                ImGui::PopItemWidth();

                ImGui::Text("Size");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##SizeX", &(m_pSelectedVBFXDesc->vSize.x));
                ImGui::SameLine();
                ImGui::InputFloat("##SizeY", &(m_pSelectedVBFXDesc->vSize.y));
                ImGui::PopItemWidth();

                ImGui::Text("Range");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##RangeX", &(m_pSelectedVBFXDesc->vRange.x));
                ImGui::SameLine();
                ImGui::InputFloat("##RangeY", &(m_pSelectedVBFXDesc->vRange.y));
                ImGui::SameLine();
                ImGui::InputFloat("##RangeZ", &(m_pSelectedVBFXDesc->vRange.z));
                ImGui::PopItemWidth();

                ImGui::Text("Pivot");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##PivotX", &(m_pSelectedVBFXDesc->vPivot.x));
                ImGui::SameLine();
                ImGui::InputFloat("##PivotY", &(m_pSelectedVBFXDesc->vPivot.y));
                ImGui::SameLine();
                ImGui::InputFloat("##PivotZ", &(m_pSelectedVBFXDesc->vPivot.z));
                ImGui::PopItemWidth();

                ImGui::Text("Speed");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##SppedX", &(m_pSelectedVBFXDesc->vSpeed.x));
                ImGui::SameLine();
                ImGui::InputFloat("##SppedY", &(m_pSelectedVBFXDesc->vSpeed.y));
                ImGui::PopItemWidth();

                ImGui::Text("LifeTime");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##LifeTimeX", &(m_pSelectedVBFXDesc->fLifeTime));
        /*        ImGui::SameLine();
                ImGui::InputFloat("##LifeTimeY", &(m_pSelectedVBFXDesc->vLifeTime.y));*/
                ImGui::PopItemWidth();
            }

            if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
            {
                /*      ImGui::Checkbox("Spread", &(m_pSelectedEffectMeshDesc->bSpread));
                      ImGui::Checkbox("Drop", &(m_pSelectedEffectMeshDesc->bDrop));*/
                ImGui::Checkbox("Root", &(m_pSelectedEffectMeshDesc->IsRootOn));

                ImGui::Text("Size");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##FXMeshSizeX", &(m_pSelectedEffectMeshDesc->vSize.x));
                ImGui::SameLine();
                ImGui::InputFloat("##FXMeshSizeY", &(m_pSelectedEffectMeshDesc->vSize.y));
                ImGui::SameLine();
                ImGui::InputFloat("##FXMeshSizeZ", &(m_pSelectedEffectMeshDesc->vSize.z));
                ImGui::PopItemWidth();

                ImGui::Text("Position");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##FXMeshPosX", &(m_pSelectedEffectMeshDesc->vPos.x));
                ImGui::SameLine();
                ImGui::InputFloat("##FXMeshPosY", &(m_pSelectedEffectMeshDesc->vPos.y));
                ImGui::SameLine();
                ImGui::InputFloat("#FX#MeshPosZ", &(m_pSelectedEffectMeshDesc->vPos.z));
                ImGui::PopItemWidth();

                ImGui::Text("Color");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##FXMeshColorX", &(m_pSelectedEffectMeshDesc->vColor.x));
                ImGui::SameLine();
                ImGui::InputFloat("##FXMeshColorY", &(m_pSelectedEffectMeshDesc->vColor.y));
                ImGui::SameLine();
                ImGui::InputFloat("##FXMeshColorZ", &(m_pSelectedEffectMeshDesc->vColor.z));
                ImGui::PopItemWidth();

                ImGui::Text("LifeTime");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##FXMeshLifeTimeX", &(m_pSelectedEffectMeshDesc->vLifeTime.x));
                ImGui::SameLine();
                ImGui::InputFloat("##FXMeshLifeTimeY", &(m_pSelectedEffectMeshDesc->vLifeTime.y));
                ImGui::PopItemWidth();
            }

            if (ImGui::CollapsingHeader("Base", ImGuiTreeNodeFlags_DefaultOpen))
            {
                vector<const _char*> szMeshTag = {};

                for (auto iter = m_MeshVBTag.begin(); iter != m_MeshVBTag.end(); ++iter)
                {
                    szMeshTag.push_back(iter->szName);
                }

                //�Ž� ����Ʈ�ڽ� ����
                if (ImGui::ListBox("Effect Mesh", &m_iSelectedMeshVBTag, szMeshTag.data(), int(szMeshTag.size()), int(szMeshTag.size() + 2)))
                {
                    m_pSelectedEffectMeshDesc->strVIBufferTag = m_MeshVBTag[m_iSelectedMeshVBTag].strMeshTag;
                }

                //�Ž� �⺻ ���� �ؽ�ó ����
                if (ImGui::BeginCombo("Texture", "")) {
                    for (size_t i = 0; i < m_Textures.size(); i++)
                    {
                        bool IsSelected = (m_iSelectedTexture == i);
                        if (ImGui::Selectable(m_Textures[i].szName, IsSelected))
                        {
                            m_iSelectedTexture = i;
                            m_pSelectedEffectMeshDesc->strTextureTag = m_Textures[i].strTextureTag;
                        }
                        if (IsSelected)
                            ImGui::SetItemDefaultFocus();

                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::End();
        }
    }

}

void CMesh_Controller::EffectMesh_Base_Tab(CEffect_Mesh::EFFECTMESH_DESC& tEffectMeshDesc, _bool& IsCreate)
{
    //�������� Ŭ������ �ڽ��� ������ ������ �Ѱ������.
    //�ϴ� �⺻���̽��� ������ �� �ְ� ������.
    //����Ʈ�Ž� (������Ʈ)�� �������� ������ �������� ���� �� ����. ex) �Ž��� ������ ����, �ؽ�ó ���� (������), �н�����, ������Ÿ�� ���..

    //����Ʈ�ڽ��� � �Ž� ������Ʈ�� ��������.
    //�ؽ�ó �̸������ � ��ǻ�� �ؽ�ó(���� �����ϵ�?) ���������� �ϴ� �����ؼ� �⺻���̽��� ���� �� �ְ�?

    if (ImGui::Begin("Mesh Base"))
    {
        vector<const _char*> szMeshTag = {};

       for (auto iter = m_MeshVBTag.begin(); iter != m_MeshVBTag.end(); ++iter)
       {
            szMeshTag.push_back(iter->szName);
        }

        //�Ž� ����Ʈ�ڽ� ����
        if (ImGui::ListBox("Effect Mesh", &m_iSelectedMeshVBTag, szMeshTag.data(), int(szMeshTag.size()), int(szMeshTag.size() + 2)))
        {
            //���õ� �Ž��±� �ӽ�����? 
            m_bMeshVBTag = true;
        }

        //�Ž� �⺻ ���� �ؽ�ó ����
        if (ImGui::BeginCombo("Texture", "")) {
            for (size_t i = 0; i < m_Textures.size(); i++)
            {
                bool IsSelected = (m_iSelectedTexture == i);
                if (ImGui::Selectable(m_Textures[i].szName, IsSelected))
                    m_iSelectedTexture = i;

                if (IsSelected)
                    ImGui::SetItemDefaultFocus();

            }

            ImGui::EndCombo();
        }

        //Root ����
        if(ImGui::Checkbox("Root", &m_IsRoot))

        ImGui::Separator();
        if (m_iSelectedTexture >= 0) {
            ImGui::Image((ImTextureID)m_Textures[m_iSelectedTexture].pTexture->Get_SRV(0), ImVec2(256, 256));
        }

        if (m_bTagFlag && m_bMeshVBTag) //����Ʈ ��Ʈ�ѷ��� �������� �̸����� �ְ�, ������ �Ž����۰� �־������ ������ �� �ְ�.
        {
            if (ImGui::Button("Create"))
            {
                //�����տ��� Desc ���� -> �������� Desc�� Ŭ�� ����

                _tchar EffectMeshTag[MAX_PATH] = {};
                CEffect_Mesh::EFFECTMESH_DESC EffectMeshDesc{};
                CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC VBFXMhesDesc{};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_TrailMeshTag, strlen(m_TrailMeshTag), EffectMeshTag, MAX_PATH);

                //EffectMeshDesc.strMyTag = EffectMeshTag;

                //����Ʈ �Ž� �̸� �� Ŭ���� ������Ʈ �̸���
                EffectMeshDesc.strMyTag = EffectMeshTag;
                EffectMeshDesc.eMyType = EFFECT_TYPE::MESH;
                EffectMeshDesc.strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;
                EffectMeshDesc.strVIBufferTag = m_MeshVBTag[m_iSelectedMeshVBTag].strMeshTag;

                //����Ʈ�Ž�(������Ʈ)�� ���� ����Ʈ ������.
                EffectMeshDesc.vLifeTime.y = 10.f;
                EffectMeshDesc.vPos = _float3(0.f, 0.f, 0.f);
                EffectMeshDesc.vSize = _float3(0.5f, 0.5f, 0.5f);
                EffectMeshDesc.fShaderPass = 0;

                //�ν��Ͻ̸Ž� ����Ʈ ������. ���⼭ �̸� ���� ������ �������.
                _fmatrix DefualtMatrix = XMMatrixIdentity();
                VBFXMhesDesc.vSize = _float2(1.f, 1.f);
                VBFXMhesDesc.iNumInstance = 1;
                
                _char szDatPath[MAX_PATH] = {};

                strcpy_s(szDatPath, sizeof(szDatPath), "../../Client/Bin");
                strcat_s(szDatPath, sizeof(szDatPath), m_MeshVBTag[m_iSelectedMeshVBTag].szDatPath);

                //���� ����
                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), m_MeshVBTag[m_iSelectedMeshVBTag].strMeshTag,
                CVIBuffer_FXMesh_Instance::Create(m_pDevice, m_pContext, szDatPath, DefualtMatrix, &VBFXMhesDesc));
                
                //������ ���� Dat ��� VB�� ����������Ұ� ����.
                strcpy_s(VBFXMhesDesc.DatFilePath, sizeof(VBFXMhesDesc.DatFilePath), m_MeshVBTag[m_iSelectedMeshVBTag].szDatPath);

                ////Desc�� VBMesh �̸� ����?
                //_tchar strFXMehsTag[MAX_PATH] = {};
                //MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_MeshVBTag[m_iSelectedMeshVBTag].szName, strlen(m_MeshVBTag[m_iSelectedMeshVBTag].szName), strFXMehsTag, MAX_PATH);
               
                m_tVBMeshDesc.emplace(EffectMeshTag, VBFXMhesDesc);

                if (m_IsRoot)
                {
                    EffectMeshDesc.IsRootOn = true;
                    m_IsRoot = false;
                }

  
                m_tEffectMeshDesc.emplace(EffectMeshTag, EffectMeshDesc);

                tEffectMeshDesc = EffectMeshDesc;

                IsCreate = true;

                m_TrailMeshTag[0] = _T('\0');
     

                m_bTagFlag = false;
            }
        }
       
        ImGui::End();
    }

}

void CMesh_Controller::Set_EffectMeshTag(const _char* szEffectMeshTag)
{
    strcat_s(m_TrailMeshTag, szEffectMeshTag);

    m_bTagFlag = true;
}

void CMesh_Controller::UpdateSelected_FXMeshFormTag(_wstring FXMeshTag)
{
    auto iterEffectMeshDesc = m_tEffectMeshDesc.find(FXMeshTag);
    
    if (iterEffectMeshDesc == m_tEffectMeshDesc.end())
        m_pSelectedEffectMeshDesc = nullptr;
    else
        m_pSelectedEffectMeshDesc = &iterEffectMeshDesc->second;

    auto iterVBFXDesc = m_tVBMeshDesc.find(FXMeshTag);
    
    if (iterVBFXDesc == m_tVBMeshDesc.end())
        m_pSelectedVBFXDesc = nullptr;
    else
        m_pSelectedVBFXDesc = &iterVBFXDesc->second;

    if (m_pSelectedEffectMeshDesc != nullptr)
        m_bSelectedMesh = true;

}

CEffect_Mesh::EFFECTMESH_DESC* CMesh_Controller::Get_EffectMeshDesc(_wstring& EffectMeshTag)
{
    auto iter = m_tEffectMeshDesc.find(EffectMeshTag);

    if (iter == m_tEffectMeshDesc.end())
        return nullptr;

    return &iter->second;
}

CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC* CMesh_Controller::Get_VBMeshDesc(_wstring& VBMesTag)
{
    auto iter = m_tVBMeshDesc.find(VBMesTag);

    if (iter == m_tVBMeshDesc.end())
        return nullptr;

    return &iter->second;
}

void CMesh_Controller::Set_EffectMeshDesc(_wstring& MeshTag, CEffect_Mesh::EFFECTMESH_DESC& MeshDesc)
{
    CEffect_Mesh::EFFECTMESH_DESC Desc = {};
    Desc = MeshDesc;

    m_tEffectMeshDesc.emplace(MeshTag, Desc);
}

void CMesh_Controller::Set_MeshVBDesc(_wstring& MeshTag, CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC& MeshVBDesc)
{
    CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC Desc = {};
    Desc = MeshVBDesc;

    m_tVBMeshDesc.emplace(MeshTag, MeshVBDesc); 
}

void CMesh_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterEffectMeshDesc = m_tEffectMeshDesc.find(DescTag);

    if (iterEffectMeshDesc != m_tEffectMeshDesc.end())
    {
        //Ȥ�� ���� �̸����� �ٽ� ��������°� ����ؼ� ��������Ұ� ����.
        m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), iterEffectMeshDesc->second.strVIBufferTag);

        m_tEffectMeshDesc.erase(iterEffectMeshDesc);
    }

    auto iterVBFXDesc = m_tVBMeshDesc.find(DescTag);

    if (iterVBFXDesc != m_tVBMeshDesc.end())
    {
        m_tVBMeshDesc.erase(iterVBFXDesc);
    }

    //�ʱ�ȭ
    m_bSelectedMesh = false;
    m_pSelectedEffectMeshDesc = nullptr;
    m_pSelectedVBFXDesc = nullptr;
}

CMesh_Controller* CMesh_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CMesh_Controller* pInstance = new CMesh_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CMesh_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CMesh_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    m_Textures.clear();
}