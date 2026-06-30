#include "EditorPch.h"
#include "AnimNotifyTool.h"
#include "AnimationActor.h"
#include "SoundNotify.h"
#include "ColliderNotify.h"

#include "EffectNotify.h"
#include "ObjectFuncNotify.h"


#pragma region 
CAnimNotifyTool::CAnimNotifyTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext { pContext }
    , m_pGameInstance { CGameInstance::GetInstance()}
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}
HRESULT CAnimNotifyTool::Initialize(LEVEL eLevel)
{
    m_eCurLevel = eLevel;

    if (FAILED(Ready_Sound()))
    {
        CRASH("Failed Ready_Sound");
        return E_FAIL;
    }

	return S_OK;
}

void CAnimNotifyTool::Update()
{

}

void CAnimNotifyTool::Render()
{
    RenderUI_EditNotify();
}

void CAnimNotifyTool::Process_Notify(CAnimationActor* pActor, const _string& strAnimName, const _string& strModelDirPath, _float fDuration)
{
    ASSERT_CRASH(pActor);
    m_pCurrentActor = pActor;
    
    m_strCurrentAnimName = strAnimName;

    if (!strModelDirPath.empty())
    {
        m_strCurrentFolderPath = strModelDirPath;
        m_strCurrentFolderPath += string("\\Notify");
    }

    m_fCurrentDuration = fDuration;
}



void CAnimNotifyTool::Clear()
{
    for (auto& notify : m_AnimNotifies)
        Safe_Release(notify);

    m_AnimNotifies.clear();

    for (auto& notify : m_SoundNotifies)
        Safe_Release(notify);
    
    m_SoundNotifies.clear();

    for (auto& notify : m_ColliderNotifies)
        Safe_Release(notify);

    m_ColliderNotifies.clear();

    for (auto& notify : m_EffectNotifies)
        Safe_Release(notify);

    m_EffectNotifies.clear();

	for (auto& notify : m_ObjectNotifies)
		Safe_Release(notify);
	m_ObjectNotifies.clear();
    
    //m_SoundNotifies.clear();
    //m_ColliderNotifies.clear();
    //m_EffectNotifies.clear();
    //m_LightNotifies.clear();
}

void CAnimNotifyTool::RenderUI_EditNotify()
{
    ImGuiIO& io = ImGui::GetIO();

    ImVec2 vPos = ImVec2(g_iWinSizeX * 0.75f, 0.f); 
    ImGui::SetNextWindowPos(vPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(g_iWinSizeX * 0.5f, g_iWinSizeY * 0.5f), ImGuiCond_Once);

    ImGui::Begin("Edit_Notify");
    

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Sound"))
        {
            RenderUI_EditSound();
            m_eType = NOTIFYTYPE::SOUND;
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Effect"))
        {
            RenderUI_EditEffect();
            m_eType = NOTIFYTYPE::EFFECT;
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Collider"))
        {
            RenderUI_EditCollider();
            m_eType = NOTIFYTYPE::COLLIDER;
            ImGui::EndTabItem();
        }

		if (ImGui::BeginTabItem("Object"))
		{
			RenderUI_EditObjectFunc();
			m_eType = NOTIFYTYPE::OBJECT;
			ImGui::EndTabItem();
		}

        if (ImGui::BeginTabItem("Save"))
        {
            RenderUI_SaveNotify();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Load"))
        {
            RenderUI_LoadNotify();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    


    ImGui::End();
}


void CAnimNotifyTool::RenderUI_EditSound()
{
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Load"))
        {
            Load_SoundFiles();
            ImGui::EndTabItem();
        }

        // 2. Load??Sound File???댁슜 Notify ?ㅼ젙??異붽??쒕떎. 
        if (ImGui::BeginTabItem("Edit"))
        {
            Select_SoundNotify();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    
    

}

void CAnimNotifyTool::RenderUI_EditEffect()
{
    // 1. Collider Tag, TrackPosition, Active;

    static float fTrackPosition = {};
    ImGui::SetNextItemWidth(120.f);
    ImGui::InputFloat("TrackPosition", &fTrackPosition);

    static string strEffectTag;
    static bool IsActive = { false };

    if (strEffectTag.empty())
        strEffectTag.resize(256);

    ImGui::SetNextItemWidth(120.f);
    ImGui::InputText("EffectTag", &strEffectTag[0], strEffectTag.capacity());

    // 2. 
    if (ImGui::Button("Apply Effect Notify"))
    {
        json EffectJson;
        EffectJson["TrackPosition"] = fTrackPosition;
        EffectJson["EffectTag"] = strEffectTag.c_str();
        CEffectNotify* pEffectNotify = CEffectNotify::From_Json(EffectJson);
        m_EffectNotifies.emplace_back(pEffectNotify);
    }
}

void CAnimNotifyTool::RenderUI_EditCollider()
{
    // 1. Collider Tag, TrackPosition, Active;

    static float fTrackPosition = {};
    ImGui::SetNextItemWidth(120.f); 
    ImGui::InputFloat("TrackPosition", &fTrackPosition);

    static string strColliderTag;
    static bool IsActive = { false };

    if (strColliderTag.empty())
    strColliderTag.resize(256);

    ImGui::SetNextItemWidth(120.f);
    ImGui::InputText("Tag", &strColliderTag[0], strColliderTag.capacity());
    
    ImGui::Checkbox("Active", & IsActive);


    if (ImGui::Button("Apply Collider Notify"))
    {
        json ColliderJson;
        ColliderJson["TrackPosition"] = fTrackPosition;
        ColliderJson["ColliderTag"] = strColliderTag.c_str(); // ?낅젰諛쏆? ?ㅽ듃留곸쑝濡?
        ColliderJson["IsActive"] = IsActive;
        CColliderNotify* pColliderNotify = CColliderNotify::From_Json(ColliderJson);
        m_ColliderNotifies.emplace_back(pColliderNotify);
    }


}

void CAnimNotifyTool::RenderUI_EditObjectFunc()
{
	static float fTrackPosition = {};
	ImGui::SetNextItemWidth(120.f);
	ImGui::InputFloat("TrackPosition", &fTrackPosition);

	static string strObjectTag;

	if (strObjectTag.empty())
		strObjectTag.resize(256);

	ImGui::SetNextItemWidth(120.f);
	ImGui::InputText("Tag (Type|Key)", strObjectTag.data(), strObjectTag.capacity());

	if (ImGui::Button("Apply Object Notify"))
	{
		json ObjectJson;
		ObjectJson["TrackPosition"] = fTrackPosition;
		ObjectJson["ObjectTag"] = strObjectTag.c_str();
		CObjectFuncNotify* pObjectNotify = CObjectFuncNotify::From_Json(ObjectJson);
		m_ObjectNotifies.emplace_back(pObjectNotify);
	}
}

void CAnimNotifyTool::RenderUI_SaveNotify()
{
    ImGui::Text("Animation Name : %s", m_strCurrentAnimName.c_str());
    ImGui::Text("Duration : %.2f", m_fCurrentDuration);

#ifdef _DEBUG
    Render_CurrentNotify();
#endif
    ImGui::Separator();

    Save_Notify();
    
}

void CAnimNotifyTool::RenderUI_LoadNotify()
{
    ImGui::Text("Animation Name : %s", m_strCurrentAnimName.c_str());
    ImGui::Text("Duration : %.2f", m_fCurrentDuration);

    if (m_IsLoadNotify)
        ImGui::Text("All Animation Notify Loaded");


#ifdef _DEBUG
    if (m_IsLoadNotify)
        Render_CurrentNotify();
#endif

    ImGui::Separator();

    // 2. Load
    Load_NotifyFromFile();

    // 3. Load
#ifdef _DEBUG
    if (ImGui::Button("Load All Animation Notifies"))
    {
        m_IsLoadNotify = true;
        ASSERT_CRASH(m_pCurrentActor);
		
        m_pCurrentActor->Register_AllNotifies(m_strCurrentFolderPath);
    }
#endif
}

void CAnimNotifyTool::Load_SoundFiles()
{

    if (ImGui::Button("Load Sound File"))
    {
        IGFD::FileDialogConfig config;
        config.path = "../../Client/Bin/Resource/Sound/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        ImGuiFileDialog::Instance()->OpenDialog("Load Sound File", "Import Sound", ".wav", config);
    }

    ImGui::SameLine();

    if (ImGui::Button("Load Sound Folder"))
    {
        IGFD::FileDialogConfig config;
        config.path = "../../Client/Bin/Resource/";
        config.flags = ImGuiFileDialogFlags_Modal;

        ImGuiFileDialog::Instance()->OpenDialog("Load Sound Folder", "Import Sound Foloder", nullptr, config);
    }

    ImVec2 vMinSize = ImVec2(600, 400);  
    ImVec2 vMaxSize = ImVec2(800, 400); 

    if (ImGuiFileDialog::Instance()->Display(
        "Load Sound File", ImGuiWindowFlags_NoCollapse
        , vMinSize
        , vMaxSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _string strSoundPath = ImGuiFileDialog::Instance()->GetCurrentFileName();
            Load_SoundsFromFile(strFilePath, strSoundPath);
        }
        ImGuiFileDialog::Instance()->Close();
    }


    if (ImGuiFileDialog::Instance()->Display(
        "Load Sound Folder", ImGuiWindowFlags_NoCollapse
        , vMinSize
        , vMaxSize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFolderPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            Load_AllSoundsFromFolder(strFolderPath);
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

// 紐⑸줉 ?뺤씤 諛?Sound ?뚯씪 ?좏깮.
void CAnimNotifyTool::Select_SoundNotify()
{
    _wstring objTag = {};
    _wstring modelTag = {};

    ImGui::BeginChild("left pane", ImVec2(g_iWinSizeX * 0.25f, 0), true);

    static int iSelectedIndex = -1;
    _uint id = 0;

    for (auto& pair : m_SoundTags)
    {
        if (ImGui::Selectable(pair.first.c_str(), id == iSelectedIndex))
        {
            iSelectedIndex = id;
            m_CurrentSoundTag = pair.first;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    if (iSelectedIndex >= 0 && iSelectedIndex < m_SoundTags.size())
        Edit_SoundNotify();

}

#ifdef _DEBUG
void CAnimNotifyTool::Render_CurrentNotify()
{
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    _bool IsDeleted = { false };

    if (ImGui::BeginTabBar("TabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Sound List"))
        {
            _uint iDeleteIndex = {};

            _uint iIndex = { 0 };
            for (auto& SoundNotify : m_SoundNotifies)
            {
                ImGui::PushID(iIndex);

                SoundNotify->ImGui_Print();

                if (ImGui::Button("Delete"))
                {
                    IsDeleted = true;
                    iDeleteIndex = iIndex;
                }

                ImGui::PopID();

                if (iIndex < m_SoundNotifies.size() - 1)
                    ImGui::Separator();

                iIndex++;
            }

            if (IsDeleted)
            {
                auto iterDelete = next(m_SoundNotifies.begin(), iDeleteIndex);
                Safe_Release(*iterDelete);
                m_SoundNotifies.erase(iterDelete);
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Effect List"))
        {
            _uint iDeleteIndex = {};

            _uint iIndex = { 0 };
            for (auto& EffectNotify : m_EffectNotifies)
            {
                ImGui::PushID(iIndex);

                EffectNotify->ImGui_Print();

                if (ImGui::Button("Delete"))
                {
                    IsDeleted = true;
                    iDeleteIndex = iIndex;
                }

                ImGui::PopID();

                if (iIndex < m_EffectNotifies.size() - 1)
                    ImGui::Separator();

                iIndex++;
            }

            if (IsDeleted)
            {
                auto iterDelete = next(m_EffectNotifies.begin(), iDeleteIndex);
                Safe_Release(*iterDelete);
                m_EffectNotifies.erase(iterDelete);
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Collider List"))
        {
            _uint iDeleteIndex = {};

            _uint iIndex = { 0 };
            for (auto& ColliderNotify : m_ColliderNotifies)
            {
                ImGui::PushID(iIndex);

                ColliderNotify->ImGui_Print();

                if (ImGui::Button("Delete"))
                {
                    IsDeleted = true;
                    iDeleteIndex = iIndex;
                }

                ImGui::PopID();

                if (iIndex < m_ColliderNotifies.size() - 1)
                    ImGui::Separator();

                iIndex++;
            }

            if (IsDeleted)
            {
                auto iterDelete = next(m_ColliderNotifies.begin(), iDeleteIndex);
                Safe_Release(*iterDelete);
				m_ColliderNotifies.erase(iterDelete);
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Light List"))
        {
            ImGui::EndTabItem();
        }

		if (ImGui::BeginTabItem("Object List"))
		{
			_uint iDeleteIndex = {};

			_uint iIndex = { 0 };
			for (auto& pObjectNotify : m_ObjectNotifies)
			{
				ImGui::PushID(iIndex);

				pObjectNotify->ImGui_Print();

				if (ImGui::Button("Delete"))
				{
					IsDeleted = true;
					iDeleteIndex = iIndex;
				}

				ImGui::PopID();

				if (iIndex < m_ObjectNotifies.size() - 1)
					ImGui::Separator();

				iIndex++;
			}

			if (IsDeleted)
			{
				auto iterDelete = next(m_ObjectNotifies.begin(), iDeleteIndex);
				Safe_Release(*iterDelete);
				m_ObjectNotifies.erase(iterDelete);
			}

			ImGui::EndTabItem();
		}

        ImGui::EndTabBar();
    }


}
#endif // _DEBUG



void CAnimNotifyTool::Save_Notify()
{

    if (ImGui::Button("Save All Notifyes"))
    {
        IGFD::FileDialogConfig config;
        config.path = m_strCurrentFolderPath;
		config.fileName = m_strCurrentAnimName;
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

        ImGuiFileDialog::Instance()->OpenDialog("Save Notify", "Export File", ".json", config);
    }

    ImVec2 vMinSize = ImVec2(600, 400);  // 理쒖냼 ?ш린
    ImVec2 vMaxSize = ImVec2(800, 400); // 理쒕? ?ш린

    if (ImGuiFileDialog::Instance()->Display("Save Notify"
        , ImGuiWindowFlags_NoCollapse
        , vMinSize
        , vMaxSize
    )) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            _string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

            Save_NotifyToJson(strFilePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void CAnimNotifyTool::Load_NotifyFromFile()
{
    // 1. 
    if (ImGui::Button("Load Notifyes"))
    {
        IGFD::FileDialogConfig config;
        config.path = m_strCurrentFolderPath;
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

        ImGuiFileDialog::Instance()->OpenDialog("Load Notify", "Import File", ".json", config);
    }

    ImVec2 vMinSize = ImVec2(600, 400);  // 理쒖냼 ?ш린
    ImVec2 vMaxSize = ImVec2(800, 400); // 理쒕? ?ш린
    _string strFileName = {};
    _string strFilePath = {};
    _string strFolderPath = {};

    if (ImGuiFileDialog::Instance()->Display("Load Notify"
        , ImGuiWindowFlags_NoCollapse
        , vMinSize
        , vMaxSize
    )) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

            size_t lastSlashPos = strFilePath.find_last_of("\\");
            if (lastSlashPos != string::npos) {
                strFolderPath += strFilePath.substr(0, lastSlashPos);

            }
            Load_NotifyFromJson(strFilePath);
        }
        ImGuiFileDialog::Instance()->Close();
    }    
}


void CAnimNotifyTool::Load_SoundsFromFile(const _string& strFilePath, const _string& strSoundPath)
{
    size_t last_dot_pos = strSoundPath.find_last_of('.');
    if (last_dot_pos != std::string::npos) {
        _string strSoundTag = strSoundPath.substr(0, last_dot_pos);
        _wstring wStrSoundTag = StringToWString(strSoundTag);

        // 1. Sound Load
        m_pGameInstance->Load_Sound(wStrSoundTag, strFilePath.c_str());

        // 2. Sound ?대쫫 愿由?
        m_SoundTags.emplace(strSoundTag, wStrSoundTag);
    }
    else
    {
        MSG_BOX("");
        return;
    }
}

void CAnimNotifyTool::Load_AllSoundsFromFolder(const _string& strFolderPath)
{

    for (const auto& entry : filesystem::directory_iterator(strFolderPath))
    {
        if (entry.is_regular_file())
        {
            _string filePath = entry.path().string();
            _string fileName = entry.path().filename().string();
            _string extension = entry.path().extension().string();

            // .wav
            if (extension == ".wav" || extension == ".WAV")
            {
                _string soundTag = entry.path().stem().string(); // ?뺤옣???쒖쇅???뚯씪紐?
                _wstring wSoundTag = StringToWString(soundTag);

                m_pGameInstance->Load_Sound(wSoundTag, filePath.c_str());
                m_SoundTags.emplace(soundTag, wSoundTag);
            }
        }
    }
}

void CAnimNotifyTool::Edit_SoundNotify()
{
    ImGui::BeginChild("Right pane", ImVec2(g_iWinSizeX * 0.25f, 0), true);

    ImGui::Text("Current Sound Tag :  %s", m_CurrentSoundTag.c_str());

    static float fTrackPosition = {};
    ImGui::InputFloat("TrackPosition", &fTrackPosition);

    // 媛??섏쑝硫?Max 媛믪쑝濡??먮룞 ?ㅼ젙.
    fTrackPosition = clamp(fTrackPosition, 0.f, m_fCurrentDuration);
   

    static float fVolume = {};
    ImGui::SliderFloat("Volume", &fVolume, 0.f, 1.f);

    if (ImGui::RadioButton("Effect", m_CurrentSoundType == "Effect"))
        m_CurrentSoundType = "Effect";

    ImGui::SameLine();

    if (ImGui::RadioButton("Other", m_CurrentSoundType == "Other"))
        m_CurrentSoundType = "Other";
  

    // 1. ?대옒?ㅻ줈 由ъ뒪?몄뿉 ??ν븯湲?
    if (ImGui::Button("Add SoundNotify"))
    {
        json SoundJson;
        SoundJson["TrackPosition"] = fTrackPosition;
        SoundJson["SoundTag"] = m_CurrentSoundTag;
        SoundJson["SoundType"] = m_CurrentSoundType;
        SoundJson["Volume"] = fVolume;  // ?뚯닔??3?먮━濡?諛섏삱由?
        CSoundNotify* pSoundNotify = CSoundNotify::From_Json(SoundJson);
        m_SoundNotifies.emplace_back(pSoundNotify);
    }

    ImGui::EndChild();
}

void CAnimNotifyTool::Save_NotifyToJson(const _string& strFilePath)
{
    // 吏?뺣맂 File 寃쎈줈濡?Json 留뚮뱾湲?.
    ofstream jsonStream(strFilePath.c_str());

    // 0. ?꾩껜 Json
    json notifyJson;

    // 1. Sound
    notifyJson["AnimName"] = m_strCurrentAnimName;
    notifyJson["Notifies"] = json::array();

    for (auto& soundNotify : m_SoundNotifies)
        notifyJson["Notifies"].emplace_back(soundNotify->To_Json());

    // 2. Effect
    for (auto& effectNotify : m_EffectNotifies)
        notifyJson["Notifies"].emplace_back(effectNotify->To_Json());

    // 3. Collider
    for (auto& colliderNotify : m_ColliderNotifies)
        notifyJson["Notifies"].emplace_back(colliderNotify->To_Json());

	//4. ObjectFunc
	for (auto& pObjectNotify : m_ObjectNotifies)
		notifyJson["Notifies"].emplace_back(pObjectNotify->To_Json());

    jsonStream << notifyJson.dump(4);
    jsonStream.close();
}

void CAnimNotifyTool::Load_NotifyFromJson(const _string& strFilePath)
{
    // 1. 
    Clear();

    ifstream jsonStream(strFilePath.c_str());
    if (!jsonStream.is_open())
    {
        return;
    }
    
    // 2. ?뚯씪 ?댁슜??json 媛앹껜濡??뚯떛
    json notifyJson;
    jsonStream >> notifyJson;
    jsonStream.close();

    m_strCurrentAnimName = notifyJson["AnimName"].get<_string>();
    
    if (notifyJson.contains("Notifies") && notifyJson["Notifies"].is_array())
    {
        for (const auto& notifyObject : notifyJson["Notifies"])
        {
            string type = notifyObject["NotifyType"].get<string>();

            if (type == "Sound")
            {
                // CSoundNotify ?대옒?ㅼ뿉??From_Json ?⑥닔媛 ?덈떎怨?媛??
                CSoundNotify* pSoundNotify = CSoundNotify::From_Json(notifyObject);
                m_SoundNotifies.emplace_back(pSoundNotify);
            }
            else if (type == "Collider")
            {
                CColliderNotify* pColliderNotify = CColliderNotify::From_Json(notifyObject);
                m_ColliderNotifies.emplace_back(pColliderNotify);
            }
            else if (type == "Effect")
            {
                CEffectNotify* pEffectNofiy = CEffectNotify::From_Json(notifyObject);
                m_EffectNotifies.emplace_back(pEffectNofiy);
            }
            
			else if (type == "Object")
			{
				CObjectFuncNotify* pEffectNofiy = CObjectFuncNotify::From_Json(notifyObject);
				m_ObjectNotifies.emplace_back(pEffectNofiy);
			}

            // else if (type == "Effect") { ... }
        }
    }
}
    



#pragma endregion


HRESULT CAnimNotifyTool::Ready_Sound()
{
    //m_pGameInstance->Load_Sound(TEXT("Augusta_Attack01"), "../../Client/Bin/Resource/Player/Augusta/Sound/Attack/Augusta_Attack01_01.wav");
    
    return S_OK;
}

CAnimNotifyTool* CAnimNotifyTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel)
{
    CAnimNotifyTool* pInstance = new CAnimNotifyTool(pDevice, pContext);

    if (FAILED(pInstance->Initialize(eLevel)))
    {
        MSG_BOX("Failed to Create : CAnimNotifyTool");
        Safe_Release(pInstance);
    }

    return pInstance;
}
void CAnimNotifyTool::Free()
{
    CBase::Free();
    Safe_Release(m_pGameInstance);
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Clear();
}