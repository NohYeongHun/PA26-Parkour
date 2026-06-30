#include "EditorPch.h"
#include "VA_Controller.h"

CVA_Controller::CVA_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CVA_Controller::Initialize()
{
    Load_AllMaskTextureFromFolder("../../Client/Bin/Resource/Effect/EffectVA/Mask");
    Load_AllMeshDatFromFolder("../../Client/Bin/Resource/Effect/EffectVA/Dat");
    Load_AllColorTextureFormFolder("../../Client/Bin/Resource/Effect/EffectVA/Color");

    return S_OK;
}

void CVA_Controller::Update()
{
    VA_Tab();
}

void CVA_Controller::Render()
{

}

void CVA_Controller::Load_AllMaskTextureFromFolder(const _string& strFolderPath)
{
    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            _string filePath = entry.path().string();
            _string fileName = entry.path().filename().string();
            _string extension = entry.path().extension().string();

            if (extension == ".png" || extension == ".Png")
            {
                MASK_TEXTURE Desc{};
                CTexture* pTexture = {};

                // 확장자 제외한 파일명
                _string strTextureTag = entry.path().stem().string();

                //파일명으로 텍스처 이름 지정
                strcpy_s(Desc.szName, sizeof(Desc.szName), strTextureTag.c_str());

                //파일명으로 텍스처 컴포넌트 이름 지정
                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_VATexture_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strTextureTag, MAX_PATH);

                //파일경로 wstring 변환
                _wstring wstrFilePath = StringToWString(filePath);
    
                //텍스처 컴포넌트 생성
                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
                    pTexture = CTexture::Create(m_pDevice, m_pContext, wstrFilePath.c_str(), 1));

                //생성한 텍스처 주소 등록, 미리보기 띄울려면 주소로 SRV가져와야해서 저장해줘야함.
                Desc.pTexture = pTexture;
                //Safe_AddRef(pTexture);

                m_Textures.push_back(Desc);
            }
        }
    }
}

//이름만 읽어서 리스트박스에 이름 띄우는 용도로만 사용하자. 
void CVA_Controller::Load_AllMeshDatFromFolder(const _string& strFolderPath)
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

                // 확장자 제외한 파일명
                _string strMeshTag = entry.path().stem().string();

                //파일명으로 매쉬 이름 지정
                strcpy_s(Desc.szName, sizeof(Desc.szName), strMeshTag.c_str());

                //파일명으로 매쉬버퍼 컴포넌트 이름 지정
                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_VAMesh_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strMeshTag, MAX_PATH);

                _string DefaultPath = "/Resource/Effect/VA/Dat/";

                DefaultPath += fileName;

                strcpy_s(Desc.szDatPath, sizeof(Desc.szDatPath), DefaultPath.c_str());

				_wstring wstrFilePath = StringToWString(filePath);

                //매쉬버퍼 컴포넌트 생성
                _float fSize = 0.01f;
                _fmatrix DefualtMatrix = XMMatrixScaling(fSize, fSize, fSize)/* * XMMatrixRotationX(90.f)*/;

                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strMeshTag,
                   CVAMesh::Create(m_pDevice, m_pContext, wstrFilePath.c_str(), DefualtMatrix, 1));

                m_MeshVBTag.push_back(Desc);
            }
        }
    }
}

void CVA_Controller::Load_AllColorTextureFormFolder(const _string& strFolderPath)
{
   
    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            _string filePath = entry.path().string();
            _string fileName = entry.path().filename().string();
            _string extension = entry.path().extension().string();

            if (extension == ".png" || extension == ".Png")
            {
                COLOR_TEXTURE Desc{};
                CTexture* pTexture = {};

                // 확장자 제외한 파일명
                _string strTextureTag = entry.path().stem().string();

                //파일명으로 텍스처 이름 지정
                strcpy_s(Desc.szName, sizeof(Desc.szName), strTextureTag.c_str());

                //파일명으로 텍스처 컴포넌트 이름 지정
                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_VATexture_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strTextureTag, MAX_PATH);

                //파일경로 wstring 변환
                _wstring wstrFilePath = StringToWString(filePath);

                //텍스처 컴포넌트 생성
                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
                    pTexture = CTexture::Create(m_pDevice, m_pContext, wstrFilePath.c_str(), 1));

                //생성한 텍스처 주소 등록, 미리보기 띄울려면 주소로 SRV가져와야해서 저장해줘야함.
                Desc.pTexture = pTexture;
                //Safe_AddRef(pTexture);

                m_ColorTextures.push_back(Desc);
            }
        }
    }
}

void CVA_Controller::VA_Tab()
{
    if (m_bSelectedMesh)
    {
        if (ImGui::Begin("VA Info"))
        {
            if (ImGui::CollapsingHeader("VA", ImGuiTreeNodeFlags_DefaultOpen))
            {
                /*      ImGui::Checkbox("Spread", &(m_pSelectedVADesc->bSpread));
                      ImGui::Checkbox("Drop", &(m_pSelectedVADesc->bDrop));*/
                ImGui::Text("ShaderPass");
                ImGui::PushItemWidth(100);
                ImGui::DragInt("##ShaderPass", &(m_pSelectedVADesc->iShaderPass), 1);
                ImGui::PopItemWidth();

				ImGui::Separator();

                ImGui::Text("AnimSpeed");
                ImGui::PushItemWidth(100);
                ImGui::InputFloat("##AnimSpeed", &(m_pSelectedVADesc->fAnimSpeed));
                ImGui::PopItemWidth();

				ImGui::Text("MovementScale");
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##Soft", &(m_pSelectedVADesc->fMovementScale));
				ImGui::PopItemWidth();
            }

            if (ImGui::CollapsingHeader("Base", ImGuiTreeNodeFlags_DefaultOpen))
            {
                vector<const _char*> szMeshTag = {};

                for (auto iter = m_MeshVBTag.begin(); iter != m_MeshVBTag.end(); ++iter)
                {
                    szMeshTag.push_back(iter->szName);
                }
                //매쉬 리스트박스 띄우기
                if (ImGui::ListBox("Mesh", &m_iSelectedMeshVBTag, szMeshTag.data(), int(szMeshTag.size()), int(szMeshTag.size() + 2)))
                {
                    m_pSelectedVADesc->strMeshTag = m_MeshVBTag[m_iSelectedMeshVBTag].strMeshTag;
                }

                //텍스처 설정
                if (ImGui::BeginCombo("Mask Texture", "")) {
                    for (size_t i = 0; i < m_Textures.size(); i++)
                    {
                        bool IsSelected = (m_iSelectedTexture == i);
                        if (ImGui::Selectable(m_Textures[i].szName, IsSelected))
                        {
                            m_iSelectedTexture = i;
                            m_pSelectedVADesc->strTextureTag = m_Textures[i].strTextureTag;
                        }
                        if (IsSelected)
                            ImGui::SetItemDefaultFocus();
                    }
      
                    ImGui::EndCombo();
                }

                ImGui::Separator();
                if (m_iSelectedTexture >= 0) {
                    ImGui::Image((ImTextureID)m_Textures[m_iSelectedTexture].pTexture->Get_SRV(0), ImVec2(256, 256));
                }

                if (ImGui::BeginCombo("Base Color", "")) {
                    for (size_t i = 0; i < m_ColorTextures.size(); i++)
                    {
                        bool IsSelected = (m_iSelectedColor == i);
                        if (ImGui::Selectable(m_ColorTextures[i].szName, IsSelected))
                        {
                            m_iSelectedColor = i;
                            m_pSelectedVADesc->strColorTextureTag = m_ColorTextures[i].strTextureTag;
                        }

                        if (IsSelected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }

                ImGui::Separator();
                if (m_iSelectedColor >= 0) {
                    ImGui::Image((ImTextureID)m_ColorTextures[m_iSelectedColor].pTexture->Get_SRV(0), ImVec2(256, 256));
                }
            }
            ImGui::End();
        }
    }

}

void CVA_Controller::VA_Base_Tab(CTestVA::VA_DESC& tVADesc, _bool& IsCreate)
{
    if (ImGui::Begin("Mesh Base"))
    {
        vector<const _char*> szMeshTag = {};

       for (auto iter = m_MeshVBTag.begin(); iter != m_MeshVBTag.end(); ++iter)
       {
            szMeshTag.push_back(iter->szName);
        }

        //매쉬 리스트박스 띄우기
        if (ImGui::ListBox("Trail Mesh", &m_iSelectedMeshVBTag, szMeshTag.data(), int(szMeshTag.size()), int(szMeshTag.size() + 2)))
        {
            m_bMeshVBTag = true;
        }

        //매쉬 기본 텍스처 설정
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

        ImGui::Separator();
        if (m_iSelectedTexture >= 0) {
            ImGui::Image((ImTextureID)m_Textures[m_iSelectedTexture].pTexture->Get_SRV(0), ImVec2(256, 256));
        }

        if (ImGui::BeginCombo("Color", "")) {
            for (size_t i = 0; i < m_ColorTextures.size(); i++)
            {
                bool IsSelected = (m_iSelectedColor == i);
                if (ImGui::Selectable(m_ColorTextures[i].szName, IsSelected))
                    m_iSelectedColor = i;

                if (IsSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::Separator();
        if (m_iSelectedColor >= 0) {
            ImGui::Image((ImTextureID)m_ColorTextures[m_iSelectedColor].pTexture->Get_SRV(0), ImVec2(256, 256));
        }

        if (m_bTagFlag && m_bMeshVBTag) //이펙트 컨트롤러가 설정해준 이름값이 있고, 선택한 매쉬버퍼가 있어야지만 생성할 수 있게.
        {
            if (ImGui::Button("Create"))
            {
                //프리팹에게 Desc 전달 -> 프리팹이 Desc로 클론 진행

                _tchar VATag[MAX_PATH] = {};
                CTestVA::VA_DESC VADesc{};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_VATag, strlen(m_VATag), VATag, MAX_PATH);

                //이펙트 이름 및 클론할 컴포넌트 이름들
                VADesc.strMyTag = VATag;
                VADesc.eMyType = EFFECT_TYPE::TRAIL;
                VADesc.strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;
                VADesc.strMeshTag = m_MeshVBTag[m_iSelectedMeshVBTag].strMeshTag;
                VADesc.strColorTextureTag = m_ColorTextures[m_iSelectedColor].strTextureTag;

                //이펙트가 가질 디폴트 설정값.
                VADesc.iShaderPass = 0;
				VADesc.fAnimSpeed = 1.f;
				VADesc.fMovementScale = 1.f;

                _char szDatPath[MAX_PATH] = {};

                strcpy_s(szDatPath, sizeof(szDatPath), "../../Client/Bin");
                strcat_s(szDatPath, sizeof(szDatPath), m_MeshVBTag[m_iSelectedMeshVBTag].szDatPath);

                if (m_IsRoot)
                {
                    VADesc.IsRootOn = true;
                    m_IsRoot = false;
                }

                //매쉬이펙트 와 매쉬VB태그를 맞춰야할지는 고민해보자.
                m_tVADesc.emplace(VATag, VADesc);

                tVADesc = VADesc;

                //생성됐음을 이펙트 컨트롤러에게 전달
                IsCreate = true;

                //초기화
                m_VATag[0] = _T('\0');
                m_bTagFlag = false;
            }
        }
       
        ImGui::End();
    }

}

void CVA_Controller::Set_VATag(const _char* szMeshTag)
{
    strcat_s(m_VATag, szMeshTag);

    m_bTagFlag = true;
}

void CVA_Controller::UpdateSelected_VAFormTag(_wstring FXMeshTag)
{
    auto iterEffectMeshDesc = m_tVADesc.find(FXMeshTag);
    
    if (iterEffectMeshDesc == m_tVADesc.end())
        m_pSelectedVADesc = nullptr;
    else
        m_pSelectedVADesc = &iterEffectMeshDesc->second;

    if (m_pSelectedVADesc != nullptr)
        m_bSelectedMesh = true;

}

CTestVA::VA_DESC* CVA_Controller::Get_VADesc(_wstring& EffectMeshTag)
{
    auto iter = m_tVADesc.find(EffectMeshTag);

    if (iter == m_tVADesc.end())
        return nullptr;

    return &iter->second;
}

void CVA_Controller::Set_VADesc(_wstring& VATag, CTestVA::VA_DESC& TrailDesc)
{
    CTestVA::VA_DESC Desc = {};
    Desc = TrailDesc;

    m_tVADesc.emplace(VATag, Desc);
}


void CVA_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterEffectMeshDesc = m_tVADesc.find(DescTag);

    if (iterEffectMeshDesc != m_tVADesc.end())
    {
        m_tVADesc.erase(iterEffectMeshDesc);
    }

    //초기화
    m_bSelectedMesh = false;
    m_pSelectedVADesc = nullptr;
}

CVA_Controller* CVA_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CVA_Controller* pInstance = new CVA_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CVA_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CVA_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    m_Textures.clear();
}