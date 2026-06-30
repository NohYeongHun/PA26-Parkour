#include "EditorPch.h"
#include "Rect_Controller.h"

CRect_Controller::CRect_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CRect_Controller::Initialize()
{

    Load_AllTextureFromFolder("../../Client/Bin/Resource/Effect/Rect/Texture");

    return S_OK;
}

void CRect_Controller::Update()
{
    Rect_Tab();
}

void CRect_Controller::Render()
{

}

void CRect_Controller::Load_AllTextureFromFolder(const _string& strFolderPath)
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
                RECT_TEXTURE Desc{};
                CTexture* pTexture = {};

                _string strTextureTag = entry.path().stem().string();

                strcpy_s(Desc.szName, sizeof(Desc.szName), strTextureTag.c_str());

                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strTextureTag, MAX_PATH);

                _wstring wstrFilePath = StringToWString(filePath);
   
                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strTextureTag,
                    pTexture = CTexture::Create(m_pDevice, m_pContext, wstrFilePath.c_str(), 1));

                Desc.pTexture = pTexture;
                //Safe_AddRef(pTexture);

                m_Textures.push_back(Desc);
            }
        }
    }
}

void CRect_Controller::Rect_Tab()
{
    if (m_bSelectedRect)
    {
        if (ImGui::Begin("Rect Info"))
        {
           
                if (ImGui::CollapsingHeader("Rect", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Checkbox("Root", &(m_pSelectedRectDesc->IsRootOn));

					ImGui::Checkbox("Loop", &(m_pSelectedRectDesc->IsLoop));

                    ImGui::Text("ShaderPass");
                    ImGui::PushItemWidth(100);
                    ImGui::DragInt("##ShaderPass", &(m_pSelectedRectDesc->iShaderPass), 1.f, 0, 6);
                    ImGui::PopItemWidth();

					ImGui::Separator();

					ImGui::Checkbox("Sprite", &(m_pSelectedRectDesc->IsSprite));

					if (m_pSelectedRectDesc->IsSprite)
					{
						ImGui::Text("Row / Col");
						ImGui::PushItemWidth(60);
						ImGui::InputInt("##Row", &(m_pSelectedRectDesc->iRows));
						ImGui::SameLine();
						ImGui::InputInt("##Col", &(m_pSelectedRectDesc->iCols));
						ImGui::PopItemWidth();
					}

					ImGui::Separator();

					ImGui::Text("Mask");
					ImGui::PushItemWidth(100);
					if (ImGui::Button("R Cut"))
						m_pSelectedRectDesc->iMaskFlag = 0;
					ImGui::SameLine();
					if (ImGui::Button("A Cut"))
						m_pSelectedRectDesc->iMaskFlag = 1;
					ImGui::PopItemWidth();

					ImGui::Text("SweepSpeed");
					ImGui::PushItemWidth(100);
					ImGui::InputFloat("##SweepSpeed", &(m_pSelectedRectDesc->fSweepSpeed));
					ImGui::PopItemWidth();

					ImGui::Text("Soft");
					ImGui::PushItemWidth(100);
					ImGui::InputFloat("##Soft", &(m_pSelectedRectDesc->fSoft));
					ImGui::PopItemWidth();

                    ImGui::Text("Size");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##RectSizeX", &(m_pSelectedRectDesc->fXSize));
                    ImGui::SameLine();
                    ImGui::InputFloat("##RectSizeY", &(m_pSelectedRectDesc->fYSize));
                    ImGui::PopItemWidth();

                    ImGui::Text("Position");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##RectPosX", &(m_pSelectedRectDesc->vPos.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##RectPosY", &(m_pSelectedRectDesc->vPos.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##RectPosZ", &(m_pSelectedRectDesc->vPos.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("LifeTime");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##RectLifeTimeX", &(m_pSelectedRectDesc->vLifeTime.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##RectLifeTimeY", &(m_pSelectedRectDesc->vLifeTime.y));
                    ImGui::PopItemWidth();

					ImGui::PushItemWidth(100);
					if (ImGui::Button("ColorFlag"))
					{
						if (m_pSelectedRectDesc->iColorFlag == 0)
							m_pSelectedRectDesc->iColorFlag = 1;
						else
							m_pSelectedRectDesc->iColorFlag = 0;
					}

                    if (ImGui::ColorEdit4("Color", m_fColor, 
						ImGuiColorEditFlags_NoOptions          // 설정 메뉴 비활성화 (HSV 등 변환 방지)
						| ImGuiColorEditFlags_NoInputs         // 텍스트 입력 비활성 (정확히 선택한 색 유지)
						| ImGuiColorEditFlags_DisplayRGB       // 항상 RGB로 표시
						| ImGuiColorEditFlags_InputRGB         // RGB 입력값으로 유지
						| ImGuiColorEditFlags_AlphaBar         // 알파 바 표시
						| ImGuiColorEditFlags_AlphaPreview))   // 알파 미리보기
                    {
                        m_pSelectedRectDesc->vColor = _float4(m_fColor[0], m_fColor[1], m_fColor[2], m_fColor[3]);
                    }

                    ImGui::Separator();
                  
                }

                if (ImGui::CollapsingHeader("Base", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    if (ImGui::BeginCombo("Texture", "")) {
                        for (size_t i = 0; i < m_Textures.size(); i++)
                        {
                            bool IsSelected = (m_iSelectedTexture == i);
                            if (ImGui::Selectable(m_Textures[i].szName, IsSelected))
                            {
                                m_iSelectedTexture = i;
                                m_pSelectedRectDesc->strTextureTag = m_Textures[i].strTextureTag;
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
                }

            ImGui::End();
        }
    }

}

void CRect_Controller::Rect_Base_Tab(CEffect_Rect::FXRECT_DESC& tRectDesc, _bool& IsCreate)
{
    if (ImGui::Begin("Rect Base"))
    {
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

        if (m_bTagFlag)
        {
            if (ImGui::Button("Create"))
            {
                _tchar RectTag[MAX_PATH] = {};
                CEffect_Rect::FXRECT_DESC RectDesc{};
                CEffect_Rect* pRect;

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_RectTag, strlen(m_RectTag), RectTag, MAX_PATH);

                RectDesc.strMyTag = RectTag;
                RectDesc.eMyType = EFFECT_TYPE::RECT;
                RectDesc.strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;

                m_tRectDesc.emplace(RectTag, RectDesc);

                tRectDesc = RectDesc;

                m_RectTag[0] = _T('\0');

                IsCreate = true;
                m_bTagFlag = false;
            }
        }
        ImGui::End();
    }
}

void CRect_Controller::UpdateSelected_RectFormTag(_wstring RectTag)
{

    auto iterRectDesc = m_tRectDesc.find(RectTag);
    
    if (iterRectDesc == m_tRectDesc.end())
    {
        m_pSelectedRectDesc = nullptr;
    }
    else
    {
        m_pSelectedRectDesc = &iterRectDesc->second;
    }


    if (m_pSelectedRectDesc != nullptr)
    {
        m_bSelectedRect = true;

        m_fColor[0] = m_pSelectedRectDesc->vColor.x;
        m_fColor[1] = m_pSelectedRectDesc->vColor.y;
        m_fColor[2] = m_pSelectedRectDesc->vColor.z;
        m_fColor[3] = m_pSelectedRectDesc->vColor.w;
    }
}

CEffect_Rect::FXRECT_DESC* CRect_Controller::Get_RectDesc(_wstring& RectTag)
{
    auto iter = m_tRectDesc.find(RectTag);
    
    if (iter == m_tRectDesc.end())
        return nullptr;

    return &iter->second;
}

void CRect_Controller::Set_RectDesc(_wstring& RectTag, CEffect_Rect::FXRECT_DESC& RectDesc)
{
    CEffect_Rect::FXRECT_DESC Desc = {};
    Desc = RectDesc;

    m_tRectDesc.emplace(RectTag, Desc);
}

void CRect_Controller::Set_RectTag(const _char* szRectTag)
{
    strcat_s(m_RectTag, szRectTag);

    m_bTagFlag = true;
}
void CRect_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterRectDesc = m_tRectDesc.find(DescTag);

    if (iterRectDesc != m_tRectDesc.end())
    {
        m_tRectDesc.erase(iterRectDesc);
    }


    m_iSelectedRect = 0;
    m_bSelectedRect = false;
    m_pSelectedRectDesc = nullptr;
   
    m_fColor[0] = 1.f;
    m_fColor[1] = 1.f;
    m_fColor[2] = 1.f;
    m_fColor[3] = 1.f;
}


CRect_Controller* CRect_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRect_Controller* pInstance = new CRect_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CRect_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CRect_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    m_Textures.clear();
}