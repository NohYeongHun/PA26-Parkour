#include "EditorPch.h"
#include "Decal_Controller.h"

CDecal_Controller::CDecal_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CDecal_Controller::Initialize()
{
	//Json 폴더 목록 읽어서 데칼 텍스처 바인딩해서 미리 매니저에 바인딩 해놔야함.
	Load_AllJsonDecalDataFromFolder("../../Client/Bin/Resource/Effect/Prefabs/Common/Decal");

	Load_AllDiffuseTextureFromFolder("../../Client/Bin/Resource/Effect/Decal/Diffuse");
	Load_AllMaskTextureFromFolder("../../Client/Bin/Resource/Effect/Decal/Mask");
	Load_AllNormalTextureFromFolder("../../Client/Bin/Resource/Effect/Decal/Normal");

    return S_OK;
}

void CDecal_Controller::Update()
{
    Decal_Tab();

	if (m_IsDecalDataFlag)
		DecalData_Base_Tab();
}

void CDecal_Controller::Render()
{

}

void CDecal_Controller::Load_AllJsonDecalDataFromFolder(const _string& strFolderPath)
{
	for (const auto& entry : filesystem::directory_iterator(strFolderPath))
	{
		if (entry.is_regular_file())
		{
			_string filePath = entry.path().string();
			_string fileName = entry.path().filename().string();
			_string extension = entry.path().extension().string();

			if (extension == ".json")
			{
				_tchar DecalDataTag[MAX_PATH] = {};

				Load_DecalData_FromJson(filePath);
			}
		}
	}
}

void CDecal_Controller::Load_DecalData_FromJson(const _string& strFilePath)
{
	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json DecalDataJson;
	JsonStream >> DecalDataJson;
	JsonStream.close();

	_wstring DecalDataTag = {};
	_int iTextureCount = {};
	_float3 EmissiveLuminance = {};

	if (DecalDataJson.contains("DecalTag"))
		DecalDataTag = StringToWString(DecalDataJson["DecalTag"].get<_string>());

	if (DecalDataJson.contains("TextureCount"))
		iTextureCount = DecalDataJson["TextureCount"].get<_int>();

	if (DecalDataJson.contains("EmissiveLuminance") && DecalDataJson["EmissiveLuminance"].is_array())
	{
		json Emissive = DecalDataJson["EmissiveLuminance"];
		EmissiveLuminance.x = Emissive[0].get<_float>();
		EmissiveLuminance.y = Emissive[1].get<_float>();
		EmissiveLuminance.z = Emissive[2].get<_float>();
	}

	const _tchar* DecalTexturePath[ENUM_CLASS(TEXTURETYPE::END)] = {};

	if (DecalDataJson.contains("Textures") && DecalDataJson["Textures"].is_array())
	{
		for (size_t i = 0; i < iTextureCount; i++)
		{
			TEXTURE_DESC Desc = {};
			_int iType = {};
			_tchar TextPath[MAX_PATH] = {};

			json Texture = DecalDataJson["Textures"][i];

			if (Texture.contains("Texture_Type"))
				Desc.eType = static_cast<TEXTURETYPE>(iType = Texture["Texture_Type"].get<_int>());

			if (Texture.contains("Texture_Path"))
				Desc.TexturePath = StringToWString(Texture["Texture_Path"].get<_string>());
			//만든 JSON 읽어서, 미리 데칼 매니저에 바인딩 + 데이터 이름 들고있게 해서 데칼 만들 때 사용하기.
			
			wcscpy_s(TextPath, Desc.TexturePath.c_str());

			DecalTexturePath[iType] = TextPath;

			//굳이 들고 있어야할 이유가 없을듯. 이거 대신에
			//데칼 클래스의 정보를 들고있어야함. 그래야 추후에 이펙트 프리팹에 데칼을 추가할 수 있을거 같음. 수정해줘야할거 같음.
			//필요한건 데칼데이터 정보 x -> 이건 그냥 구조체로 json 내보내기용으로 묶을 구조체였을 뿐임. 컨트롤러에서 들고있어야할 이유 x
			//데칼 클래스의 정보 o -> 프리팹에 추가해야할 정보 제공 + json으로 내보내기.

		}
	}

	m_pGameInstance->Add_Decal(DecalDataTag, DecalTexturePath, EmissiveLuminance);

	m_DecalData.push_back(WStringToString(DecalDataTag));
}

void CDecal_Controller::Load_AllDiffuseTextureFromFolder(const _string& strFolderPath)
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
				DECAL_DIFFUSE Desc{};
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


				_string strPath = "../../Client/Bin/Resource/Effect/Prefabs/Common/Decal/Texture/";
				strPath += strTextureTag;
				strPath += ".png";

				Desc.pTexture = pTexture;
				Desc.strTexturePath = strPath;
				//Safe_AddRef(pTexture);

				m_DiffuseTextures.push_back(Desc);
			}
		}
	}
}

void CDecal_Controller::Load_AllNormalTextureFromFolder(const _string& strFolderPath)
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
				DECAL_NORMAL Desc{};
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


				//클라중심
				_string strPath = "../../Client/Bin/Resource/Effect/Prefabs/Common/Decal/Texture/";
				strPath += strTextureTag;
				strPath += ".png";

				Desc.pTexture = pTexture;
				Desc.strTexturePath = strPath;
				//Safe_AddRef(pTexture);

				m_NormalTextures.push_back(Desc);
			}
		}
	}
}

void CDecal_Controller::Load_AllMaskTextureFromFolder(const _string& strFolderPath)
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
				DECAL_MASK Desc{};
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


				//클라중심
				_string strPath = "../../Client/Bin/Resource/Effect/Prefabs/Common/Decal/Texture/";
				strPath += strTextureTag;
				strPath += ".png";

				Desc.pTexture = pTexture;
				Desc.strTexturePath = strPath;
				//Safe_AddRef(pTexture);

				m_MaskTextures.push_back(Desc);
			}
		}
	}
}

void CDecal_Controller::Decal_Tab()
{
    if (m_bSelectedDecal)
    {
        if (ImGui::Begin("Decal Info"))
        {
                if (ImGui::CollapsingHeader("Decal", ImGuiTreeNodeFlags_DefaultOpen))
                {
					vector<const char*> vDecalItems;
					vDecalItems.reserve(m_DecalData.size());

					for (auto& s : m_DecalData)
						vDecalItems.push_back(s.c_str());

					//데칼 리스트 박스 
					if (ImGui::ListBox("Decal Data", &m_iSelectedDacalData, vDecalItems.data(), int(vDecalItems.size()), int(vDecalItems.size() + 2)))
					{
						m_pSelectedDecalDesc->wstrDecalTag = StringToWString(m_DecalData[m_iSelectedDacalData]);
					}

					if(ImGui::Button("Create Base"))
					{
						m_IsDecalDataFlag = true;
					}

                    ImGui::Checkbox("Root", &(m_pSelectedDecalDesc->IsRootOn));

                    ImGui::Text("LifeTime");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##DecalLifeTime", &(m_pSelectedDecalDesc->LifeTime));
                    ImGui::PopItemWidth();

					ImGui::Text("BlendTime");
					ImGui::PushItemWidth(60);
					ImGui::InputFloat("##DecalBlendTime", &(m_pSelectedDecalDesc->fBlendTime));
					ImGui::PopItemWidth();

					ImGui::Text("EmissiveIntensity");
					ImGui::PushItemWidth(60);
					ImGui::InputFloat("##DecalEmissiveIntensity", &(m_pSelectedDecalDesc->fEmissiveIntensity));
					ImGui::PopItemWidth();

                    if (ImGui::ColorEdit4("Color", m_fColor, 
						ImGuiColorEditFlags_NoOptions          // 설정 메뉴 비활성화 (HSV 등 변환 방지)
						| ImGuiColorEditFlags_NoInputs         // 텍스트 입력 비활성 (정확히 선택한 색 유지)
						| ImGuiColorEditFlags_DisplayRGB       // 항상 RGB로 표시
						| ImGuiColorEditFlags_InputRGB         // RGB 입력값으로 유지
						| ImGuiColorEditFlags_AlphaBar         // 알파 바 표시
						| ImGuiColorEditFlags_AlphaPreview))   // 알파 미리보기
                    {
                        m_pSelectedDecalDesc->vColor = _float4(m_fColor[0], m_fColor[1], m_fColor[2], m_fColor[3]);
                    }

                    ImGui::Separator();
                  
                }

            ImGui::End();
        }
    }

}

void CDecal_Controller::Decal_Base_Tab(CEffect_Decal::DECAL_DESC& tEffectDecalDesc, _bool& IsCreate)
{
	 if (ImGui::Begin("Decal Base"))
    {
		 vector<const char*> vDecalItems;
		 vDecalItems.reserve(m_DecalData.size());

		 for (auto& s : m_DecalData)
			 vDecalItems.push_back(s.c_str());

        //데칼 리스트 박스 
        if (ImGui::ListBox("Decal Data", &m_iSelectedDacalData, vDecalItems.data(), int(vDecalItems.size()), int(vDecalItems.size() + 2)))
        {
			m_bDataFlag = true;
        }

        if (m_bTagFlag && m_bDataFlag) //이펙트 컨트롤러가 설정해준 이름값이 있고, 선택한 데이터가 있어야지만 생성할 수 있게.
        {
            if (ImGui::Button("Create"))
            {
                //프리팹에게 Desc 전달 -> 프리팹이 Desc로 클론 진행

                _tchar DecalTag[MAX_PATH] = {};
				_tchar DecalDataTag[MAX_PATH] = {};
                CEffect_Decal::DECAL_DESC DecalDesc{};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_DecalTag, strlen(m_DecalTag), DecalTag, MAX_PATH);


                //이펙트 매쉬 이름 및 클론할 컴포넌트 이름들
				DecalDesc.strMyTag = DecalTag;
				DecalDesc.eMyType = EFFECT_TYPE::DECAL;
				DecalDesc.wstrDecalTag = StringToWString(m_DecalData[m_iSelectedDacalData]);

                //이펙트매쉬(오브젝트)가 가질 디폴트 설정값.
				DecalDesc.LifeTime = 5.f;
				DecalDesc.vColor = _float4(1.f, 1.f, 1.f, 1.f);

                _char szDatPath[MAX_PATH] = {};

                //매쉬이펙트 와 매쉬VB태그를 맞춰야할지는 고민해보자.
                m_tDecalDesc.emplace(DecalTag, DecalDesc);

				tEffectDecalDesc = DecalDesc;

                //생성됐음을 이펙트 컨트롤러에게 전달
                IsCreate = true;

                //초기화
				m_DecalTag[0] = _T('\0');
                m_bTagFlag = false;
            }
        }

		if (ImGui::Button("Decal Data Create"))
			m_IsDecalDataFlag = true;

		if (m_IsDecalDataFlag)
			DecalData_Base_Tab();
       
        ImGui::End();
    }
}

void CDecal_Controller::DecalData_Base_Tab()
{
    if (ImGui::Begin("Decal Base"))
    {
		if (ImGui::InputText("DecalData", m_DecalDataTag, IM_ARRAYSIZE(m_DecalDataTag), ImGuiInputTextFlags_EnterReturnsTrue))
			m_bDataTagFlag = true;

		if (ImGui::Checkbox("MaskEmissive", &m_bMaskEmissive))
		{
			if (m_bMaskEmissive)
				m_bDiffuseEmissive = false;
		}
		if (ImGui::Checkbox("DiffuseEmissive", &m_bDiffuseEmissive))
		{
			if (m_bDiffuseEmissive)
				m_bMaskEmissive = false;
		}

		ImGui::Text("EmissiveLuminance");
		ImGui::PushItemWidth(60);
		ImGui::InputFloat("##EmissiveLuminanceX", &(m_EmissiveLuminance.x));
		ImGui::SameLine();
		ImGui::InputFloat("##EmissiveLuminancY", &(m_EmissiveLuminance.y));
		ImGui::SameLine();
		ImGui::InputFloat("##EmissiveLuminanceZ", &(m_EmissiveLuminance.z));
		ImGui::PopItemWidth();
	

		//디퓨즈 텍스처
        if (ImGui::BeginCombo("Diffuse", "")) {
            for (size_t i = 0; i < m_DiffuseTextures.size(); i++)
            {
                bool IsSelected = (m_iSelectedDiffuseTexture == i);
                if (ImGui::Selectable(m_DiffuseTextures[i].szName, IsSelected))
					m_iSelectedDiffuseTexture = i;

                if (IsSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();
        if (m_iSelectedDiffuseTexture >= 0) {
            ImGui::Image((ImTextureID)m_DiffuseTextures[m_iSelectedDiffuseTexture].pTexture->Get_SRV(0), ImVec2(256, 256));
        }

		//노말 텍스처
		if (ImGui::BeginCombo("Noraml", "")) {
			for (size_t i = 0; i < m_NormalTextures.size(); i++)
			{
				bool IsSelected = (m_iSelectedNormalTexture == i);
				if (ImGui::Selectable(m_NormalTextures[i].szName, IsSelected))
					m_iSelectedNormalTexture = i;

				if (IsSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();
		if (m_iSelectedNormalTexture >= 0) {
			ImGui::Image((ImTextureID)m_NormalTextures[m_iSelectedNormalTexture].pTexture->Get_SRV(0), ImVec2(256, 256));
		}


		//마스크 텍스처
		if (ImGui::BeginCombo("Mask", "")) {
			for (size_t i = 0; i < m_MaskTextures.size(); i++)
			{
				bool IsSelected = (m_iSelectedMaskTexture == i);
				if (ImGui::Selectable(m_MaskTextures[i].szName, IsSelected))
					m_iSelectedMaskTexture = i;

				if (IsSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();
		if (m_iSelectedMaskTexture >= 0) {
			ImGui::Image((ImTextureID)m_MaskTextures[m_iSelectedMaskTexture].pTexture->Get_SRV(0), ImVec2(256, 256));
		}


        if (m_bDataTagFlag)
        {
            if (ImGui::Button("Create"))
            {
                _tchar DecalDataTag[MAX_PATH] = {};
				_tchar szDiffuse[MAX_PATH] = {};
				_tchar szMask[MAX_PATH] = {};
				_tchar szNormal[MAX_PATH] = {};
				_tchar szEmissive[MAX_PATH] = {};
				const _tchar* DecalTexturePath[ENUM_CLASS(TEXTURETYPE::END)] = {};
				DECAL_DATADESC DataDesc = {};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_DecalDataTag, strlen(m_DecalDataTag), DecalDataTag, MAX_PATH);

				DataDesc.vEmissiveLuminance = m_EmissiveLuminance;

				if (m_iSelectedDiffuseTexture >= 0)
				{
					TEXTURE_DESC TextureDesc = {};
					TextureDesc.eType = TEXTURETYPE::DIFFUSE;
					TextureDesc.TexturePath =  StringToWString(m_DiffuseTextures[m_iSelectedDiffuseTexture].strTexturePath);

					DataDesc.TextureDesc.push_back(TextureDesc);

					wcscpy_s(szDiffuse, TextureDesc.TexturePath.c_str());
					DecalTexturePath[ENUM_CLASS(TEXTURETYPE::DIFFUSE)] = szDiffuse;
				}

				if (m_iSelectedMaskTexture >= 0)
				{
					TEXTURE_DESC TextureDesc = {};
					TextureDesc.eType = TEXTURETYPE::MASK;
					TextureDesc.TexturePath = StringToWString(m_MaskTextures[m_iSelectedMaskTexture].strTexturePath);

					DataDesc.TextureDesc.push_back(TextureDesc);

					wcscpy_s(szMask, TextureDesc.TexturePath.c_str());
					DecalTexturePath[ENUM_CLASS(TEXTURETYPE::MASK)] = szMask;
				}

				if (m_iSelectedNormalTexture >= 0)
				{
					TEXTURE_DESC TextureDesc = {};
					TextureDesc.eType = TEXTURETYPE::NORMAL;
					TextureDesc.TexturePath = StringToWString(m_NormalTextures[m_iSelectedNormalTexture].strTexturePath);

					DataDesc.TextureDesc.push_back(TextureDesc);

					wcscpy_s(szNormal, TextureDesc.TexturePath.c_str());
					DecalTexturePath[ENUM_CLASS(TEXTURETYPE::NORMAL)] = szNormal;
				}

				if (m_bMaskEmissive || m_bDiffuseEmissive)
				{
					TEXTURE_DESC TextureDesc = {};
					TextureDesc.eType = TEXTURETYPE::EMISSIVE;

					if(m_bMaskEmissive)
						TextureDesc.TexturePath = StringToWString(m_MaskTextures[m_iSelectedMaskTexture].strTexturePath);
					else
						TextureDesc.TexturePath = StringToWString(m_DiffuseTextures[m_iSelectedDiffuseTexture].strTexturePath);

					DataDesc.TextureDesc.push_back(TextureDesc);

					wcscpy_s(szEmissive, TextureDesc.TexturePath.c_str());
					DecalTexturePath[ENUM_CLASS(TEXTURETYPE::EMISSIVE)] = szEmissive;
				}

				m_pGameInstance->Add_Decal(DecalDataTag, DecalTexturePath, m_EmissiveLuminance);

				DataDesc.wstrDecalDataTag = DecalDataTag;

				//푸쉬백 해서 꺼낼쓸건데, 여기서 새로 만든 얘는 json으로 저장하는 것도 여기서 작업해줘야할거 같음.
				//로딩때 json 읽어서 따로 매니저에 미리 등록해놓을거임. 등록된 얘는 Data에 이름 등록해서 데칼 만들때 이름꺼내서 쓸 수 있게 해주고자 함.

				DecalData_To_Json(&DataDesc);

				m_DecalData.push_back(string(m_DecalDataTag));

				m_DecalDataTag[0] = _T('\0');
				m_bDataTagFlag = false;

				m_IsDecalDataFlag = false;

				m_EmissiveLuminance = _float3(0.f, 0.f, 0.f);
				m_bDiffuseEmissive = false;
				m_bMaskEmissive = false;

				m_iSelectedNormalTexture = -1;
				m_iSelectedMaskTexture = -1;
				m_iSelectedDiffuseTexture = -1;
            }
        }
        ImGui::End();
    }
}

void CDecal_Controller::UpdateSelected_DecalFormTag(_wstring DecalTag)
{

    auto iterDecalDesc = m_tDecalDesc.find(DecalTag);
    
    if (iterDecalDesc == m_tDecalDesc.end())
    {
        m_pSelectedDecalDesc = nullptr;
    }
    else
    {
        m_pSelectedDecalDesc = &iterDecalDesc->second;
    }


    if (m_pSelectedDecalDesc != nullptr)
    {
        m_bSelectedDecal = true;

        m_fColor[0] = m_pSelectedDecalDesc->vColor.x;
        m_fColor[1] = m_pSelectedDecalDesc->vColor.y;
        m_fColor[2] = m_pSelectedDecalDesc->vColor.z;
        m_fColor[3] = m_pSelectedDecalDesc->vColor.w;
    }
}

CEffect_Decal::DECAL_DESC* CDecal_Controller::Get_DecalDesc(_wstring& DecalTag)
{
    auto iter = m_tDecalDesc.find(DecalTag);
    
    if (iter == m_tDecalDesc.end())
        return nullptr;

    return &iter->second;
}

void CDecal_Controller::Set_DecalDesc(_wstring& DecalTag, CEffect_Decal::DECAL_DESC& DecalDesc)
{
	CEffect_Decal::DECAL_DESC Desc = {};
    Desc = DecalDesc;

    m_tDecalDesc.emplace(DecalTag, Desc);
}

void CDecal_Controller::Set_DecalTag(const _char* szDecalTag)
{
    strcat_s(m_DecalTag, szDecalTag);

    m_bTagFlag = true;
}
void CDecal_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterDecalDesc = m_tDecalDesc.find(DescTag);

    if (iterDecalDesc != m_tDecalDesc.end())
    {
        m_tDecalDesc.erase(iterDecalDesc);
    }


    m_iSelectedDecal = 0;
    m_bSelectedDecal = false;
    m_pSelectedDecalDesc = nullptr;
   
    m_fColor[0] = 1.f;
    m_fColor[1] = 1.f;
    m_fColor[2] = 1.f;
    m_fColor[3] = 1.f;
}

void CDecal_Controller::DecalData_To_Json(DECAL_DATADESC* pDesc)
{
	_string strFilePath = "../../Client/Bin/Resource/Effect/Prefabs/Common/Decal/";
	_string strDacalDataTag = WStringToString(pDesc->wstrDecalDataTag);

	strFilePath += strDacalDataTag;
	strFilePath += ".json";

	ofstream jsonStream(strFilePath.c_str());

	json DecalDataJson;

	DecalDataJson["DecalTag"] = strDacalDataTag;

	DecalDataJson["TextureCount"] = pDesc->TextureDesc.size();

	json EmissiveLuminanceJson = json::array();
	EmissiveLuminanceJson.push_back(m_EmissiveLuminance.x);
	EmissiveLuminanceJson.push_back(m_EmissiveLuminance.y);
	EmissiveLuminanceJson.push_back(m_EmissiveLuminance.z);
	DecalDataJson["EmissiveLuminance"] = EmissiveLuminanceJson;

	json TextureDataJson = json::array();

	for (size_t i = 0; i < pDesc->TextureDesc.size(); i++)
	{
		json Texture;

		Texture["Texture_Type"] = pDesc->TextureDesc[i].eType;
		Texture["Texture_Path"] = WStringToString(pDesc->TextureDesc[i].TexturePath);

		TextureDataJson.push_back(Texture);
	}

	DecalDataJson["Textures"] = TextureDataJson;

	jsonStream << DecalDataJson.dump(2);
	jsonStream.close();
}


CDecal_Controller* CDecal_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CDecal_Controller* pInstance = new CDecal_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CDecal_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CDecal_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
}