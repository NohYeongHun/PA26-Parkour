#include "EditorPch.h"
#include "Spectrum_Controller.h"

CSpectrum_Controller::CSpectrum_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CSpectrum_Controller::Initialize()
{
    Load_AllTextureFromFolder("../../Client/Bin/Resource/Effect/Spectrum/Mask");
    Load_AllColorTextureFormFolder("../../Client/Bin/Resource/Effect/Spectrum/Color");

    return S_OK;
}

void CSpectrum_Controller::Update()
{
	Spectrum_Info_Tab();
}

void CSpectrum_Controller::Render()
{

}

void CSpectrum_Controller::Load_AllTextureFromFolder(const _string& strFolderPath)
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
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_SpectrumTexture_");
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


void CSpectrum_Controller::Load_AllColorTextureFormFolder(const _string& strFolderPath)
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
                strcpy_s(szDefault, sizeof(szDefault), "Prototype_Component_SpectrumTexture_");
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


void CSpectrum_Controller::Spectrum_Tab()
{
    if (m_bSelectedSpectrum)
    {
        if (ImGui::Begin("Spectrum Info"))
        {
            if (ImGui::CollapsingHeader("Spectrum", ImGuiTreeNodeFlags_DefaultOpen))
            {
                /*      ImGui::Checkbox("Spread", &(m_pSelectedSpectrumDesc->bSpread));
                      ImGui::Checkbox("Drop", &(m_pSelectedSpectrumDesc->bDrop));*/
                ImGui::Text("ShaderPass");
                ImGui::PushItemWidth(100);
                ImGui::DragInt("##ShaderPass", &(m_pSelectedSpectrumDesc->iShaderPass), 1.f);
                ImGui::PopItemWidth();

				ImGui::Separator();

                ImGui::Text("Generation");
                ImGui::PushItemWidth(100);
                ImGui::InputFloat("##Generation", &(m_pSelectedSpectrumDesc->fGeneration));
                ImGui::PopItemWidth();

                ImGui::Text("LifeTime");
                ImGui::PushItemWidth(100);
                ImGui::InputFloat("##SweepWitdh", &(m_pSelectedSpectrumDesc->fLifeTime));
                ImGui::PopItemWidth();

				/*ImGui::Text("Soft");
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##Soft", &(m_pSelectedSpectrumDesc.));
				ImGui::PopItemWidth();

				ImGui::Text("ColorSpeed");
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##ColorSpeed", &(m_pSelectedSpectrumDesc->fColorSpeed));
				ImGui::PopItemWidth();

				ImGui::Text("MaskSpeed");
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##MaskSpeed", &(m_pSelectedSpectrumDesc->fMaskSpeed));
				ImGui::PopItemWidth();

				ImGui::Text("Alpha");
				ImGui::PushItemWidth(60);
				ImGui::InputFloat("##Alpha", &(m_pSelectedSpectrumDesc->fAlpha));
				ImGui::PopItemWidth();

				ImGui::Text("ColorGain");
				ImGui::PushItemWidth(60);
				ImGui::InputFloat("##ColorGain", &(m_pSelectedSpectrumDesc->fColorGain));
				ImGui::PopItemWidth();

				ImGui::Text("ColorGamma");
				ImGui::PushItemWidth(60);
				ImGui::InputFloat("##ColorGamma", &(m_pSelectedSpectrumDesc->fColorGamma));
				ImGui::PopItemWidth();

                ImGui::Text("Dir");
                ImGui::PushItemWidth(100);
                if (ImGui::Button("Left"))
                    m_pSelectedSpectrumDesc->iDirFlag = 0;
                ImGui::SameLine();
                if (ImGui::Button("Right"))
                    m_pSelectedSpectrumDesc->iDirFlag = 1;
                ImGui::PopItemWidth();

				ImGui::Text("Mask");
				ImGui::PushItemWidth(100);
				if (ImGui::Button("R Cut"))
					m_pSelectedSpectrumDesc->iMaskFlag = 0;
				ImGui::SameLine();
				if (ImGui::Button("A Cut"))
					m_pSelectedSpectrumDesc->iMaskFlag = 1;
				ImGui::PopItemWidth();*/

            }

			if (ImGui::CollapsingHeader("VBSpectrum", ImGuiTreeNodeFlags_DefaultOpen))
			{
				/*      ImGui::Checkbox("Spread", &(m_pSelectedSpectrumDesc->bSpread));
					  ImGui::Checkbox("Drop", &(m_pSelectedSpectrumDesc->bDrop));*/
				ImGui::Text("MaxSamples");
				ImGui::PushItemWidth(100);
				ImGui::DragInt("##MaxSamples", &(m_pSelectedSpectrumVBDesc->MaxSamples), 1.f);
				ImGui::PopItemWidth();

				ImGui::Separator();

				ImGui::Text("Size");
				ImGui::PushItemWidth(100);
				ImGui::InputFloat("##Size", &(m_pSelectedSpectrumVBDesc->fSize));
				ImGui::PopItemWidth();
			}

            if (ImGui::CollapsingHeader("Base", ImGuiTreeNodeFlags_DefaultOpen))
            {
             
                //매쉬 기본 색상 텍스처 설정
                if (ImGui::BeginCombo("Texture", "")) {
                    for (size_t i = 0; i < m_Textures.size(); i++)
                    {
                        bool IsSelected = (m_iSelectedTexture == i);
                        if (ImGui::Selectable(m_Textures[i].szName, IsSelected))
                        {
                            m_iSelectedTexture = i;
                            m_pSelectedSpectrumDesc->strTextureTag = m_Textures[i].strTextureTag;
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
                            m_pSelectedSpectrumDesc->strColorTextureTag = m_ColorTextures[i].strTextureTag;
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
        }
		ImGui::End();
    }

}

void CSpectrum_Controller::Spectrum_Info_Tab()
{
	if (ImGui::Begin("Spectrum_Info"))
	{
		if (ImGui::InputText("SpectrumTag", m_SpectrumTag, IM_ARRAYSIZE(m_SpectrumTag), ImGuiInputTextFlags_EnterReturnsTrue))
			m_bTagFlag = true;

		if (m_bTagFlag)
		{
			if (ImGui::Button("Create Spectrum"))
				m_IsBaseFlag = true;

			if (m_IsBaseFlag)
				Spectrum_Base_Tab();
		}

		ImGui::NewLine();

		if (!m_tSpectrumDesc.empty())
		{
			vector<_string> strSpectrumTag = {};
			vector<const _char*> szSpectrumTag = {};

			for (auto iter = m_tSpectrumDesc.begin(); iter != m_tSpectrumDesc.end(); ++iter)
			{
				_string strTag = WStringToString(iter->first);

				strSpectrumTag.push_back(strTag);
			}

			//string -> char
			for (auto& strTag : strSpectrumTag)
			{
				szSpectrumTag.push_back(strTag.c_str());
			}

			if (ImGui::ListBox("SpectrumList", &m_iSelectedSpectrum, szSpectrumTag.data(), int(szSpectrumTag.size()), int(szSpectrumTag.size() + 2)))
			{
				UpdateSelected_SpectrumFromIndex();
			}

			if (m_pSelectedSpecturm != nullptr)
			{
				Spectrum_Tab();

				if (ImGui::Button("Apply"))
				{
					_wstring SpectrumTag = m_pSelectedSpecturm->Get_MyTag();
					_wstring SpectrumVBTag = m_pSelectedSpectrumDesc->strVIBufferTag;

					auto Spectrum = m_Sspectrums.find(m_pSelectedSpecturm->Get_MyTag());

					if (Spectrum != m_Sspectrums.end())
					{
						m_pSelectedSpecturm->SetActivate(false);

						Safe_Release(Spectrum->second);
						m_Sspectrums.erase(Spectrum);

						m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), m_pSelectedSpectrumDesc->strVIBufferTag);
						m_pSelectedSpecturm = nullptr;

						m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), SpectrumVBTag,
							CVIBuffer_Spectrum::Create(m_pDevice, m_pContext, m_pSelectedSpectrumVBDesc));

						CSpectrum* pSpectrum = nullptr;

						pSpectrum = static_cast<CSpectrum*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectSpectrum"), PROTOTYPE::GAMEOBJECT, m_pSelectedSpectrumDesc));

						m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::EFFECT), TEXT("Spectrum"), pSpectrum);

						m_Sspectrums.emplace(SpectrumTag, pSpectrum);
						m_pSelectedSpecturm = pSpectrum;

						_matrix Matrix = {};

						m_pSelectedSpecturm->Reset(Matrix, nullptr);
					}

				}

			}

			Save_SelectedSpectrum_To_Json();
		}
		ImGui::End();
	}
}

void CSpectrum_Controller::Spectrum_Base_Tab()
{
	if (ImGui::Begin("Spectrum Base"))
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

		if (m_bTagFlag)
		{
			if (ImGui::Button("Create"))
			{
				_tchar SpectrumTag[MAX_PATH] = {};
				CSpectrum::SPECTRUM_DESC SpectrumDesc{};
				CVIBuffer_Spectrum::VB_SPECTRUM_DESC VIBufferDesc{};
				CSpectrum* pSpectrum = nullptr;

				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_SpectrumTag, strlen(m_SpectrumTag), SpectrumTag, MAX_PATH);

				SpectrumDesc.strMyTag = SpectrumTag;
				SpectrumDesc.eMyType = EFFECT_TYPE::SPECTRUM;
				SpectrumDesc.strTextureTag = m_Textures[m_iSelectedTexture].strTextureTag;
				SpectrumDesc.strColorTextureTag = m_ColorTextures[m_iSelectedColor].strTextureTag;
				SpectrumDesc.strVIBufferTag = TEXT("Prototype_Componenet_VIBuffer_Spectrum_");
				SpectrumDesc.strVIBufferTag += SpectrumTag;
				SpectrumDesc.fLifeTime = 5.f;
				SpectrumDesc.iShaderPass = 0;
				SpectrumDesc.fGeneration = 0.2f;

				//기본값 설정
				VIBufferDesc.fSize = 1.f;
				VIBufferDesc.MaxSamples = 20.f;

				m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::EFFECT), SpectrumDesc.strVIBufferTag,
					CVIBuffer_Spectrum::Create(m_pDevice, m_pContext, &VIBufferDesc));

				m_tSpectrumDesc.emplace(SpectrumTag, SpectrumDesc);
				m_tVBDesc.emplace(SpectrumTag, VIBufferDesc);

				pSpectrum = static_cast<CSpectrum*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::EFFECT), TEXT("Prototype_GameObject_EffectSpectrum"), PROTOTYPE::GAMEOBJECT, &SpectrumDesc));

				m_Sspectrums.emplace(SpectrumTag, pSpectrum);

				m_SpectrumTag[0] = _T('\0');

				m_bTagFlag = false;
				m_IsBaseFlag = false;
			}
		}
		ImGui::End();
	}
}

void CSpectrum_Controller::Set_SpectrumTag(const _char* szSpectrumTag)
{
    strcat_s(m_SpectrumTag, szSpectrumTag);

    m_bTagFlag = true;
}

void CSpectrum_Controller::UpdateSelected_SpectrumFormTag(_wstring SpectrumTag)
{
    auto iterEffectSpectrumDesc = m_tSpectrumDesc.find(SpectrumTag);
    
    if (iterEffectSpectrumDesc == m_tSpectrumDesc.end())
        m_pSelectedSpectrumDesc = nullptr;
    else
        m_pSelectedSpectrumDesc = &iterEffectSpectrumDesc->second;

	auto iterEffectSpectrumVBDesc = m_tVBDesc.find(SpectrumTag);

	if (iterEffectSpectrumVBDesc == m_tVBDesc.end())
		m_pSelectedSpectrumVBDesc = nullptr;
	else
		m_pSelectedSpectrumVBDesc = &iterEffectSpectrumVBDesc->second;

    if (m_pSelectedSpectrumDesc != nullptr)
        m_bSelectedSpectrum = true;

}

void CSpectrum_Controller::UpdateSelected_SpectrumFromIndex()
{
	if (m_pSelectedSpecturm != nullptr)
		m_pSelectedSpecturm->SetActivate(false);

	_int iCheckIndex = 0;

	m_pSelectedSpecturm = nullptr;

	for (auto iter = m_Sspectrums.begin(); iter != m_Sspectrums.end();)
	{
		if (iCheckIndex == m_iSelectedSpectrum)
		{
			m_pSelectedSpecturm = iter->second;
			iCheckIndex = 0;
			break;
		}
		else
		{
			++iter;
			++iCheckIndex;
		}
	}

	if(m_pSelectedSpecturm != nullptr)
		UpdateSelected_SpectrumFormTag(m_pSelectedSpecturm->Get_MyTag());
}

CSpectrum::SPECTRUM_DESC* CSpectrum_Controller::Get_SpectrumDesc(_wstring& EffectSpectrumTag)
{
    auto iter = m_tSpectrumDesc.find(EffectSpectrumTag);

    if (iter == m_tSpectrumDesc.end())
        return nullptr;

    return &iter->second;
}

void CSpectrum_Controller::Set_SpectrumDesc(_wstring&SpectrumTag, CSpectrum::SPECTRUM_DESC& TrailDesc)
{
	CSpectrum::SPECTRUM_DESC Desc = {};
    Desc = TrailDesc;

    m_tSpectrumDesc.emplace(SpectrumTag, Desc);
}

void CSpectrum_Controller::Set_VBSpectrumDesc(_wstring& SpectrumTag, CVIBuffer_Spectrum::VB_SPECTRUM_DESC& SpectrumVBDesc)
{
	CVIBuffer_Spectrum::VB_SPECTRUM_DESC Desc = {};
	Desc = SpectrumVBDesc;

	m_tVBDesc.emplace(SpectrumTag, Desc);
}


void CSpectrum_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterEffectSpectrumDesc = m_tSpectrumDesc.find(DescTag);

    if (iterEffectSpectrumDesc != m_tSpectrumDesc.end())
    {
		m_pGameInstance->Remove_Prototype(ENUM_CLASS(LEVEL::EFFECT), iterEffectSpectrumDesc->second.strVIBufferTag);

        m_tSpectrumDesc.erase(iterEffectSpectrumDesc);
    }

	auto iterVBDesc = m_tVBDesc.find(DescTag);

	if (iterVBDesc != m_tVBDesc.end())
	{
		m_tVBDesc.erase(iterVBDesc);
	}

    //초기화
    m_bSelectedSpectrum = false;
    m_pSelectedSpectrumDesc = nullptr;
	m_pSelectedSpectrumVBDesc = nullptr;
}

void CSpectrum_Controller::Save_SelectedSpectrum_To_Json()
{
	if (ImGui::Button("Save Spectrum"))
	{
		IGFD::FileDialogConfig config;

		config.path = "../../Client/Bin/Resource/Effect/Spectrum/";
		config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

		ImGuiFileDialog::Instance()->OpenDialog("Save Spectrum", "Export", ".json", config);
	}
	if (ImGuiFileDialog::Instance()->Display("Save Spectrum", ImGuiWindowFlags_NoCollapse))
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
			
			/// VB저장
			_string SpectrumVBPath = {};
			SpectrumVBPath = strFolderPath;
			SpectrumVBPath += "/SpectrumVB/";
			SpectrumVBPath += WStringToString(m_pSelectedSpecturm->Get_MyTag());
			SpectrumVBPath += ".json";

			ofstream VBjsonStream(SpectrumVBPath);
			json SpectrumVBJson;

			Spectrum_VB_To_Json(SpectrumVBJson);

			VBjsonStream << SpectrumVBJson.dump(2);
			VBjsonStream.close();

			/// OB저장

			_string SpectrumOBPath = {};
			SpectrumOBPath = strFolderPath;
			SpectrumOBPath += "/SpectrumOB/";
			SpectrumOBPath += WStringToString(m_pSelectedSpecturm->Get_MyTag());
			SpectrumOBPath += ".json";

			ofstream OBjsonStream(SpectrumOBPath);
			json SpectrumOBJson;

			Spectrum_OB_To_Json(SpectrumOBJson);

			OBjsonStream << SpectrumOBJson.dump(2);
			OBjsonStream.close();
		}

		ImGuiFileDialog::Instance()->Close();
	}
}


void CSpectrum_Controller::Spectrum_VB_To_Json(json& SpectrumVBJson)
{
	SpectrumVBJson["MaxSamples"] = m_pSelectedSpectrumVBDesc->MaxSamples;
	SpectrumVBJson["Size"] = m_pSelectedSpectrumVBDesc->fSize;
}

void CSpectrum_Controller::Spectrum_OB_To_Json(json& SpectrumJson)
{
	SpectrumJson["MyTag"] = WStringToString(m_pSelectedSpectrumDesc->strMyTag);
	SpectrumJson["MyType"] = m_pSelectedSpectrumDesc->eMyType;

	SpectrumJson["TextureTag"] = WStringToString(m_pSelectedSpectrumDesc->strTextureTag);
	SpectrumJson["VIBufferTag"] = WStringToString(m_pSelectedSpectrumDesc->strVIBufferTag);
	SpectrumJson["ColorTexturTag"] = WStringToString(m_pSelectedSpectrumDesc->strColorTextureTag);

	SpectrumJson["ShaderPass"] = m_pSelectedSpectrumDesc->iShaderPass;
	SpectrumJson["Generation"] = m_pSelectedSpectrumDesc->fGeneration;
	SpectrumJson["LifeTime"] = m_pSelectedSpectrumDesc->fLifeTime;
}

CSpectrum_Controller* CSpectrum_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSpectrum_Controller* pInstance = new CSpectrum_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CSpectrum_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSpectrum_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

    m_Textures.clear();

}