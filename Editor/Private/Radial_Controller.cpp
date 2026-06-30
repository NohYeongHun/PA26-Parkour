#include "EditorPch.h"
#include "Radial_Controller.h"

CRadial_Controller::CRadial_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::GetInstance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

HRESULT CRadial_Controller::Initialize()
{
	
    return S_OK;
}

void CRadial_Controller::Update()
{
    Radial_Tab();

}

void CRadial_Controller::Render()
{

}


void CRadial_Controller::Radial_Tab()
{
    if (m_bSelectedRadial)
    {
        if (ImGui::Begin("Radial Info"))
        {
           
                if (ImGui::CollapsingHeader("Radial", ImGuiTreeNodeFlags_DefaultOpen))
                {
					ImGui::Checkbox("Position", &(m_pSelectedRadialDesc->PositionFlag));
					
					ImGui::Text("LifeTime");
                    ImGui::PushItemWidth(60);
                    ImGui::InputFloat("##RadialLifeTime", &(m_pSelectedRadialDesc->fLifeTime));
                    ImGui::PopItemWidth();

					ImGui::Text("Center");
					ImGui::PushItemWidth(60);
					ImGui::InputFloat("##RadialCenterX", &(m_pSelectedRadialDesc->Center.x));
					ImGui::SameLine();
					ImGui::InputFloat("##RadialCenterY", &(m_pSelectedRadialDesc->Center.y));
					ImGui::PopItemWidth();

					ImGui::Text("DistanceRange");
					ImGui::PushItemWidth(60);
					ImGui::InputFloat("##RadialDistanceRangeX", &(m_pSelectedRadialDesc->DistanceRange.x));
					ImGui::SameLine();
					ImGui::InputFloat("##RadialDistanceRangeY", &(m_pSelectedRadialDesc->DistanceRange.y));
					ImGui::PopItemWidth();

					ImGui::Text("IntensityRange");
					ImGui::PushItemWidth(60);
					ImGui::InputFloat("##RadialIntensityRange", &(m_pSelectedRadialDesc->IntensityRange));
					ImGui::PopItemWidth();

                    ImGui::Separator();
                }

            ImGui::End();
        }
    }

}

void CRadial_Controller::Radial_Base_Tab(CEffect_Radial::RADIAL_DESC& tEffectRadialDesc, _bool& IsCreate)
{
	 if (ImGui::Begin("Radial Base"))
    {

        if (m_bTagFlag) //이펙트 컨트롤러가 설정해준 이름값
        {
            if (ImGui::Button("Create"))
            {
                //프리팹에게 Desc 전달 -> 프리팹이 Desc로 클론 진행

                _tchar RadialTag[MAX_PATH] = {};
                CEffect_Radial::RADIAL_DESC RadialDesc{};

                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_RadialTag, strlen(m_RadialTag), RadialTag, MAX_PATH);

				RadialDesc.strMyTag = RadialTag;
				RadialDesc.eMyType = EFFECT_TYPE::RADIAL;

				RadialDesc.fLifeTime = 5.f;

                //매쉬이펙트 와 매쉬VB태그를 맞춰야할지는 고민해보자.
                m_tRadialDesc.emplace(RadialTag, RadialDesc);

				tEffectRadialDesc = RadialDesc;

                //생성됐음을 이펙트 컨트롤러에게 전달
                IsCreate = true;

                //초기화
				m_RadialTag[0] = _T('\0');
                m_bTagFlag = false;
            }
        }

       
        ImGui::End();
    }
}


void CRadial_Controller::UpdateSelected_RadialFormTag(_wstring RadialTag)
{

    auto iterRadialDesc = m_tRadialDesc.find(RadialTag);
    
    if (iterRadialDesc == m_tRadialDesc.end())
    {
        m_pSelectedRadialDesc = nullptr;
    }
    else
    {
        m_pSelectedRadialDesc = &iterRadialDesc->second;
    }


    if (m_pSelectedRadialDesc != nullptr)
    {
        m_bSelectedRadial = true;
    }
}

CEffect_Radial::RADIAL_DESC* CRadial_Controller::Get_RadialDesc(_wstring& RadialTag)
{
    auto iter = m_tRadialDesc.find(RadialTag);
    
    if (iter == m_tRadialDesc.end())
        return nullptr;

    return &iter->second;
}

void CRadial_Controller::Set_RadialDesc(_wstring& RadialTag, CEffect_Radial::RADIAL_DESC& RadialDesc)
{
	CEffect_Radial::RADIAL_DESC Desc = {};
    Desc = RadialDesc;

    m_tRadialDesc.emplace(RadialTag, Desc);
}

void CRadial_Controller::Set_RadialTag(const _char* szRadialTag)
{
    strcat_s(m_RadialTag, szRadialTag);

    m_bTagFlag = true;
}
void CRadial_Controller::Remove_Desc(const _wstring& DescTag)
{
    auto iterRadialDesc = m_tRadialDesc.find(DescTag);

    if (iterRadialDesc != m_tRadialDesc.end())
    {
        m_tRadialDesc.erase(iterRadialDesc);
    }


    m_iSelectedRadial = 0;
    m_bSelectedRadial = false;
    m_pSelectedRadialDesc = nullptr;
  
}

//void CRadial_Controller::RadialData_To_Json(Radial_DATADESC* pDesc)
//{
//	_string strFilePath = "../../Client/Bin/Resource/Effect/Prefabs/Common/Radial/";
//	_string strDacalDataTag = WStringToString(pDesc->wstrRadialDataTag);
//
//	strFilePath += strDacalDataTag;
//	strFilePath += ".json";
//
//	ofstream jsonStream(strFilePath.c_str());
//
//	json RadialDataJson;
//
//	RadialDataJson["RadialTag"] = strDacalDataTag;
//
//	RadialDataJson["TextureCount"] = pDesc->TextureDesc.size();
//
//	json EmissiveLuminanceJson = json::array();
//	EmissiveLuminanceJson.push_back(m_EmissiveLuminance.x);
//	EmissiveLuminanceJson.push_back(m_EmissiveLuminance.y);
//	EmissiveLuminanceJson.push_back(m_EmissiveLuminance.z);
//	RadialDataJson["EmissiveLuminance"] = EmissiveLuminanceJson;
//
//	json TextureDataJson = json::array();
//
//	for (size_t i = 0; i < pDesc->TextureDesc.size(); i++)
//	{
//		json Texture;
//
//		Texture["Texture_Type"] = pDesc->TextureDesc[i].eType;
//		Texture["Texture_Path"] = WStringToString(pDesc->TextureDesc[i].TexturePath);
//
//		TextureDataJson.push_back(Texture);
//	}
//
//	RadialDataJson["Textures"] = TextureDataJson;
//
//	jsonStream << RadialDataJson.dump(2);
//	jsonStream.close();
//}


CRadial_Controller* CRadial_Controller::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CRadial_Controller* pInstance = new CRadial_Controller(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : CRadial_Controller");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CRadial_Controller::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
}