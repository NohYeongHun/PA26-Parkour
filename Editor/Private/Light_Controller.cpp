#include "EditorPch.h"
#include "Light_Controller.h"

CLight_Controller::CLight_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CLight_Controller::Initialize()
{
	//Json 폴더 목록 읽어서 데칼 텍스처 바인딩해서 미리 매니저에 바인딩 해놔야함.
	Load_AllJsonLightDataFromFolder("../../Client/Bin/Resource/Effect/Prefabs/Common/Light");

    return S_OK;
}

void CLight_Controller::Update()
{
    Light_Tab();

	if (m_IsLightDataFlag)
		LightData_Base_Tab();
}

void CLight_Controller::Render()
{

}

void CLight_Controller::Load_AllJsonLightDataFromFolder(const _string& strFolderPath)
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
				_tchar LightDataTag[MAX_PATH] = {};

				Load_LightData_FromJson(filePath);
			}
		}
	}
}

void CLight_Controller::Load_LightData_FromJson(const _string& strFilePath)
{
	ifstream JsonStream(strFilePath.c_str());

	if (!JsonStream.is_open())
		return;

	json LightDataJson;
	JsonStream >> LightDataJson;
	JsonStream.close();

	LIGHT_DESC LightDesc = {};

	_wstring LightDataTag = {};
	_float4 vColor = {};

	if (LightDataJson.contains("LightTag"))
		LightDataTag = StringToWString(LightDataJson["LightTag"].get<_string>());

	if (LightDataJson.contains("BaseColor") && LightDataJson["BaseColor"].is_array())
	{
		json Color = LightDataJson["BaseColor"];
		vColor.x = Color[0].get<_float>();
		vColor.y = Color[1].get<_float>();
		vColor.z = Color[2].get<_float>();
		vColor.w = Color[3].get<_float>();
	}

	LightDesc.eType = LIGHT_DESC::POINT;
	LightDesc.vPosition = _float4(0.f, 0.f, 0.f, 1.f);
	LightDesc.fRange = 1.f;
	LightDesc.vDiffuse = vColor;

	m_pGameInstance->Add_Light(LightDataTag, LightDesc);
	
	m_pGameInstance->Set_LightActive(LightDataTag, false);

	m_LightData.push_back(WStringToString(LightDataTag));
}

void CLight_Controller::Light_Tab()
{
    if (m_bSelectedLight)
    {
        if (ImGui::Begin("Light Info"))
        {
                if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
                {
					vector<const char*> vLightItems;
					vLightItems.reserve(m_LightData.size());

					for (auto& s : m_LightData)
						vLightItems.push_back(s.c_str());

					//데칼 리스트 박스 
					if (ImGui::ListBox("Light Data", &m_iSelectedDacalData, vLightItems.data(), int(vLightItems.size()), int(vLightItems.size() + 2)))
					{
						m_pSelectedLightDesc->wstrLightTag = StringToWString(m_LightData[m_iSelectedDacalData]);
					}

					if(ImGui::Button("Create Base"))
					{
						m_IsLightDataFlag = true;
					}

                    ImGui::Text("LifeTime");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##LightLifeTimeX", &(m_pSelectedLightDesc->vLifeTime.x));
					ImGui::PushItemWidth(60);
					ImGui::InputFloat("##LightLifeTimeY", &(m_pSelectedLightDesc->vLifeTime.y));
                    ImGui::PopItemWidth();

					ImGui::Text("Speed");
					ImGui::PushItemWidth(60);
					ImGui::InputFloat("##LightSpeed", &(m_pSelectedLightDesc->fSpeed));
					ImGui::PopItemWidth();

					ImGui::Text("Range");
					ImGui::PushItemWidth(60);
					ImGui::InputFloat("##LightRangeX", &(m_pSelectedLightDesc->vRange.x));
					ImGui::PushItemWidth(60);
					ImGui::InputFloat("##LightRangeY", &(m_pSelectedLightDesc->vRange.y));
					ImGui::PopItemWidth();

					ImGui::Text("Ambient");
					ImGui::PushItemWidth(60);
					ImGui::InputFloat("##LightAmbient", &(m_pSelectedLightDesc->fAmbient));
					ImGui::PopItemWidth();

                    if (ImGui::ColorEdit4("Color", m_fColor, 
						ImGuiColorEditFlags_NoOptions          // 설정 메뉴 비활성화 (HSV 등 변환 방지)
						| ImGuiColorEditFlags_NoInputs         // 텍스트 입력 비활성 (정확히 선택한 색 유지)
						| ImGuiColorEditFlags_DisplayRGB       // 항상 RGB로 표시
						| ImGuiColorEditFlags_InputRGB         // RGB 입력값으로 유지
						| ImGuiColorEditFlags_AlphaBar         // 알파 바 표시
						| ImGuiColorEditFlags_AlphaPreview))   // 알파 미리보기
                    {
                        m_pSelectedLightDesc->vColor = _float4(m_fColor[0], m_fColor[1], m_fColor[2], m_fColor[3]);
                    }

                    ImGui::Separator();
                  
                }

            ImGui::End();
        }
    }

}

void CLight_Controller::Light_Base_Tab(CEffect_Light::LIGHT_DESC& tEffectLightDesc, _bool& IsCreate)
{
	 if (ImGui::Begin("Light Base"))
    {
		 vector<const char*> vLightItems;
		 vLightItems.reserve(m_LightData.size());

		 for (auto& s : m_LightData)
			 vLightItems.push_back(s.c_str());

        // 리스트 박스 
        if (ImGui::ListBox("Light Data", &m_iSelectedDacalData, vLightItems.data(), int(vLightItems.size()), int(vLightItems.size() + 2)))
        {
			m_bDataFlag = true;
        }

        if (m_bTagFlag && m_bDataFlag) //이펙트 컨트롤러가 설정해준 이름값이 있고, 선택한 데이터가 있어야지만 생성할 수 있게.
        {
            if (ImGui::Button("Create"))
            {
                //프리팹에게 Desc 전달 -> 프리팹이 Desc로 클론 진행

                _tchar LightTag[MAX_PATH] = {};
				_tchar LightDataTag[MAX_PATH] = {};
                CEffect_Light::LIGHT_DESC LightDesc{};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_LightTag, strlen(m_LightTag), LightTag, MAX_PATH);


                //이펙트 이름 및 클론할 컴포넌트 이름들
				LightDesc.strMyTag = LightTag;
				LightDesc.eMyType = EFFECT_TYPE::LIGHT;
				LightDesc.wstrLightTag = StringToWString(m_LightData[m_iSelectedDacalData]);

                //이펙트(오브젝트)가 가질 디폴트 설정값.
				LightDesc.vLifeTime = _float2(0.f, 5.f);
				LightDesc.vColor = _float4(1.f, 1.f, 1.f, 1.f);

                //매쉬이펙트 와 매쉬VB태그를 맞춰야할지는 고민해보자.
                m_tLightDesc.emplace(LightTag, LightDesc);

				tEffectLightDesc = LightDesc;

                //생성됐음을 이펙트 컨트롤러에게 전달
                IsCreate = true;

                //초기화
				m_LightTag[0] = _T('\0');
                m_bTagFlag = false;
            }
        }

		if (ImGui::Button("Light Data Create"))
			m_IsLightDataFlag = true;

		if (m_IsLightDataFlag)
			LightData_Base_Tab();
       
        ImGui::End();
    }
}

void CLight_Controller::LightData_Base_Tab()
{
    if (ImGui::Begin("Light Base"))
    {
		if (ImGui::InputText("LightData", m_LightDataTag, IM_ARRAYSIZE(m_LightDataTag), ImGuiInputTextFlags_EnterReturnsTrue))
			m_bDataTagFlag = true;

		ImGui::ColorEdit4("Base Color", m_vBaseColor,
			ImGuiColorEditFlags_NoOptions          // 설정 메뉴 비활성화 (HSV 등 변환 방지)
			| ImGuiColorEditFlags_NoInputs         // 텍스트 입력 비활성 (정확히 선택한 색 유지)
			| ImGuiColorEditFlags_DisplayRGB       // 항상 RGB로 표시
			| ImGuiColorEditFlags_InputRGB         // RGB 입력값으로 유지
			| ImGuiColorEditFlags_AlphaBar         // 알파 바 표시
			| ImGuiColorEditFlags_AlphaPreview);   // 알파 미리보기

        if (m_bDataTagFlag)
        {
            if (ImGui::Button("Create"))
            {
                _tchar LightDataTag[MAX_PATH] = {};

				LIGHT_DATADESC DataDesc = {};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_LightDataTag, strlen(m_LightDataTag), LightDataTag, MAX_PATH);

				LIGHT_DESC LightDesc = {};
				LightDesc.vDiffuse = _float4(m_vBaseColor);
				LightDesc.fRange = 1.f;
				LightDesc.eType = LIGHT_DESC::POINT;
				LightDesc.vPosition = _float4(0.f, 0.f, 0.f, 1.f);
				
				m_pGameInstance->Add_Light(LightDataTag, LightDesc);

				DataDesc.wstrLightTag = LightDataTag;

				LightData_To_Json(&DataDesc);

				m_LightData.push_back(string(m_LightDataTag));

				m_LightDataTag[0] = _T('\0');
				m_bDataTagFlag = false;

				m_IsLightDataFlag = false;

				m_vBaseColor[0] = 1.f;
				m_vBaseColor[1] = 1.f;
				m_vBaseColor[2] = 1.f;
				m_vBaseColor[3] = 1.f;
            }
        }
        ImGui::End();
    }
}

void CLight_Controller::UpdateSelected_LightFormTag(_wstring LightTag)
{

    auto iterLightDesc = m_tLightDesc.find(LightTag);
    
    if (iterLightDesc == m_tLightDesc.end())
    {
        m_pSelectedLightDesc = nullptr;
    }
    else
    {
        m_pSelectedLightDesc = &iterLightDesc->second;
    }

    if (m_pSelectedLightDesc != nullptr)
    {
        m_bSelectedLight = true;

        m_fColor[0] = m_pSelectedLightDesc->vColor.x;
        m_fColor[1] = m_pSelectedLightDesc->vColor.y;
        m_fColor[2] = m_pSelectedLightDesc->vColor.z;
        m_fColor[3] = m_pSelectedLightDesc->vColor.w;
    }
}

CEffect_Light::LIGHT_DESC* CLight_Controller::Get_LightDesc(_wstring& LightTag)
{
    auto iter = m_tLightDesc.find(LightTag);
    
    if (iter == m_tLightDesc.end())
        return nullptr;

    return &iter->second;
}

void CLight_Controller::Set_LightDesc(_wstring& LightTag, CEffect_Light::LIGHT_DESC& LightDesc)
{
	CEffect_Light::LIGHT_DESC Desc = {};
    Desc = LightDesc;

    m_tLightDesc.emplace(LightTag, Desc);
}

void CLight_Controller::Set_LightTag(const _char* szLightTag)
{
    strcat_s(m_LightTag, szLightTag);

    m_bTagFlag = true;
}
void CLight_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterLightDesc = m_tLightDesc.find(DescTag);

    if (iterLightDesc != m_tLightDesc.end())
    {
        m_tLightDesc.erase(iterLightDesc);
    }


    m_iSelectedLight = 0;
    m_bSelectedLight = false;
    m_pSelectedLightDesc = nullptr;
   
    m_fColor[0] = 1.f;
    m_fColor[1] = 1.f;
    m_fColor[2] = 1.f;
    m_fColor[3] = 1.f;
}

void CLight_Controller::LightData_To_Json(LIGHT_DATADESC* pDesc)
{
	_string strFilePath = "../../Client/Bin/Resource/Effect/Prefabs/Common/Light/";
	_string strDacalDataTag = WStringToString(pDesc->wstrLightTag);

	strFilePath += strDacalDataTag;
	strFilePath += ".json";

	ofstream jsonStream(strFilePath.c_str());

	json LightDataJson;

	LightDataJson["LightTag"] = strDacalDataTag;

	json BaseColor = json::array();
	BaseColor.push_back(m_vBaseColor[0]);
	BaseColor.push_back(m_vBaseColor[1]);
	BaseColor.push_back(m_vBaseColor[2]);
	BaseColor.push_back(m_vBaseColor[3]);
	LightDataJson["BaseColor"] = BaseColor;

	jsonStream << LightDataJson.dump(2);
	jsonStream.close();
}


CLight_Controller* CLight_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLight_Controller* pInstance = new CLight_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CLight_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLight_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
}