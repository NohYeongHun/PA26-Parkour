#include "EditorPch.h"
#include "TrailMesh_Controller.h"

CTrailMesh_Controller::CTrailMesh_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CTrailMesh_Controller::Initialize()
{
    Load_AllTextureFromFolder("../../Client/Bin/Resource/Effect/TrailMesh/Texture");
    Load_AllMeshDatFromFolder("../../Client/Bin/Resource/Effect/TrailMesh/Dat");
    Load_AllColorTextureFormFolder("../../Client/Bin/Resource/Effect/TrailMesh/Color");
	Load_AllDissolveTextureFromFolder("../../Client/Bin/Resource/Effect/TrailMesh/Dissolve");
	Load_AllDistortionTextureFromFolder("../../Client/Bin/Resource/Effect/TrailMesh/Distortion");

    return S_OK;
}

void CTrailMesh_Controller::Update()
{
    TrailMesh_Tab();
}

void CTrailMesh_Controller::Render()
{

}

void CTrailMesh_Controller::Load_AllTextureFromFolder(const _string& strFolderPath)
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
                MESH_TEXTURE Desc{};
                CTexture* pTexture = {};

                // 확장자 제외한 파일명
                _string strTextureTag = entry.path().stem().string();

                //파일명으로 텍스처 이름 지정
                strcpy_s(Desc.szName, sizeof(Desc.szName), strTextureTag.c_str());

                //파일명으로 텍스처 컴포넌트 이름 지정
                _char szDefault[MAX_PATH];
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");
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
void CTrailMesh_Controller::Load_AllMeshDatFromFolder(const _string& strFolderPath)
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
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Mesh_");
                strcat_s(szDefault, Desc.szName);
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDefault, strlen(szDefault), Desc.strMeshTag, MAX_PATH);

                _string DefaultPath = "/Resource/Effect/TrailMesh/Dat/";

                DefaultPath += fileName;

                strcpy_s(Desc.szDatPath, sizeof(Desc.szDatPath), DefaultPath.c_str());

                //매쉬버퍼 컴포넌트 생성
                _float fSize = 0.01f;
                _fmatrix DefualtMatrix = XMMatrixScaling(fSize, fSize, fSize)/* * XMMatrixRotationX(90.f)*/;
                /* * XMMatrixRotationX(XM_PIDIV2) * XMMatrixRotationY(XM_PI)*/;
             /*   _fmatrix DefualtMatrix = XMMatrixIdentity() * XMMatrixRotationX(-90);*/

                m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), Desc.strMeshTag,
                   CVIBuffer_Mesh::Create(m_pDevice, m_pContext, filePath.c_str(), DefualtMatrix));

                //m_tVBMeshDesc.emplace(strMeshTag, FXMeshDesc);
                m_MeshVBTag.push_back(Desc);
            }
        }
    }
}

void CTrailMesh_Controller::Load_AllColorTextureFormFolder(const _string& strFolderPath)
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
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");
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

void CTrailMesh_Controller::Load_AllDissolveTextureFromFolder(const _string& strFolderPath)
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
				DISSOLVE_TEXTURE Desc{};
				CTexture* pTexture = {};

				// 확장자 제외한 파일명
				_string strTextureTag = entry.path().stem().string();

				//파일명으로 텍스처 이름 지정
				strcpy_s(Desc.szName, sizeof(Desc.szName), strTextureTag.c_str());

				//파일명으로 텍스처 컴포넌트 이름 지정
				_char szDefault[MAX_PATH];
				strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");
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

				m_DissolveTextures.push_back(Desc);
			}
		}
	}
}

void CTrailMesh_Controller::Load_AllDistortionTextureFromFolder(const _string& strFolderPath)
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
				DISTORTION_TEXTURE Desc{};
				CTexture* pTexture = {};

				// 확장자 제외한 파일명
				_string strTextureTag = entry.path().stem().string();

				//파일명으로 텍스처 이름 지정
				strcpy_s(Desc.szName, sizeof(Desc.szName), strTextureTag.c_str());

				//파일명으로 텍스처 컴포넌트 이름 지정
				_char szDefault[MAX_PATH];
				strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_Texture_");
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

				m_DistortionTextures.push_back(Desc);
			}
		}
	}
}

void CTrailMesh_Controller::TrailMesh_Tab()
{
    if (m_bSelectedMesh)
    {
        if (ImGui::Begin("TrailMesh Info"))
        {
            if (ImGui::CollapsingHeader("TrailMesh", ImGuiTreeNodeFlags_DefaultOpen))
            {
                /*      ImGui::Checkbox("Spread", &(m_pSelectedTrailMeshDesc->bSpread));
                      ImGui::Checkbox("Drop", &(m_pSelectedTrailMeshDesc->bDrop));*/
                ImGui::Text("ShaderPass");
                ImGui::PushItemWidth(100);
                ImGui::DragInt("##ShaderPass", &(m_pSelectedTrailMeshDesc->iShaderPass), 1.f, 0, 6);
                ImGui::PopItemWidth();

				ImGui::Separator();

				ImGui::Checkbox("Dissolve", &(m_pSelectedTrailMeshDesc->IsDissolve));

				ImGui::Checkbox("Distortion", &(m_pSelectedTrailMeshDesc->IsDistortion));

				ImGui::Checkbox("Loop", &(m_pSelectedTrailMeshDesc->IsLoop));

				if (m_pSelectedTrailMeshDesc->IsDistortion)
				{
					ImGui::Text("DistortionWeight");
					ImGui::PushItemWidth(100);
					ImGui::InputFloat("##DistortionWeight", &(m_pSelectedTrailMeshDesc->fDistortionWeight));
					ImGui::PopItemWidth();
				}
				ImGui::Separator();

                ImGui::Text("Sweep");
                ImGui::PushItemWidth(100);
                ImGui::InputFloat("##Sweep", &(m_pSelectedTrailMeshDesc->fSweep));
                ImGui::PopItemWidth();

                ImGui::Text("SweepWitdh");
                ImGui::PushItemWidth(100);
                ImGui::InputFloat("##SweepWitdh", &(m_pSelectedTrailMeshDesc->fSweepWitdh));
                ImGui::PopItemWidth();

				ImGui::Text("Soft");
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##Soft", &(m_pSelectedTrailMeshDesc->fSoft));
				ImGui::PopItemWidth();

				ImGui::Text("ColorSpeed");
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##ColorSpeed", &(m_pSelectedTrailMeshDesc->fColorSpeed));
				ImGui::PopItemWidth();

				ImGui::Text("MaskSpeed");
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##MaskSpeed", &(m_pSelectedTrailMeshDesc->fMaskSpeed));
				ImGui::PopItemWidth();

				ImGui::Text("Alpha");
				ImGui::PushItemWidth(60);
				ImGui::InputFloat("##Alpha", &(m_pSelectedTrailMeshDesc->fAlpha));
				ImGui::PopItemWidth();

				ImGui::Text("ColorGain");
				ImGui::PushItemWidth(60);
				ImGui::InputFloat("##ColorGain", &(m_pSelectedTrailMeshDesc->fColorGain));
				ImGui::PopItemWidth();

				ImGui::Text("ColorGamma");
				ImGui::PushItemWidth(60);
				ImGui::InputFloat("##ColorGamma", &(m_pSelectedTrailMeshDesc->fColorGamma));
				ImGui::PopItemWidth();

                ImGui::Text("Dir");
                ImGui::PushItemWidth(100);
                if (ImGui::Button("Left"))
                    m_pSelectedTrailMeshDesc->iDirFlag = 0;
                ImGui::SameLine();
                if (ImGui::Button("Right"))
                    m_pSelectedTrailMeshDesc->iDirFlag = 1;
                ImGui::PopItemWidth();

				ImGui::Text("Mask");
				ImGui::PushItemWidth(100);
				if (ImGui::Button("R Cut"))
					m_pSelectedTrailMeshDesc->iMaskFlag = 0;
				ImGui::SameLine();
				if (ImGui::Button("A Cut"))
					m_pSelectedTrailMeshDesc->iMaskFlag = 1;
				ImGui::PopItemWidth();

                ImGui::Text("Size");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##ParticleSizeX", &(m_pSelectedTrailMeshDesc->vSize.x));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticleSizeY", &(m_pSelectedTrailMeshDesc->vSize.y));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticleSizeZ", &(m_pSelectedTrailMeshDesc->vSize.z));
                ImGui::PopItemWidth();

                ImGui::Text("Position");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##ParticlePosX", &(m_pSelectedTrailMeshDesc->vPos.x));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticlePosY", &(m_pSelectedTrailMeshDesc->vPos.y));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticlePosZ", &(m_pSelectedTrailMeshDesc->vPos.z));
                ImGui::PopItemWidth();

                ImGui::Text("Color");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##ParticleColorX", &(m_pSelectedTrailMeshDesc->vColor.x));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticleColorY", &(m_pSelectedTrailMeshDesc->vColor.y));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticleColorZ", &(m_pSelectedTrailMeshDesc->vColor.z));
                ImGui::PopItemWidth();

                ImGui::Text("LifeTime");
                ImGui::PushItemWidth(60);
                ImGui::InputFloat("##ParticleLifeTimeX", &(m_pSelectedTrailMeshDesc->vLifeTime.x));
                ImGui::SameLine();
                ImGui::InputFloat("##ParticleLifeTimeY", &(m_pSelectedTrailMeshDesc->vLifeTime.y));
                ImGui::PopItemWidth();

                ImGui::Checkbox("Bone Rotation", &(m_pSelectedTrailMeshDesc->IsRootOn));
            }

            if (ImGui::CollapsingHeader("Base", ImGuiTreeNodeFlags_DefaultOpen))
            {
                vector<const _char*> szMeshTag = {};

                for (auto iter = m_MeshVBTag.begin(); iter != m_MeshVBTag.end(); ++iter)
                {
                    szMeshTag.push_back(iter->szName);
                }
                //매쉬 리스트박스 띄우기
                if (ImGui::ListBox("Trail Mesh", &m_iSelectedMeshVBTag, szMeshTag.data(), int(szMeshTag.size()), int(szMeshTag.size() + 2)))
                {
                    m_pSelectedTrailMeshDesc->strVIBufferTag = m_MeshVBTag[m_iSelectedMeshVBTag].strMeshTag;
                }

                //매쉬 기본 색상 텍스처 설정
                if (ImGui::BeginCombo("Texture", "")) {
                    for (size_t i = 0; i < m_Textures.size(); i++)
                    {
                        bool IsSelected = (m_iSelectedTexture == i);
                        if (ImGui::Selectable(m_Textures[i].szName, IsSelected))
                        {
                            m_iSelectedTexture = i;
                            m_pSelectedTrailMeshDesc->strTextureTag = m_Textures[i].strTextureTag;
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
                            m_pSelectedTrailMeshDesc->strColorTextureTag = m_ColorTextures[i].strTextureTag;
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


				ImGui::Separator();
				if (m_pSelectedTrailMeshDesc->IsDissolve)
				{
					//디졸브 텍스처
					if (ImGui::BeginCombo("Dissolves", "")) {
						for (size_t i = 0; i < m_DissolveTextures.size(); i++)
						{
							bool IsSelected = (m_iSelectedDissovle == i);
							if (ImGui::Selectable(m_DissolveTextures[i].szName, IsSelected))
							{
								m_iSelectedDissovle = i;
								m_pSelectedTrailMeshDesc->strDissolveTextureTag = m_DissolveTextures[i].strTextureTag;

							}

							if (IsSelected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}

					ImGui::Separator();
					if (m_iSelectedDissovle >= 0) {
						ImGui::Image((ImTextureID)m_DissolveTextures[m_iSelectedDissovle].pTexture->Get_SRV(0), ImVec2(256, 256));
					}
				}

				ImGui::Separator();
				if (m_pSelectedTrailMeshDesc->IsDistortion)
				{
					//디토 텍스처
					if (ImGui::BeginCombo("Distortions", "")) {
						for (size_t i = 0; i < m_DistortionTextures.size(); i++)
						{
							bool IsSelected = (m_iSelectedDistortion == i);
							if (ImGui::Selectable(m_DistortionTextures[i].szName, IsSelected))
							{
								m_iSelectedDistortion = i;
								m_pSelectedTrailMeshDesc->strDistortionTextureTag = m_DistortionTextures[i].strTextureTag;

							}

							if (IsSelected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}

					ImGui::Separator();
					if (m_iSelectedDistortion >= 0) {
						ImGui::Image((ImTextureID)m_DistortionTextures[m_iSelectedDistortion].pTexture->Get_SRV(0), ImVec2(256, 256));
					}

				}
            }
            ImGui::End();
        }
    }

}

void CTrailMesh_Controller::TrailMesh_Base_Tab(CTrail_Mesh::TRAILMESH_DESC& tTrailMeshDesc, _bool& IsCreate)
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
            //선택된 매쉬태그 임시저장? 
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

        //Root 설정
        ImGui::Checkbox("Root", &m_IsRoot);

        if (m_bTagFlag && m_bMeshVBTag) //이펙트 컨트롤러가 설정해준 이름값이 있고, 선택한 매쉬버퍼가 있어야지만 생성할 수 있게.
        {
            if (ImGui::Button("Create"))
            {
                //프리팹에게 Desc 전달 -> 프리팹이 Desc로 클론 진행

                _tchar TrailMeshTag[MAX_PATH] = {};
                CTrail_Mesh::TRAILMESH_DESC TrailMeshDesc{};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_TrailMeshTag, strlen(m_TrailMeshTag), TrailMeshTag, MAX_PATH);

                //이펙트 매쉬 이름 및 클론할 컴포넌트 이름들
                TrailMeshDesc.strMyTag = TrailMeshTag;
                TrailMeshDesc.eMyType = EFFECT_TYPE::TRAIL;
                TrailMeshDesc.strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;
                TrailMeshDesc.strVIBufferTag = m_MeshVBTag[m_iSelectedMeshVBTag].strMeshTag;
                TrailMeshDesc.strColorTextureTag = m_ColorTextures[m_iSelectedColor].strTextureTag;

                //이펙트매쉬(오브젝트)가 가질 디폴트 설정값.
                TrailMeshDesc.vLifeTime.y = 10.f;
                TrailMeshDesc.vPos = _float3(0.f, 0.f, 0.f);
                TrailMeshDesc.vSize = _float3(0.5f, 0.5f, 0.5f);
                TrailMeshDesc.iShaderPass = 0;

                _char szDatPath[MAX_PATH] = {};

                strcpy_s(szDatPath, sizeof(szDatPath), "../../Client/Bin");
                strcat_s(szDatPath, sizeof(szDatPath), m_MeshVBTag[m_iSelectedMeshVBTag].szDatPath);

                ////Desc에 VBMesh 이름 저장?
                //_tchar strFXMehsTag[MAX_PATH] = {};
                //MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_MeshVBTag[m_iSelectedMeshVBTag].szName, strlen(m_MeshVBTag[m_iSelectedMeshVBTag].szName), strFXMehsTag, MAX_PATH);
               
                if (m_IsRoot)
                {
                    TrailMeshDesc.IsRootOn = true;
                    m_IsRoot = false;
                }

                //매쉬이펙트 와 매쉬VB태그를 맞춰야할지는 고민해보자.
                m_tTrailMeshDesc.emplace(TrailMeshTag, TrailMeshDesc);

                tTrailMeshDesc = TrailMeshDesc;

                //생성됐음을 이펙트 컨트롤러에게 전달
                IsCreate = true;

                //초기화
                m_TrailMeshTag[0] = _T('\0');
                m_bTagFlag = false;
            }
        }
       
        ImGui::End();
    }

}

void CTrailMesh_Controller::Set_TrailMeshTag(const _char* szMeshTag)
{
    strcat_s(m_TrailMeshTag, szMeshTag);

    m_bTagFlag = true;
}

void CTrailMesh_Controller::UpdateSelected_TrailMeshFormTag(_wstring FXMeshTag)
{
    auto iterEffectMeshDesc = m_tTrailMeshDesc.find(FXMeshTag);
    
    if (iterEffectMeshDesc == m_tTrailMeshDesc.end())
        m_pSelectedTrailMeshDesc = nullptr;
    else
        m_pSelectedTrailMeshDesc = &iterEffectMeshDesc->second;

    if (m_pSelectedTrailMeshDesc != nullptr)
        m_bSelectedMesh = true;

}

CTrail_Mesh::TRAILMESH_DESC* CTrailMesh_Controller::Get_TrailMeshDesc(_wstring& EffectMeshTag)
{
    auto iter = m_tTrailMeshDesc.find(EffectMeshTag);

    if (iter == m_tTrailMeshDesc.end())
        return nullptr;

    return &iter->second;
}

void CTrailMesh_Controller::Set_TrailMeshDesc(_wstring& TrailMeshTag, CTrail_Mesh::TRAILMESH_DESC& TrailDesc)
{
    CTrail_Mesh::TRAILMESH_DESC Desc = {};
    Desc = TrailDesc;

    m_tTrailMeshDesc.emplace(TrailMeshTag, Desc);
}


void CTrailMesh_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterEffectMeshDesc = m_tTrailMeshDesc.find(DescTag);

    if (iterEffectMeshDesc != m_tTrailMeshDesc.end())
    {
        m_tTrailMeshDesc.erase(iterEffectMeshDesc);
    }

    //초기화
    m_bSelectedMesh = false;
    m_pSelectedTrailMeshDesc = nullptr;
}

CTrailMesh_Controller* CTrailMesh_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTrailMesh_Controller* pInstance = new CTrailMesh_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CTrailMesh_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTrailMesh_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    m_Textures.clear();

}