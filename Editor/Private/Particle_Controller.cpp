#include "EditorPch.h"
#include "Particle_Controller.h"

CParticle_Controller::CParticle_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CParticle_Controller::Initialize()
{
    Load_AllTextureFromFolder("../../Client/Bin/Resource/Effect/Texture");

    return S_OK;
}

void CParticle_Controller::Update()
{
    Particle_Tab();
}

void CParticle_Controller::Render()
{

}

void CParticle_Controller::Load_AllTextureFromFolder(const _string& strFolderPath)
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
                PARTICLE_TEXTURE Desc{};
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

void CParticle_Controller::Particle_Tab()
{
    if (m_bSelectedParticle)
    {
        if (ImGui::Begin("Particle Info"))
        {
                if (ImGui::CollapsingHeader("VIBuffer", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Checkbox("Loop", &(m_pSelectedVBDesc->IsLoop));
                    ImGui::Checkbox("Stretch", &(m_pSelectedVBDesc->IsStretch));
                    if (ImGui::Checkbox("Sprite", &(m_pSelectedVBDesc->IsSprite)))
                    {
                        m_pSelectedParticleDesc->IsSprite = m_pSelectedVBDesc->IsSprite;
                    }
                    ImGui::Checkbox("Delay", &(m_pSelectedVBDesc->IsDelay));

					if (ImGui::Checkbox("SpawnRing", &(m_pSelectedVBDesc->IsSpawnRing)))
					{
						m_pSelectedVBDesc->IsSpawnBox = false;
					}

					ImGui::Checkbox("RingAngle", &(m_pSelectedVBDesc->IsRingAngle));

					if (ImGui::Checkbox("SpawnBox", &(m_pSelectedVBDesc->IsSpawnBox)))
					{
						m_pSelectedVBDesc->IsSpawnRing = false;
					}
                    ImGui::Separator();

					if (m_pSelectedVBDesc->IsSpawnRing)
					{
						ImGui::Text("RMin/RMax");
						ImGui::PushItemWidth(60);
						ImGui::InputFloat("##RMin", &(m_pSelectedVBDesc->fRmin));
						ImGui::SameLine();
						ImGui::InputFloat("##RMax", &(m_pSelectedVBDesc->fRmax));
						ImGui::PopItemWidth();

						ImGui::Text("Degree Start/End");
						ImGui::PushItemWidth(60);
						ImGui::InputFloat("##DegreeX", &(m_pSelectedVBDesc->fDegreeAngle.x));
						ImGui::SameLine();
						ImGui::InputFloat("##DegreeY", &(m_pSelectedVBDesc->fDegreeAngle.y));
						ImGui::PopItemWidth();
						ImGui::Separator();
					}

                    ImGui::Text("NumInstance");
                    ImGui::PushItemWidth(100);
                    ImGui::DragInt("##NumInstance", (int*)&(m_pSelectedVBDesc->iNumInstance));
                    ImGui::PopItemWidth();

                    ImGui::PushItemWidth(200);

                    ImGui::Text("SpreadWeight");
                    ImGui::SameLine();
                    ImGui::DragFloat("##SpreadW", &(m_pSelectedVBDesc->fSpreadWeight), 0.1f ,0.f, 1.f);

                    ImGui::Text("DropWeight");
                    ImGui::SameLine();
                    ImGui::DragFloat("##DorpW", &(m_pSelectedVBDesc->fDropWeight), 0.1f, 0.f, 1.f);

                    ImGui::Text("RotationWeight");
                    ImGui::SameLine();
                    ImGui::DragFloat("##RotationW", &(m_pSelectedVBDesc->fRotationWeight), 0.1f, 0.f, 1.f);

                    ImGui::Text("Gravity");
                    ImGui::SameLine();
                    ImGui::DragFloat("##Gravity", &(m_pSelectedVBDesc->fGravity), 0.1f, 0.f, 9.8f);
     
                    ImGui::PopItemWidth();

					ImGui::Separator();
                    ////////////////////////////////// 스트레치 빌보드 설정
                    if (m_pSelectedVBDesc->IsStretch)
                    {
                        ImGui::Text("StretchWeight");
                        ImGui::SameLine();
                        ImGui::DragFloat("##StretchWeight", &(m_pSelectedVBDesc->fStretchWeight), 0.1f, 0.f, 1.f);

                        ImGui::Text("StretchMin/Max");
                        ImGui::PushItemWidth(60);
                        ImGui::InputFloat("##StretchMin", &(m_pSelectedVBDesc->fStretchRange.x));
                        ImGui::SameLine();
                        ImGui::InputFloat("##StretchMax", &(m_pSelectedVBDesc->fStretchRange.y));
                        ImGui::PopItemWidth();

                        ImGui::Separator();
                    }
                    ////////////////////////////////// 

                    //////////////////////////////////  스프라이트 설정
                    if (m_pSelectedVBDesc->IsSprite)
                    {
                        ImGui::Text("SpriteWeight");
                        ImGui::PushItemWidth(100);
                        ImGui::DragFloat("##SpriteWeight", &(m_pSelectedVBDesc->fSpriteWeight), 0.1f, 0.f, 1.f);
                        ImGui::PopItemWidth();

                        ImGui::Text("DefaultSpeed");
                        ImGui::PushItemWidth(100);
                        ImGui::DragFloat("##DefaultSpeed", &(m_pSelectedVBDesc->fDefualtSpeed), 0.1f, 0.f, 2.5f);
                        ImGui::PopItemWidth();

                        ImGui::Separator();
                    }
                    //////////////////////////////////

                    ////////////////////////////////// 딜레이 설정
                    if (m_pSelectedVBDesc->IsDelay)
                    {
                        ImGui::Text("Delay");
                        ImGui::PushItemWidth(100);
                        ImGui::DragFloat("##Delay", &(m_pSelectedVBDesc->fDelay.y), 0.1f, 0.f, 3.f);
                        ImGui::PopItemWidth();
                        ImGui::Separator();
                    }
                    //////////////////////////////////

                    ImGui::Text("Center");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##CenterX", &(m_pSelectedVBDesc->vCenter.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##CenterY", &(m_pSelectedVBDesc->vCenter.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##CenterZ", &(m_pSelectedVBDesc->vCenter.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("Size");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##SizeX", &(m_pSelectedVBDesc->vSize.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##SizeY", &(m_pSelectedVBDesc->vSize.y));
                    ImGui::PopItemWidth();

                    ImGui::Text("Range");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##RangeX", &(m_pSelectedVBDesc->vRange.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##RangeY", &(m_pSelectedVBDesc->vRange.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##RangeZ", &(m_pSelectedVBDesc->vRange.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("Pivot");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##PivotX", &(m_pSelectedVBDesc->vPivot.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##PivotY", &(m_pSelectedVBDesc->vPivot.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##PivotZ", &(m_pSelectedVBDesc->vPivot.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("Speed");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##SppedX", &(m_pSelectedVBDesc->vSpeed.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##SppedY", &(m_pSelectedVBDesc->vSpeed.y));
                    ImGui::PopItemWidth();

                    ImGui::Text("LifeTime");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##LifeTimeX", &(m_pSelectedVBDesc->vLifeTime.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##LifeTimeY", &(m_pSelectedVBDesc->vLifeTime.y));
                    ImGui::PopItemWidth();
                }

                if (ImGui::CollapsingHeader("Particle", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Checkbox("Root", &(m_pSelectedParticleDesc->IsRootOn));

					ImGui::Checkbox("Pivot", &(m_pSelectedParticleDesc->IsPivot));

					ImGui::Checkbox("IsLoop", &(m_pSelectedParticleDesc->IsLoop));

                    ImGui::Text("ShaderPass");
                    ImGui::PushItemWidth(100);
                    ImGui::DragInt("##ShaderPass", &(m_pSelectedParticleDesc->fShaderPass), 1.f, 0, 6);
                    ImGui::PopItemWidth();

					ImGui::Text("Mask");
					ImGui::PushItemWidth(100);
					if (ImGui::Button("A Cut"))
						m_pSelectedParticleDesc->iMaskFlag = 1;
					ImGui::SameLine();
					if (ImGui::Button("RGB Cut"))
						m_pSelectedParticleDesc->iMaskFlag = 0;
					ImGui::PopItemWidth();


                    ImGui::Text("Size");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##ParticleSizeX", &(m_pSelectedParticleDesc->vSize.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticleSizeY", &(m_pSelectedParticleDesc->vSize.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticleSizeZ", &(m_pSelectedParticleDesc->vSize.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("Position");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##ParticlePosX", &(m_pSelectedParticleDesc->vPos.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticlePosY", &(m_pSelectedParticleDesc->vPos.y));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticlePosZ", &(m_pSelectedParticleDesc->vPos.z));
                    ImGui::PopItemWidth();

                    ImGui::Text("LifeTime");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##ParticleLifeTimeX", &(m_pSelectedParticleDesc->vLifeTime.x));
                    ImGui::SameLine();
                    ImGui::InputFloat("##ParticleLifeTimeY", &(m_pSelectedParticleDesc->vLifeTime.y));
                    ImGui::PopItemWidth();

                    if (ImGui::ColorEdit4("Color", m_fColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview))
                    {
                        m_pSelectedParticleDesc->vColor = _float4(m_fColor[0], m_fColor[1], m_fColor[2], m_fColor[3]);
                    }

                    ImGui::Separator();
                   
                    if (m_pSelectedParticleDesc->IsSprite)
                    {
                        ImGui::Text("Row / Col");
                        ImGui::PushItemWidth(60);
                        ImGui::InputInt("##Row", &(m_pSelectedParticleDesc->iRows));
                        ImGui::SameLine();
                        ImGui::InputInt("##Col", &(m_pSelectedParticleDesc->iCols));
                        ImGui::PopItemWidth();
                    }
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
                                m_pSelectedParticleDesc->strTextureTag = m_Textures[i].strTextureTag;
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

void CParticle_Controller::Particle_Base_Tab(CParticle::PARTICLE_DESC& tParticleDesc, _bool& IsCreate)
{
    if (ImGui::Begin("Particle Base"))
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
                _tchar ParticleTag[MAX_PATH] = {};
                CParticle::PARTICLE_DESC ParticleDesc{};
                CVIBuffer_Point_Instance::POINT_INSTANCE_DESC VIBufferDesc{};
                CParticle* pParticle;

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_ParticleTag, strlen(m_ParticleTag), ParticleTag, MAX_PATH);

                ParticleDesc.strMyTag = ParticleTag;
                ParticleDesc.eMyType = EFFECT_TYPE::PARTICLE;
                ParticleDesc.strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;
                ParticleDesc.strVIBufferTag = TEXT("Prototype_Componenet_VIBuffer_Instance_Point_");
                ParticleDesc.strVIBufferTag += ParticleTag;
           
                VIBufferDesc.iNumInstance = 1;
                VIBufferDesc.vSize = _float2(5.f, 5.f);

                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), ParticleDesc.strVIBufferTag,
                    CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, &VIBufferDesc));

                m_tParticleDesc.emplace(ParticleTag, ParticleDesc);
                m_tVBDesc.emplace(ParticleTag, VIBufferDesc);

                tParticleDesc = ParticleDesc;

                m_ParticleTag[0] = _T('\0');

                IsCreate = true;
                m_bTagFlag = false;
            }
        }
        ImGui::End();
    }
}

void CParticle_Controller::UpdateSelected_ParticleFormTag(_wstring ParticleTag)
{

    auto iterParticleDesc = m_tParticleDesc.find(ParticleTag);
    
    if (iterParticleDesc == m_tParticleDesc.end())
    {
        m_pSelectedParticleDesc = nullptr;
    }
    else
    {
        m_pSelectedParticleDesc = &iterParticleDesc->second;
    }

    auto iterVBDesc = m_tVBDesc.find(ParticleTag);

    if (iterVBDesc == m_tVBDesc.end())
    {
        m_pSelectedVBDesc = nullptr;
    }
    else
    {
        m_pSelectedVBDesc = &iterVBDesc->second;
    }

    if (m_pSelectedParticleDesc != nullptr)
    {
        m_bSelectedParticle = true;

        m_fColor[0] = m_pSelectedParticleDesc->vColor.x;
        m_fColor[1] = m_pSelectedParticleDesc->vColor.y;
        m_fColor[2] = m_pSelectedParticleDesc->vColor.z;
        m_fColor[3] = m_pSelectedParticleDesc->vColor.w;
    }
}

CParticle::PARTICLE_DESC* CParticle_Controller::Get_ParticleDesc(_wstring& ParticleTag)
{
    auto iter = m_tParticleDesc.find(ParticleTag);
    
    if (iter == m_tParticleDesc.end())
        return nullptr;

    return &iter->second;
}

CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* CParticle_Controller::Get_VBDesc(_wstring& VBTag)
{
    auto iter = m_tVBDesc.find(VBTag);

    if (iter == m_tVBDesc.end())
        return nullptr;

    return &iter->second;
}
void CParticle_Controller::Set_ParticleDesc(_wstring& ParticleTag, CParticle::PARTICLE_DESC& ParticleDesc)
{
    CParticle::PARTICLE_DESC Desc = {};
    Desc = ParticleDesc;

    m_tParticleDesc.emplace(ParticleTag, Desc);
}
void CParticle_Controller::Set_VBDesc(_wstring& ParticleTag, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC& ParticlVBeDesc)
{
    CVIBuffer_Point_Instance::POINT_INSTANCE_DESC Desc = {};
    Desc = ParticlVBeDesc;

    m_tVBDesc.emplace(ParticleTag, Desc);
}
void CParticle_Controller::Set_ParticleTag(const _char* szParticleTag)
{
    strcat_s(m_ParticleTag, szParticleTag);

    m_bTagFlag = true;
}
void CParticle_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterParticleDesc = m_tParticleDesc.find(DescTag);

    if (iterParticleDesc != m_tParticleDesc.end())
    {
        m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), iterParticleDesc->second.strVIBufferTag);

        m_tParticleDesc.erase(iterParticleDesc);
    }

    auto iterVBDesc = m_tVBDesc.find(DescTag);

    if (iterVBDesc != m_tVBDesc.end())
    {
        m_tVBDesc.erase(iterVBDesc);
    }

    m_iSelectedParticle = 0;
    m_bSelectedParticle = false;
    m_pSelectedParticleDesc = nullptr;
    m_pSelectedVBDesc = nullptr;
   
    m_fColor[0] = 0.f;
    m_fColor[1] = 0.f;
    m_fColor[2] = 0.f;
    m_fColor[3] = 0.f;
}

//CParticle::PARTICLE_DESC CParticle_Controller::Find_Particle(_tchar ParticleTag)
//{
//    for (auto iter = m_tParticleDesc.begin(); iter != m_tParticleDesc.end(); )
//    {
//        if (iter->first == &ParticleTag)
//        {
//            return iter->second;
//        }
//        else
//            ++iter;
//    }
//}

CParticle_Controller* CParticle_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CParticle_Controller* pInstance = new CParticle_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CParticle_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CParticle_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    m_Textures.clear();
}