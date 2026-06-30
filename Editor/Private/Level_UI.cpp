#include "EditorPch.h"
#include "Level_UI.h"

#include "Event_Level.h"
#include "Custom_UI.h"
#include "FreeCamera.h"
#include "GameObject.h"
#include "Animator_UI.h"

#include "VIBuffer_Rect_Instance_UI.h"


#define         STR2WSTR(str)                                   _wstring(str.begin(), str.end())
#define         WSTR2STR(wstr)                                  _string(wstr.begin(), wstr.end())

#define         STR_ONLYFILENAME(str)                           std::filesystem::path(str).stem().string();

#define			TO_RAD(DEGREE)									XMConvertToRadians(DEGREE)
#define			TO_DEG(RADIAN)									XMConvertToDegrees(RADIAN)

#define			IS_BETWEEN(condition, minValue, maxValue)		(((minValue) <= (condition)) && ((condition) < (maxValue)))	// ?댁긽 and 誘몃쭔

#define			ROT_TO_QUAT(ROT_X, ROT_Y, ROT_Z)				XMQuaternionRotationRollPitchYaw(ROT_X, ROT_Y, ROT_Z)
#define			MAT_TO_ROT(FLOAT4X4)							_float3{TO_DEG(asin(-FLOAT4X4._32)), TO_DEG(atan2(FLOAT4X4._31, FLOAT4X4._33)), TO_DEG(atan2(FLOAT4X4._12, FLOAT4X4._22))}
#define			QUAT_TO_MAT(QUAT)								XMMatrixRotationQuaternion(QUAT)



CLevel_UI::CLevel_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_UI::Initialize()
{
    const   _uint       iDestLevel = ENUM_CLASS(LEVEL::UI);

    // ==============================
    // * Add Prototypes
    // ==============================

    // Custom UI
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_GameObject_Custom_UI",
        CCustom_UI::Create(m_pDevice, m_pContext))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Custom_UI Load Failed. The Custom_UI may have already been loaded.\n");
    
    // Shader
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Editor_Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Shader Load Failed. The Shader may have already been loaded.\n");
    
    // Shader_Instance
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_Shader_VtxPosTex_Instance"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Editor_Shader_VtxPosTex_Instance.hlsl"), VTXUIINSTANCE::Elements, VTXUIINSTANCE::iNumElements))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Shader_Instance Load Failed. The Shader_Instance may have already been loaded.\n");

    // VIBuffer_Rect
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect"),
        CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] VIBuffer_Rect Load Failed. The VIBuffer_Rect may have already been loaded.\n");

    // VIBuffer_Rect_Instance_UI
    CVIBuffer_Rect_Instance_UI::RECT_INSTANCE_UI_DESC tRectInstDesc = {};
    tRectInstDesc.iNumInstance = 500U;
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, TEXT("Prototype_Component_VIBuffer_Rect_Instance_UI"),
        CVIBuffer_Rect_Instance_UI::Create(m_pDevice, m_pContext, &tRectInstDesc))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] VIBuffer_Rect_Instance_UI Load Failed. The VIBuffer_Rect_Instance_UI may have already been loaded.\n");

    // Animator_UI
    if (FAILED(m_pGameInstance->Add_Prototype(iDestLevel, L"Prototype_Component_Animator_UI",
        CAnimator_UI::Create(m_pDevice, m_pContext))))
        OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Animator_UI Load Failed. The Animator_UI may have already been loaded.\n");


    // ==============================
    // * Add GameObjects
    // ==============================
    




    // ==============================
    // * Add Lights
    // ==============================
    LIGHT_DESC			LightDesc{};

    LightDesc.eType = LIGHT_DESC::TYPE::DIRECTION;
    LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);	// Light 諛⑺뼢
    LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);		// Light ?됱긽 諛?諛앷린???멸린
    LightDesc.vAmbient = _float4(0.4f, 0.4f, 0.4f, 1.f);	// Light ?섍꼍愿묒쑝濡?媛?? 理쒖냼 諛앷린 蹂댁옣??愿??
    LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);		// Light 諛섏궗愿?

    if (FAILED(m_pGameInstance->Add_Light(L"Light_Default", LightDesc)))
        return E_FAIL;

    return S_OK;
}

void CLevel_UI::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("UI"));

    for (auto& customUI : m_vecCustomUIs)
        customUI.pCustomUI->Priority_Update(fTimeDelta);
    for (auto& customUI : m_vecCustomUIs)
        customUI.pCustomUI->Update(fTimeDelta);
    for (auto& customUI : m_vecCustomUIs)
        customUI.pCustomUI->Late_Update(fTimeDelta);

    m_pPreObj = m_pCurObj;

    Update_Picking();
    Update_MenuWindow();

    Update_Hierarchy();
    
    Update_SaveLoad();
    Update_Inspector();
    Update_InstanceEditor();

    Update_AnimEditor(fTimeDelta);
    Update_ObjectParents();


    Update_SelectedKeyframeDesc();
}

void CLevel_UI::Render()
{
    for (auto& customUI : m_vecCustomUIs)
        customUI.pCustomUI->Render();
}

void CLevel_UI::Update_Picking()
{
    // ?쇳궧 ?좏깮..?
    //m_pGameInstance->isPicked();
}

void CLevel_UI::Update_MenuWindow()
{
    // ============================== 
    // ?대?吏 濡쒕뱶?댁꽌 UI媛앹껜濡?異붽??섎뒗 李?
    // ============================== 

    ImGui::Begin("Editor");

#pragma region Load Image

    // ===== [UI] Load Image =====
    if (ImGui::Button("Load Image..", ImVec2(100.f, 50.f)))
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Image_Load", "Select Image", ".png,.jpg,.dds,.tga", config);
    }
    // End ==============================


    // ===== [Logic] Load FilePath, Create & Store CustomUI =====
    _wstring strFilePath = {}, strFileName = {};

    static _int isInstanceSelected = 0; // 0 = false, 1 = true
    ImGui::Text("UI Type");
    if (ImGui::RadioButton("Normal", isInstanceSelected == 0))
        isInstanceSelected = 0;
    if (ImGui::RadioButton("Instance", isInstanceSelected == 1))
        isInstanceSelected = 1;
    _bool isInstance = (isInstanceSelected == 1);

    ImGui::Separator();

    if (ImGuiFileDialog::Instance()->Display("UI_Image_Load"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // ?뚯씪 ?좏깮 ??
        {
            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _string fileName = STR_ONLYFILENAME(ImGuiFileDialog::Instance()->GetCurrentFileName());

            strFilePath = StringToWString(filePath);
            strFileName = StringToWString(fileName);


            // ?곷?寃쎈줈
            _tchar curPath[256] = {};
            _wgetcwd(curPath, 256);
            filesystem::path basePath = curPath;
            filesystem::path targetPath = filePath;
            filesystem::path relativePath = filesystem::relative(targetPath, basePath);


            CCustom_UI::CUSTOM_UI_DESC tCustomUIDesc = {};
            tCustomUIDesc.fSizeX = 100;
            tCustomUIDesc.fSizeY = 100;
            tCustomUIDesc.fX = g_iWinSizeX / 2.f;
            tCustomUIDesc.fY = g_iWinSizeY / 2.f;
            tCustomUIDesc.strFilePath = relativePath.wstring();
            tCustomUIDesc.strFileName = strFileName;
            tCustomUIDesc.isInstance = isInstance;

            // ?앹꽦 ??濡쒖뺄 而⑦뀒?대꼫??異붽?
            CGameObject* pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::UI), L"Prototype_GameObject_Custom_UI", PROTOTYPE::GAMEOBJECT, &tCustomUIDesc));
            //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::UI), L"Layer_UI_Custom", pCustomObj)))
            //    CRASH(Failed to add Custom_UI gameobject.);

            HIERARCHY_OBJ_DESC tObjDesc = { };
            tObjDesc.pCustomUI = static_cast<CCustom_UI*>(pCustomObj);
            tObjDesc.strObjName = StringToWString(fileName);

            m_vecCustomUIs.push_back(tObjDesc);
            m_pCurObj = pCustomObj;

            CTransform* pObjTransformCom = dynamic_cast<CTransform*>(pCustomObj->Get_Component(L"Com_Transform"));
            pObjTransformCom->Scale( _float3(
                dynamic_cast<CCustom_UI*>(pCustomObj)->Get_UIDesc().vecSize[0].x,
                dynamic_cast<CCustom_UI*>(pCustomObj)->Get_UIDesc().vecSize[0].y,
                1
            ));

            if (isInstance)
            {
                vector<CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC>* pDescs = &dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc().vecInstanceDescs;
                pDescs->push_back(CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC());
            }

        }
        ImGuiFileDialog::Instance()->Close();
    }
    // End ==============================

#pragma endregion

#pragma region [Window Toggle] Save / Load

    // ===== [Button] Save / Load =====
    if (ImGui::Button("Save / Load", ImVec2(100.f, 50.f)))
    {
        m_isOn_SaveLoad = !m_isOn_SaveLoad;
    }

#pragma endregion



    ImGui::End();
}

void CLevel_UI::Update_Hierarchy()
{
    // ============================== 
    // * Hierarchy
    // ============================== 

    ImGui::Begin("Hierarchy");

    if (ImGui::BeginMenu("Manual Menu"))
    {
        if (ImGui::MenuItem("Manual Update Childs"))
            Update_ObjectChilds();

        ImGui::EndMenu();
    }

    ImGui::Separator();

    static _bool isActiveCurObject = true;

    // quick edit UIName
    if (m_pCurObj)
    {
        CCustom_UI::CUSTOM_UI_DESC tDesc = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc();
        static _char szUIName[256] = {};
        
        _string strUIName = WStringToString(tDesc.strUIName);
        strcpy_s(szUIName, strUIName.c_str());

        if (ImGui::Checkbox("##CurObjectToggle", &isActiveCurObject))
        {
            m_pCurObj->SetActivate(isActiveCurObject);
        }
        ImGui::SameLine();

        ImGui::Text("Name ");
        ImGui::SameLine();
        if (ImGui::InputText("##Edit Name", szUIName, 256))
        {
            _string strEditUIName = szUIName;
            
            tDesc.strUIName = StringToWString(strEditUIName);
            dynamic_cast<CCustom_UI*>(m_pCurObj)->Set_UIDesc(tDesc);
        }
        ImGui::SameLine();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.0f, 0.0f, 1.0f));
        if (ImGui::Button("Delete##Hierarchy Object Delete"))
        {
            //m_vecCustomUIs.erase();
            for (_uint i = 0; i < m_vecCustomUIs.size(); i++)
                if (m_pCurObj == (m_vecCustomUIs[i].pCustomUI))
                {
                    m_vecCustomUIs.erase(m_vecCustomUIs.begin() + i);
                    Safe_Release(m_pCurObj);
                    m_pCurObj = nullptr;
                    break;
                }
        }

        ImGui::PopStyleColor(3);
    }
    else
    {
        ImGui::Text("Nothing Selected");
    }

    ImGui::Separator();

    // 留??꾨젅?꾨쭏??踰≫꽣瑜??듯빐 遺紐?援ъ“瑜??뚯븙?섍퀬,
    // 洹멸구 而⑦뀒?대꼫???댁? ?? ?섏씠?대씪?ㅼ뿉???쒖떆?

    ImGuiTreeNodeFlags flags = 0;// ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Selected;

    // ?섏씠?대씪??硫붿씤
    for (auto& ui : m_vecCustomUIs)
    {
        CCustom_UI* pUI = ui.pCustomUI;
        CCustom_UI::CUSTOM_UI_DESC desc = pUI->Get_UIDesc();

        // 遺紐④? ?녿뒗 (理쒖긽?? 媛앹껜留?癒쇱? ?쒖떆
        if (desc.strParentName.empty())
            Update_Hierarchy_CheckTree(pUI, flags);
    }

    // 遺紐⑥씠由꾩? ?덉?留??대떦 遺紐④? ?녿뒗 寃쎌슦 蹂꾨룄 UI濡??쒖떆
    ImGuiTreeNodeFlags flags_missingParent = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected;
    if (ImGui::CollapsingHeader("Missing Parent Objects", flags_missingParent))
    {
        for (auto& ui : m_vecCustomUIs)
        {
            _bool isParentMissing = true;

            static _int iSelected = -1;
            _uint iIndex = 0;

            // 遺紐??대쫫???녿뒗 寃쎌슦 泥댄겕X (?꾩뿉???대? 李얠븯?쇰?濡?
            if (ui.pCustomUI->Get_UIDesc().strParentName.empty() ||
                ui.pCustomUI->Get_UIDesc().strParentName == ui.pCustomUI->Get_UIDesc().strUIName)
                continue;
            // ?대떦?섎뒗 遺紐④? ?덈뒗吏 寃??
            for (auto& otherui : m_vecCustomUIs)
                if (otherui.pCustomUI->Get_UIDesc().strUIName ==
                    ui.pCustomUI->Get_UIDesc().strParentName)
                {
                    isParentMissing = false;

                    break;
                    iIndex++;
                }

            
            if (isParentMissing)
            {
                _wstring wstrUIName = ui.pCustomUI->Get_UIDesc().strUIName;
                _string strUIName = WStringToString(wstrUIName);
                if (ImGui::Selectable(strUIName.c_str(), iSelected == iIndex))
                {
                    m_pCurObj = ui.pCustomUI;
                    isActiveCurObject = static_cast<CCustom_UI*>(m_pCurObj)->Get_Active();
                }
            }
        }
    }
    
    ImGui::End();
    
    
    

}

void CLevel_UI::Update_Hierarchy_CheckTree(CCustom_UI* pParentUI, ImGuiTreeNodeFlags flags)
{
    CCustom_UI::CUSTOM_UI_DESC desc = pParentUI->Get_UIDesc();

    _string strLabel = WStringToString(desc.strUIName);
    if (strLabel == "") strLabel = " ";
    if (ImGui::TreeNodeEx(strLabel.c_str(), flags))
    {
        // ?대┃ ???좏깮.
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            m_pCurObj = pParentUI;

        // m_vecCustomUIs ?꾩껜瑜??뚮㈃?? 遺紐??대쫫???쇱튂?섎뒗 媛앹껜瑜?李얠쓬
        for (auto& ui : m_vecCustomUIs)
        {
            CCustom_UI* pChild = ui.pCustomUI;
            CCustom_UI::CUSTOM_UI_DESC childDesc = pChild->Get_UIDesc();

            if (childDesc.strParentName == desc.strUIName)
                Update_Hierarchy_CheckTree(pChild, flags); // ?ш? ?몄텧
        }

        ImGui::TreePop();
    }
}

void CLevel_UI::Update_SaveLoad()
{
    if (!m_isOn_SaveLoad)
        return;
        
    ImGui::Begin("Save / Load");
    const ImVec2 buttonSize = { 100.f, 20.f };
#pragma region [UI] Open Dialog for Save / Load
    // ==============================
    // * [UI] UI Save
    // ==============================
    ImGui::Text("..Current UI Info");
    if (ImGui::Button("Save##InfoSave", buttonSize) &&
        m_pCurObj)
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/FJson/UIInfo/";
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Info_Save", "Select Info Save Path", ".json", config);
    }
    ImGui::SameLine();
    // ==============================
    // * [UI] UI Load
    // ==============================
    if (ImGui::Button("Load##InfoLoad", buttonSize))
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/FJson/UIInfo/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Info_Load", "Select Info", ".json", config);
    }

    ImGui::Separator();


    // ==============================
    // * [UI] Anim Save
    // ==============================
    ImGui::Text("..Current Anim");

    static _char szAnimName[256] = {};
    ImGui::Text("[Save] Anim Name");
    ImGui::InputText("##Anim Name", szAnimName, 256);

    if (ImGui::Button("Save##AnimSave", buttonSize) &&
        m_pCurObj)
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/FJson/UIAnim/";
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Anim_Save", "Select Anim Save Path", ".json", config);
    }
    ImGui::SameLine();
    // ==============================
    // * [UI] Anim Load
    // ==============================
    if (ImGui::Button("Load##AnimLoad", buttonSize) && 
        m_pCurObj)
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/FJson/UIAnim/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Anim_Load", "Select Anim", ".json", config);
    }

    ImGui::Separator();

    
    // ==============================
    // * [UI] Tree Save
    // ==============================
    ImGui::Text("..Current Tree");

    static _char szTreeName[256] = {};
    ImGui::Text("[Save] Tree Name");
    ImGui::InputText("##Tree Name", szTreeName, 256);

    if (ImGui::Button("Save##TreeSave", buttonSize) &&
        !m_vecCustomUIs.empty())
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/FJson/UITree/";
        config.flags = ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Tree_Save", "Select Tree Save Path", ".json", config);
    }
    ImGui::SameLine();
    // ==============================
    // * [UI] Tree Load
    // ==============================
    if (ImGui::Button("Load##TreeLoad", buttonSize))
    {
        IGFD::FileDialogConfig config;

        config.path = "../../Client/Bin/Resource/UI/FJson/UITree/";
        config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField | ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("UI_Tree_Load", "Select Tree", ".json", config);
    }

#pragma endregion
    ImGui::End();


#pragma region [Logic] Save / Load with Dialog
    // ==============================
    // * [Logic] UI Save
    // ==============================
    if (ImGuiFileDialog::Instance()->Display("UI_Info_Save"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            Update_ObjectChilds();

            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _wstring strFilePath = StringToWString(filePath);

            _string fileName = STR_ONLYFILENAME(ImGuiFileDialog::Instance()->GetCurrentFileName());
            _wstring strFileName = StringToWString(fileName);

            UI_INFO_DESC tCurUIInfoDesc = {};

            tCurUIInfoDesc.tUIDesc = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc();
            tCurUIInfoDesc.vPos = m_vCurObjPos;
            tCurUIInfoDesc.vRot = m_vCurObjRot;
            tCurUIInfoDesc.vSca = m_vCurObjSca;

            json jUIInfoData = {};
            to_json(jUIInfoData, tCurUIInfoDesc);

            ofstream file(strFilePath);
            file << jUIInfoData.dump(4);
            file.close();
        }
        ImGuiFileDialog::Instance()->Close();
    }
    // ==============================
    // * [Logic] UI Load
    // ==============================
    if (ImGuiFileDialog::Instance()->Display("UI_Info_Load"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // ?뚯씪 ?좏깮 ??
        {
            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _wstring strFilePath = StringToWString(filePath);

            ifstream file(strFilePath);
            json jUIInfoData = {};
            if (file.is_open()) {
                file >> jUIInfoData;
            }

            CCustom_UI::CUSTOM_UI_DESC tLoadUIInfoDesc = {};
            from_json(jUIInfoData["tUIDesc"], tLoadUIInfoDesc);


            _float3 vPos = { jUIInfoData["vPos"][0], jUIInfoData["vPos"][1], jUIInfoData["vPos"][2] };    m_vCurObjPos = vPos;
            _float3 vRot = { jUIInfoData["vRot"][0], jUIInfoData["vRot"][1], jUIInfoData["vRot"][2] };    m_vCurObjRot = vRot;
            _float3 vSca = { jUIInfoData["vSca"][0], jUIInfoData["vSca"][1], jUIInfoData["vSca"][2] };    m_vCurObjSca = vSca;

            CGameObject* pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::UI), L"Prototype_GameObject_Custom_UI", PROTOTYPE::GAMEOBJECT, &tLoadUIInfoDesc));
            //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::UI), L"Layer_UI_Custom", pCustomObj)))
            //    CRASH(Failed to add Custom_UI gameobject.);

            HIERARCHY_OBJ_DESC tObjDesc = { };
            tObjDesc.pCustomUI = static_cast<CCustom_UI*>(pCustomObj);
            tObjDesc.strObjName = tLoadUIInfoDesc.strFileName;

            _matrix matScale = XMMatrixScaling(vSca.x, vSca.y, vSca.z);
            _matrix matRotX = XMMatrixRotationX(TO_RAD(vRot.x));
            _matrix matRotY = XMMatrixRotationY(TO_RAD(vRot.y));
            _matrix matRotZ = XMMatrixRotationZ(TO_RAD(vRot.z));
            _matrix matRot = matRotZ * matRotY * matRotX;
            _matrix matTrans = XMMatrixTranslation(vPos.x, vPos.y, vPos.z);

            _matrix matWorld = matScale * matRot * matTrans;
            static_cast<CTransform*>(pCustomObj->Get_Component(L"Com_Transform"))->Set_WorldMatrix(matWorld);


            m_vecCustomUIs.push_back(tObjDesc);
            m_pCurObj = pCustomObj;
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // ==============================
    // * [Logic] Anim Save
    // ==============================
    if (ImGuiFileDialog::Instance()->Display("UI_Anim_Save"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // ?뚯씪 ?좏깮 ??
        {
            Update_ObjectChilds();

            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _wstring strFilePath = StringToWString(filePath);

            UI_ANIM_DESC tAnimDesc = {};

            tAnimDesc.tUIDesc = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc();
            _string strAnimName = _string(szAnimName);
            tAnimDesc.strAnimName = StringToWString(strAnimName);
            //tAnimDesc.iLerpType = m_iLerpType;
            tAnimDesc.isLoop = m_isAnimLoop;

            for (auto& keyframeDesc : m_vecUIKeyFrameDescs)
                tAnimDesc.vecKeyFrames.push_back(keyframeDesc);

            json jUIAnimData = {};
            to_json(jUIAnimData, tAnimDesc);

            ofstream file(filePath);
            file << jUIAnimData.dump(4);
            file.close();

            memset(szAnimName, 0, sizeof(szAnimName));
        }
        ImGuiFileDialog::Instance()->Close();
    }
    // ==============================
    // * [Logic] Anim Load
    // ==============================
    if (ImGuiFileDialog::Instance()->Display("UI_Anim_Load"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // ?뚯씪 ?좏깮 ??
        {
            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _wstring strFilePath = StringToWString(filePath);

            ifstream file(strFilePath);
            json jUIAnimData = {};
            if (file.is_open()) {
                file >> jUIAnimData;
            }

            UI_ANIM_DESC tLoadAnimDesc = {};
            from_json(jUIAnimData, tLoadAnimDesc);

            CAnimator_UI* ObjAnimatorCom = dynamic_cast<CAnimator_UI*> (m_pCurObj->Get_Component(L"Com_Animator_UI"));
            if (!ObjAnimatorCom) CRASH(cannot find AnimatorCom);

            _wstring strCurObjName = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc().strFileName;
            _wstring strReqObjName = tLoadAnimDesc.tUIDesc.strFileName;
            if (strCurObjName == strReqObjName)
            {
                if (FAILED(ObjAnimatorCom->Insert_Animation(tLoadAnimDesc)))
                {
                    _wstring strLog = L"[Level_UI][Update_SaveLoad] Insert Animation Failed. Animation [" + tLoadAnimDesc.strAnimName + L"] Already Exist.";
                    OutputDebugString(strLog.c_str());
                }
                else
                {
                    ObjAnimatorCom->Change_Animation(tLoadAnimDesc.strAnimName); // �ҷ��� �ִϸ��̼����� �Ҵ�

                    m_vecUIKeyFrameDescs.clear();
                    m_pSelectedKeyFrameDesc = nullptr;
                    for (auto& keyframe : tLoadAnimDesc.vecKeyFrames)
                        m_vecUIKeyFrameDescs.push_back(keyframe);
                }
            }
            else
            {
                _wstring strLog = L"[Level_UI][Update_SaveLoad] Load Failed. This Animation is not for this object.\nRequired Object Name : " + tLoadAnimDesc.tUIDesc.strFileName;
                OutputDebugString(strLog.c_str());
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // ==============================
    // * [Logic] Tree Save
    // =============================='
    if (ImGuiFileDialog::Instance()->Display("UI_Tree_Save"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // ?뚯씪 ?좏깮 ??
        {
            Update_ObjectChilds();

            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _wstring strFilePath = StringToWString(filePath);

            CUSTOM_UITREE_DESC tTreeDesc = {};

            _string strTreeName = _string(szTreeName);
            tTreeDesc.strTreeName = StringToWString(strTreeName);


            for (auto& tCustomUIDesc : m_vecCustomUIs)
            {
                // UI Info Desc..
                UI_INFO_DESC tCurUIInfoDesc = {};
                tCurUIInfoDesc.tUIDesc = dynamic_cast<CCustom_UI*>(tCustomUIDesc.pCustomUI)->Get_UIDesc();
                
                // (Info Desc) Transform ?뺣낫 怨꾩궛 諛????
                CTransform* pTargetTransform = static_cast<CTransform*>(tCustomUIDesc.pCustomUI->Get_Component(L"Com_Transform"));

                _vector		vXMObjPosition = {}, vXMObjQuaternion = {}, vXMObjScale = {};
                _float3		vStoreObjPosition = {}, vStoreObjRotation = {}, vStoreObjScale = {};
                XMMatrixDecompose(&vXMObjScale, &vXMObjQuaternion, &vXMObjPosition, pTargetTransform->Get_WorldMatrix());

                _float4x4	matStoreObjQuaternion = {};	// 荑쇳꽣?덉뼵
                XMStoreFloat4x4(&matStoreObjQuaternion, QUAT_TO_MAT(vXMObjQuaternion));

                XMStoreFloat3(&vStoreObjPosition, vXMObjPosition);
                vStoreObjRotation = MAT_TO_ROT(matStoreObjQuaternion);
                XMStoreFloat3(&vStoreObjScale, vXMObjScale);

                tCurUIInfoDesc.vPos = vStoreObjPosition;
                tCurUIInfoDesc.vRot = vStoreObjRotation;
                tCurUIInfoDesc.vSca = vStoreObjScale;

                // Info Desc ???
                tTreeDesc.vecUIInfoDescs.push_back(tCurUIInfoDesc);
            }

            json jUITreeData = {};
            to_json(jUITreeData, tTreeDesc);

            ofstream file(filePath);
            file << jUITreeData.dump(4);
            file.close();

            memset(szTreeName, 0, sizeof(szTreeName));
        }
        ImGuiFileDialog::Instance()->Close();
    }
    // ==============================
    // * [Logic] Tree Load
    // ==============================
    if (ImGuiFileDialog::Instance()->Display("UI_Tree_Load"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())    // ?뚯씪 ?좏깮 ??
        {
            _string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            _string fileName = STR_ONLYFILENAME(ImGuiFileDialog::Instance()->GetCurrentFileName());

            _wstring strFilePath = StringToWString(filePath);
            _wstring strFileName = StringToWString(fileName);

            ifstream file(strFilePath);
            json jUITreeData = {};
            if (file.is_open()) {
                file >> jUITreeData;
            }


            // relative path
            //_tchar curPath[256] = {};
            //_wgetcwd(curPath, 256);
            //filesystem::path basePath = curPath;
            //filesystem::path targetPath = filePath;
            //filesystem::path relativePath = filesystem::relative(targetPath, basePath);


            // json load
            CUSTOM_UITREE_DESC tLoadTreeDesc = {};
            from_json(jUITreeData, tLoadTreeDesc);


            // clear current data
            m_pCurObj = nullptr;
            m_vCurObjPos = {}; m_vCurObjRot = {}; m_vCurObjSca = {};
            for (auto& customUI : m_vecCustomUIs)
                Safe_Release(customUI.pCustomUI);
            m_vecCustomUIs.clear();


            // add 
            for (auto& loadDesc : tLoadTreeDesc.vecUIInfoDescs)
            {
                UI_INFO_DESC tLoadUIInfoDesc = loadDesc;

                m_vCurObjPos = tLoadUIInfoDesc.vPos;
                m_vCurObjRot = tLoadUIInfoDesc.vRot;
                m_vCurObjSca = tLoadUIInfoDesc.vSca;

                CGameObject* pCustomObj = static_cast<CGameObject*>(m_pGameInstance->Clone_Prototype(ENUM_CLASS(LEVEL::UI), L"Prototype_GameObject_Custom_UI", PROTOTYPE::GAMEOBJECT, &tLoadUIInfoDesc));
                //if (FAILED(m_pGameInstance->Add_GameObject_ToLayer(ENUM_CLASS(LEVEL::UI), L"Layer_UI_Custom", pCustomObj)))
                //    CRASH(Failed to add Custom_UI gameobject.);

                HIERARCHY_OBJ_DESC tObjDesc = { };
                tObjDesc.pCustomUI = static_cast<CCustom_UI*>(pCustomObj);
                tObjDesc.strObjName = tLoadUIInfoDesc.tUIDesc.strFileName;

                _matrix matScale = XMMatrixScaling(m_vCurObjSca.x, m_vCurObjSca.y, m_vCurObjSca.z);
                _matrix matRotX = XMMatrixRotationX(TO_RAD(m_vCurObjRot.x));
                _matrix matRotY = XMMatrixRotationY(TO_RAD(m_vCurObjRot.y));
                _matrix matRotZ = XMMatrixRotationZ(TO_RAD(m_vCurObjRot.z));
                _matrix matRot = matRotZ * matRotY * matRotX;
                _matrix matTrans = XMMatrixTranslation(m_vCurObjPos.x, m_vCurObjPos.y, m_vCurObjPos.z);

                _matrix matWorld = matScale * matRot * matTrans;
                static_cast<CTransform*>(pCustomObj->Get_Component(L"Com_Transform"))->Set_WorldMatrix(matWorld);



                m_vecCustomUIs.push_back(tObjDesc);
                //m_pCurObj = pCustomObj;
            }

        }
        ImGuiFileDialog::Instance()->Close();
    }
#pragma endregion

}

void CLevel_UI::Update_Inspector()
{
    // ============================== 
    // ?좎궗 ?몄뒪?숉꽣 李? 而댄룷?뚰듃 議곗옉 媛?ν븯?꾨줉
    // ============================== 


    if (m_pCurObj == nullptr)
        return;

    // ?좏깮以묒씤 ?ㅻ툕?앺듃 媛?遺덈윭???Transform ?섏젙 媛?ν븯?꾨줉

    ImGui::Begin("Inspector");
    
    CTransform* pTargetTransform = dynamic_cast<CTransform*>(m_pCurObj->Get_Component(L"Com_Transform"));
    _bool   isOn_TransformCom = pTargetTransform != nullptr;

#pragma region [Component] Transform

    if (isOn_TransformCom)
    {
        if (m_pPreObj != m_pCurObj ||
            m_isPlayAnimation)
            // ?꾨땲硫???곸쓽 遺紐④? 諛붾뚯뿀????
        {
            
            _vector		vXMObjPosition = {}, vXMObjQuaternion = {}, vXMObjScale = {};
            _float3		vStoreObjPosition = {}, vStoreObjRotation = {}, vStoreObjScale = {};
            XMMatrixDecompose(&vXMObjScale, &vXMObjQuaternion, &vXMObjPosition, pTargetTransform->Get_WorldMatrix());

            _float4x4	matStoreObjQuaternion = {};	// 荑쇳꽣?덉뼵
            XMStoreFloat4x4(&matStoreObjQuaternion, QUAT_TO_MAT(vXMObjQuaternion));

            XMStoreFloat3(&vStoreObjPosition, vXMObjPosition);
            vStoreObjRotation = MAT_TO_ROT(matStoreObjQuaternion);
            XMStoreFloat3(&vStoreObjScale, vXMObjScale);

            // ??낇븯??蹂댁뿬以?
            m_vCurObjPos = vStoreObjPosition;
            m_vCurObjRot = vStoreObjRotation;
            m_vCurObjSca = vStoreObjScale;
        }
        

        if (ImGui::CollapsingHeader("Transform"))
        {
            static _float fSensitivity = 1.f;
            ImGui::Text("Sensitivity");
            ImGui::SameLine();
            ImGui::DragFloat("##Sensitivity", &fSensitivity, 0.001f, 0.001f, 10.f);
            ImGui::Separator();

            if (ImGui::BeginMenu("Reset Menu"))
            {
                vector<_float2> targetImgSize = static_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc().vecSize;

                if (ImGui::MenuItem("Reset Position"))  { m_vCurObjPos = { 0.f, 0.f, 0.f }; }
                if (ImGui::MenuItem("Reset Rotation"))  { m_vCurObjRot = { 0.f, 0.f, 0.f }; }
                if (ImGui::MenuItem("Reset Scale"))     { m_vCurObjSca = { targetImgSize[0].x, targetImgSize[0].y, 1.f}; }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Transform")) {
                    m_vCurObjPos = { 0.f, 0.f, 0.f };
                    m_vCurObjRot = { 0.f, 0.f, 0.f };
                    m_vCurObjSca = { 1.f, 1.f, 1.f };
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();

            ImGui::PushItemWidth(60);

            // Position Ctrl
            ImGui::Text("Position");
            ImGui::DragFloat("X##pos", &m_vCurObjPos.x, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Y##pos", &m_vCurObjPos.y, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Z##pos", &m_vCurObjPos.z, fSensitivity);
            ImGui::Separator();

            // Rotation Ctrl
            ImGui::Text("Rotation");
            ImGui::DragFloat("X##rot", &m_vCurObjRot.x, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Y##rot", &m_vCurObjRot.y, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Z##rot", &m_vCurObjRot.z, fSensitivity);
            ImGui::Separator();

            // Scale Ctrl
            ImGui::Text("Scale");
            ImGui::DragFloat("X##sca", &m_vCurObjSca.x, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Y##sca", &m_vCurObjSca.y, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Z##sca", &m_vCurObjSca.z, fSensitivity);
            ImGui::Separator();

            ImGui::PopItemWidth();
        }


        _matrix matXMEditPosition = XMMatrixTranslationFromVector(XMLoadFloat3(&m_vCurObjPos));
        _matrix matXMEditRotation = XMMatrixRotationRollPitchYaw(TO_RAD(m_vCurObjRot.x), TO_RAD(m_vCurObjRot.y), TO_RAD(m_vCurObjRot.z));
        _matrix matXMEditScale = XMMatrixScalingFromVector(XMLoadFloat3(&m_vCurObjSca));

        _matrix matXMEditResult = matXMEditScale * matXMEditRotation * matXMEditPosition;

        // UI ?댁쓽 Begin ?뚮Ц???곸슜 ?덈릺?붾벏. ?꾩떆濡?鍮꾪솢?깊솕??
        if (!m_isPlayAnimation)
            pTargetTransform->Set_WorldMatrix(matXMEditResult);
    }

#pragma endregion

#pragma region [Other] Description Edit

    if (ImGui::CollapsingHeader("Edit UI Desciption"))
    {
        static _char    szUIName[256] = {};
        static _uint    iUIType = {};
        static _char    szParentName[256] = {};
        
        static _uint    iPassType = 2;
        static _float2  vScreenLT = {};
        static _float2  vScreenRB = { g_iWinSizeX, g_iWinSizeY };
        static _bool    isInverseScreenDiscard = false;
        static _float   fCutout = 0.3f;

        static _float2  vSectorBorder = {};
        static _float   fUIScale = 1.f;
        

        CCustom_UI::CUSTOM_UI_DESC tDesc = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc();


        // 湲곗〈 媛?諛섏쁺
        _string strUIName = _string(tDesc.strUIName.begin(), tDesc.strUIName.end());
        strcpy_s(szUIName, strUIName.c_str());

        iUIType = tDesc.iUIType;

        _string strParentName = _string(tDesc.strParentName.begin(), tDesc.strParentName.end());
        strcpy_s(szParentName, strParentName.c_str());

        iPassType = tDesc.iPassType;
        isInverseScreenDiscard = tDesc.isInverseScreenDiscard;
        fCutout = tDesc.fCutout;

        vSectorBorder = tDesc.vSectorBorder;
        fUIScale = tDesc.fUIScale;


        ImGui::Text("UI Name");
        ImGui::InputText("##UI Name", szUIName, 256);

        ImGui::Separator();

        // iUIType
        ImGui::Text("UI Type");
        const char* szUITypeNames[] = { "NONE", "BUTTON", "INTERACT" };
        const _uint iTypeCount = ENUM_CLASS(CCustom_UI::UI_TYPE::END);
        const char* szCurrentItem = szUITypeNames[iUIType];

        if (ImGui::BeginCombo("##UI Type", szCurrentItem))
        {
            for (_uint i = 0; i < iTypeCount; ++i)
            {
                const _bool isSelected = (iUIType == i);
                if (ImGui::Selectable(szUITypeNames[i], isSelected))
                    iUIType = i;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        // szParentName
        ImGui::Text("Parent Name");
        ImGui::InputText("##Parent Name", szParentName, 256);

        ImGui::Separator();

        // iPassType
        ImGui::Text("Pass Type");
        const char* szPassTypeNames[] = { "Normal", "Cutout", "Transparent", "Gradient", "Grad_9Sec", "Variant"};
        const _uint iPassTypeCount = 6;
        const char* szCurrentPassItem = szPassTypeNames[iPassType];

        if (ImGui::BeginCombo("##Pass Type", szCurrentPassItem))
        {
            for (_uint i = 0; i < iPassTypeCount; ++i)
            {
                const _bool isSelected = (iPassType == i);
                if (ImGui::Selectable(szPassTypeNames[i], isSelected))
                    iPassType = i;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        // isInverseScreenDiscard
        ImGui::Text("isInverseScreenDiscard");
        ImGui::Checkbox("##isInverseScreenDiscard", &isInverseScreenDiscard);
        ImGui::Separator();


        // fCutout
        if (iPassType == 1)
        {
            ImGui::Text("Cutout Rate");
            ImGui::DragFloat("##CutoutRate", &fCutout, 0.001f, 0.f, 1.f);
            ImGui::Separator();
        }

        // vSector, fScale
        if (iPassType == 4)
        {
            // vSector
            ImGui::PushItemWidth(90.f);

            ImGui::Text("Sector Border");
            ImGui::Text("X");
            ImGui::SameLine();
            ImGui::DragFloat("##SectorBorderX", &vSectorBorder.x, 0.1f, 0.f, 0.f, "%.1f");
            ImGui::SameLine();
            ImGui::Text("Y");
            ImGui::SameLine();
            ImGui::DragFloat("##SectorBorderY", &vSectorBorder.y, 0.1f, 0.f, 0.f, "%.1f");

            ImGui::PopItemWidth();
            ImGui::Separator();
            
            // fScale
            ImGui::Text("UI Sector Scale");
            ImGui::Text("Scale");
            ImGui::SameLine();
            ImGui::DragFloat("##SectorScale", &fUIScale, 0.001f);
        }


        // ?ㅼ떆 媛??좊떦
        _string strEditedUIName = szUIName;
        tDesc.strUIName = _wstring(strEditedUIName.begin(), strEditedUIName.end());

        tDesc.iUIType = iUIType;

        _string strEditedParentName = szParentName;
        tDesc.strParentName = _wstring(strEditedParentName.begin(), strEditedParentName.end());

        tDesc.iPassType = iPassType;
        tDesc.isInverseScreenDiscard  = isInverseScreenDiscard;
        tDesc.fCutout = fCutout;

        tDesc.vSectorBorder = vSectorBorder;
        tDesc.fUIScale = fUIScale;

        dynamic_cast<CCustom_UI*>(m_pCurObj)->Set_UIDesc(tDesc);
    }
#pragma endregion

#pragma region [Other] AnimEdit Toggle

    if (ImGui::CollapsingHeader("Animation Editor"))
    {
        if (ImGui::Button("Open Anim Editor", ImVec2(200.f, 20.f)))
            m_isOn_AnimEdit = !m_isOn_AnimEdit;
    }

#pragma endregion

    ImGui::End();
}

void CLevel_UI::Update_AnimEditor(_float fTimeDelta)
{
    if (!m_pCurObj ||
        !m_isOn_AnimEdit)
        return;

    static _int iKeyFrame = 0;
    static _int iRecentKeyFrame = 0;

    static _int iTexIndex = 0;
    static _int iMaxTexIndex = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc().iNumFiles;
    static _float fAlpha = 0.f;

    static _float2 vScreenLT = {};			// ǥ�õ� ȭ������ ��ǥ ����. (���� 0, 0 / ���� ȭ��ũ��)
    static _float2 vScreenRB = { g_iWinSizeX, g_iWinSizeY };
    static _float4 vBlendToOuterWidth = {};


    static _int iAnimEditorSelected = -1;
    static _int iAnimListSelected = -1;


    CCustom_UI* pTargetUI = dynamic_cast<CCustom_UI*>(m_pCurObj);
    CAnimator_UI* pTargetAnimator = dynamic_cast<CAnimator_UI*>(pTargetUI->Get_Component(L"Com_Animator_UI"));


    UI_ANIM_KEYFRAME_DESC tKeyFrameDesc = {};

    ImGui::Begin("Animation Editor");

#pragma region Add Keyframe Menu

    if (ImGui::CollapsingHeader("Add Menu"))
    {
        // iKeyFrame
        ImGui::Text("Keyframe Index");
        ImGui::InputInt("##KeyFrame Index", &iKeyFrame);
        if          (m_vecUIKeyFrameDescs.empty())  iKeyFrame = 0;
        else if     (m_pSelectedKeyFrameDesc)       {}
        else if     (iRecentKeyFrame >= iKeyFrame)  iKeyFrame = iRecentKeyFrame + 1;

        ImGui::Separator();

        // iTexIndex
        ImGui::Text("Texture Index (NumTex : %d)", iMaxTexIndex);
        ImGui::InputInt("##Texture Index", &iTexIndex);
        if          (iTexIndex < 0)                 iTexIndex = 0;
        else if     (m_pSelectedKeyFrameDesc)       {}
        else if     (iMaxTexIndex <= iTexIndex)     iTexIndex = iMaxTexIndex - 1;

        ImGui::Separator();

        // fAlpha
        ImGui::Text("Texture Alpha");
        ImGui::DragFloat("##Texture Alpha", &fAlpha, 0.001f, 0.f, 1.f);

        ImGui::Separator();

        ImGui::Text("Lerp Type");

        const char* szLerpTypeNames[] = {
            "LINEAR",
            "LT",    
            "RB",
            "CUBIC"
        };

        // LerpType (m)
        if (ImGui::BeginCombo("Lerp Type##LerpType", szLerpTypeNames[m_iLerpType]))
        {
            for (_uint i = 0; i < IM_ARRAYSIZE(szLerpTypeNames); i++)
            {
                // ?꾩옱 ?좏깮 ?щ?
                _bool isSelected = (m_iLerpType == i);

                if (ImGui::Selectable(szLerpTypeNames[i], isSelected))
                    m_iLerpType = i; // ?좏깮 ??媛??낅뜲?댄듃

                // ?좏깮????ぉ??泥댄겕 ?쒖떆
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
            
        ImGui::Separator();

        // Loop (m)
        ImGui::Checkbox("Loop", &m_isAnimLoop);

        ImGui::Separator();

        // vScreenLT, vScreenRB
        ImGui::PushItemWidth(60.f);
        ImGui::Text("Screen Clip");

        ImGui::Text("LT | X");
        ImGui::SameLine();
        ImGui::DragFloat("##ClipLT_X", &vScreenLT.x, 1.f, 0.f, vScreenRB.x);
        ImGui::SameLine();
        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::DragFloat("##ClipLT_Y", &vScreenLT.y, 1.f, 0.f, vScreenRB.y);

        ImGui::Text("RB | X");
        ImGui::SameLine();
        ImGui::DragFloat("##ClipRB_X", &vScreenRB.x, 1.f, vScreenLT.x, g_iMaxWidth);
        ImGui::SameLine();
        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::DragFloat("##ClipRB_Y", &vScreenRB.y, 1.f, vScreenLT.y, g_iMaxHeight);
        ImGui::SameLine();

        ImGui::PopItemWidth();
        ImGui::Separator();

        // vBlendToOuterWidth 
        ImGui::PushItemWidth(40.f);
        ImGui::Text("Blend Outer Width");
        ImGui::Text("L");
        ImGui::SameLine();
        ImGui::DragFloat("##Blend_L", &vBlendToOuterWidth.x, 1.f);
        ImGui::SameLine();
        ImGui::Text("R");
        ImGui::SameLine();
        ImGui::DragFloat("##Blend_R", &vBlendToOuterWidth.y, 1.f);
        ImGui::SameLine();
        ImGui::Text("T");
        ImGui::SameLine();
        ImGui::DragFloat("##Blend_T", &vBlendToOuterWidth.z, 1.f);
        ImGui::SameLine();
        ImGui::Text("B");
        ImGui::SameLine();
        ImGui::DragFloat("##Blend_B", &vBlendToOuterWidth.w, 1.f);
        ImGui::SameLine();

        ImGui::PopItemWidth();
        ImGui::Separator();



        // Add / Edit / Delete
        if (m_pSelectedKeyFrameDesc == nullptr)
        {
            if (ImGui::Button("Add Keyframe"))
            {
                UI_ANIM_KEYFRAME_DESC tTempDesc = {
                    (_uint)iKeyFrame,
                    (_uint)m_iLerpType,
                    (_uint)iTexIndex,
                    fAlpha,
                    m_vCurObjPos,
                    m_vCurObjRot,
                    m_vCurObjSca,
                    vScreenLT,
                    vScreenRB,
                    vBlendToOuterWidth
                };

                m_vecUIKeyFrameDescs.push_back(tTempDesc);
            }
            
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,          ImVec4(0.0f, 0.0f, 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,    ImVec4(0.0f, 0.0f, 0.9f, 1.0f));
            if (ImGui::Button("Instant Load Anim"))
            {
                UI_ANIM_DESC tAnimDesc = {};
                tAnimDesc.tUIDesc = dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc();
                tAnimDesc.strAnimName = L"Instant Animation";
                tAnimDesc.isLoop = m_isAnimLoop;
                for (auto& keyframeDesc : m_vecUIKeyFrameDescs)
                    tAnimDesc.vecKeyFrames.push_back(keyframeDesc);

                if (pTargetAnimator->Find_Animation(tAnimDesc.strAnimName))
                    pTargetAnimator->Remove_Animation(tAnimDesc.strAnimName);

                pTargetAnimator->Insert_Animation(tAnimDesc);
            }
            ImGui::PopStyleColor(3);
        }
        else
        {
            if (ImGui::Button("Edit Keyframe"))
            {
                m_pSelectedKeyFrameDesc->fAlpha = fAlpha;
                m_pSelectedKeyFrameDesc->iKeyframeIndex = iKeyFrame;
                m_pSelectedKeyFrameDesc->iTexIndex = iTexIndex;
                m_pSelectedKeyFrameDesc->iLerpType = m_iLerpType;

                m_pSelectedKeyFrameDesc->vPos = m_vCurObjPos;
                m_pSelectedKeyFrameDesc->vRot = m_vCurObjRot;
                m_pSelectedKeyFrameDesc->vSca = m_vCurObjSca;

                m_pSelectedKeyFrameDesc->vScreenLT = vScreenLT;
                m_pSelectedKeyFrameDesc->vScreenRB = vScreenRB;
                m_pSelectedKeyFrameDesc->vBlendToOuterWidth = vBlendToOuterWidth;

                m_pSelectedKeyFrameDesc = nullptr;
                iRecentKeyFrame = m_vecUIKeyFrameDescs.back().iKeyframeIndex;
            }
            ImGui::SameLine();
            if (ImGui::Button("Deselect##KeyFrame Deselect"))
            {
                m_pSelectedKeyFrameDesc = nullptr;
                iRecentKeyFrame = m_vecUIKeyFrameDescs.back().iKeyframeIndex;
            }
            if (ImGui::CollapsingHeader("Danger Section##Danger Section"))
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.0f, 0.0f, 1.0f));
                if (ImGui::Button("Remove Keyframe"))
                {
                    for (_uint i = 0; i < m_vecUIKeyFrameDescs.size(); i++)
                        if (&m_vecUIKeyFrameDescs[i] == m_pSelectedKeyFrameDesc)
                        {
                            m_vecUIKeyFrameDescs.erase(m_vecUIKeyFrameDescs.begin() + i);
                            m_pSelectedKeyFrameDesc = nullptr;
                            break;
                        }
                }
                ImGui::PopStyleColor(3);
            }
        }

    }

#pragma endregion

#pragma region Keyframe List

    if (ImGui::CollapsingHeader("Keyframe List"))
    {
        const char* szLerpTypeNames[] = {
            "LN-",
            "LT",    
            "RB",
            "CB~"
        };

        if (m_vecUIKeyFrameDescs.empty())
            ImGui::Selectable("(Empty)##AnimEdit", false);

        for (_uint i = 0; i < m_vecUIKeyFrameDescs.size(); i++)
        {
            _string strLabel = "Keyframe [" + to_string(i + 1) + "] \t| [" + to_string(m_vecUIKeyFrameDescs[i].iKeyframeIndex) + "] \t| [" + szLerpTypeNames[m_vecUIKeyFrameDescs[i].iLerpType] + "]";
            if (ImGui::Selectable(strLabel.c_str(), iAnimEditorSelected == i))
            {
                m_pSelectedKeyFrameDesc = &m_vecUIKeyFrameDescs[i];

                iKeyFrame = m_pSelectedKeyFrameDesc->iKeyframeIndex;
                iTexIndex = m_pSelectedKeyFrameDesc->iTexIndex;
                fAlpha = m_pSelectedKeyFrameDesc->fAlpha;
                m_iLerpType = m_pSelectedKeyFrameDesc->iLerpType;

                m_vCurObjPos = m_pSelectedKeyFrameDesc->vPos;
                m_vCurObjRot = m_pSelectedKeyFrameDesc->vRot;
                m_vCurObjSca = m_pSelectedKeyFrameDesc->vSca;

                vScreenLT = m_pSelectedKeyFrameDesc->vScreenLT;
                vScreenRB = m_pSelectedKeyFrameDesc->vScreenRB;
                vBlendToOuterWidth = m_pSelectedKeyFrameDesc->vBlendToOuterWidth;
            }
        }



    }

#pragma endregion

#pragma region Animation List

    if (ImGui::CollapsingHeader("Animation List"))
    {

        if (m_pSelectedUIAnim != nullptr &&
            !m_isPlayAnimation)
        {
            if (ImGui::Button("Play Animation"))
                m_isPlayAnimation = true;
        }
        else if (m_pSelectedUIAnim != nullptr &&
            m_isPlayAnimation)
        {
            if (ImGui::Button("Stop Animation"))
                m_isPlayAnimation = false;
        }
        else if (ImGui::Button("Nothing Selected")) {}


        if (m_pSelectedUIAnim)
        {
            ImGui::SameLine();
            if (ImGui::Button("Deselect##AnimList Deselect"))
                m_pSelectedUIAnim = nullptr;
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.0f, 0.0f, 1.0f));
            if (ImGui::Button("Delete##AnimList AnimDelete"))
            {
                pTargetAnimator->Remove_Animation(m_pSelectedUIAnim->strAnimName);
                m_pSelectedUIAnim = nullptr;
            }
            ImGui::PopStyleColor(3);
        }


        if (pTargetAnimator->Find_Animation(0) == nullptr)
            ImGui::Selectable("(Empty)##AnimList", false);

        _uint iIndex = 0;
        while (true)
        {
            CLevel_UI::UI_ANIM_DESC* pDesc = pTargetAnimator->Find_Animation(iIndex);
            if (!pDesc) break;

            _string strLabel = "Anim [" + to_string(iIndex) + "] | [" + _string(pDesc->strAnimName.begin(), pDesc->strAnimName.end()) + "]";
            if (ImGui::Selectable(strLabel.c_str(), iAnimListSelected == iIndex))
            {
                m_pSelectedUIAnim = pDesc;
            }

            iIndex++;
        }

    }

#pragma endregion

    ImGui::End();



#pragma region Update Animation
    if (pTargetUI &&
        pTargetAnimator->Find_Animation(0) != nullptr &&
        m_isPlayAnimation)
    {
        if (pTargetAnimator->Get_CurAnimation() == nullptr)
        {
            if (m_pSelectedUIAnim != nullptr)
                pTargetAnimator->Change_Animation(m_pSelectedUIAnim->strAnimName);
        }
    }
    else
    {
        pTargetAnimator->Deselect_Animation();
    }

#pragma endregion



}

void CLevel_UI::Update_InstanceEditor()
{
    if (m_pCurObj == nullptr ||
        !dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc().isInstance)
        return;

    ImGui::Begin("Instance Editor");

#pragma region [Instance] Transform

    // ==============================
    // * [Instance] Transform
    // ==============================
    vector<CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC>* pDescs = &dynamic_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc().vecInstanceDescs;
    static _uint iInstSelected = 0;

    static _float4 vSInstRight = { 1.f, 0.f, 0.f ,0.f };
    static _float4 vSInstUp    = { 0.f, 1.f, 0.f ,0.f };
    static _float4 vSInstLook  = { 0.f, 0.f, 1.f ,0.f };
    static _float4 vSInstTrans = { 0.f, 0.f, 0.f ,1.f };
    // to transform
    static _float3 curInstPos = {};
    static _float3 curInstRot = {};
    static _float3 curInstSca = {};

    static _float2 vTexcoordX = { 0, 0 };
    static _float2 vTexcoordY = { 1, 1 };
    static _float2 vClipTexcoordX = { 0, 0 };
    static _float2 vClipTexcoordY = { 1, 1 };



    if (ImGui::CollapsingHeader("[Instance] Transform"))
    {
        // Add Instance
        if (ImGui::Button("Add"))
        {
            CVIBuffer_Rect_Instance_UI::SINGLE_INST_DESC tDesc = {};
            pDescs->push_back(tDesc);
            m_pSelectedInstance = &pDescs->back();
        }
        ImGui::SameLine();
        if (m_pSelectedInstance &&
            ImGui::Button("Unselect"))
        {
            m_pSelectedInstance = nullptr;
        }

        if (ImGui::CollapsingHeader("Danger Section##Instance Danger"))
        {
            if (!pDescs->empty() &&
                ImGui::Button("Delete"))
                pDescs->erase(pDescs->begin() + pDescs->size() - 1);
        }

        if (m_pSelectedInstance)
        {
            // Extracting Float3 Editable Values from Instance.
            _matrix matTransform = _matrix(
                XMLoadFloat4(&vSInstRight),
                XMLoadFloat4(&vSInstUp),
                XMLoadFloat4(&vSInstLook),
                XMLoadFloat4(&vSInstTrans)
            );

            _vector		vXMObjPosition = {}, vXMObjQuaternion = {}, vXMObjScale = {};
            _float3		vStoreObjPosition = {}, vStoreObjRotation = {}, vStoreObjScale = {};
            XMMatrixDecompose(&vXMObjScale, &vXMObjQuaternion, &vXMObjPosition, matTransform);

            _float4x4	matStoreObjQuaternion = {};	// quat
            XMStoreFloat4x4(&matStoreObjQuaternion, QUAT_TO_MAT(vXMObjQuaternion));

            XMStoreFloat3(&vStoreObjPosition, vXMObjPosition);
            vStoreObjRotation = MAT_TO_ROT(matStoreObjQuaternion);
            XMStoreFloat3(&vStoreObjScale, vXMObjScale);


            // Apply to Editor
            curInstPos = vStoreObjPosition;
            curInstRot = vStoreObjRotation;
            curInstSca = vStoreObjScale;



            // ==============================
            // * ImgUI
            // ==============================
            ImGui::Text("Inst Transform");
            ImGui::Separator();

            static _float fSensitivity = 1.f;
            ImGui::Text("Sensitivity");
            ImGui::SameLine();
            ImGui::DragFloat("##Sensitivity", &fSensitivity, 0.001f, 0.001f, 10.f);
            ImGui::Separator();

            if (ImGui::BeginMenu("Reset Menu"))
            {
                vector<_float2> targetImgSize = static_cast<CCustom_UI*>(m_pCurObj)->Get_UIDesc().vecSize;

                if (ImGui::MenuItem("Reset Position")) { curInstPos = { 0.f, 0.f, 0.f }; }
                if (ImGui::MenuItem("Reset Rotation")) { curInstRot = { 0.f, 0.f, 0.f }; }
                if (ImGui::MenuItem("Reset Scale")) { curInstSca = { targetImgSize[0].x, targetImgSize[0].y, 1.f }; }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Transform")) {
                    curInstPos = { 0.f, 0.f, 0.f };
                    curInstRot = { 0.f, 0.f, 0.f };
                    curInstSca = { 1.f, 1.f, 1.f };
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();

            ImGui::PushItemWidth(60);

            // Position Ctrl
            ImGui::Text("Position");
            ImGui::DragFloat("X##pos", &curInstPos.x, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Y##pos", &curInstPos.y, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Z##pos", &curInstPos.z, fSensitivity);
            ImGui::Separator();

            // Rotation Ctrl
            ImGui::Text("Rotation");
            ImGui::DragFloat("X##rot", &curInstRot.x, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Y##rot", &curInstRot.y, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Z##rot", &curInstRot.z, fSensitivity);
            ImGui::Separator();

            // Scale Ctrl
            ImGui::Text("Scale");
            ImGui::DragFloat("X##sca", &curInstSca.x, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Y##sca", &curInstSca.y, fSensitivity);   ImGui::SameLine();
            ImGui::DragFloat("Z##sca", &curInstSca.z, fSensitivity);
            ImGui::Separator();

            ImGui::PopItemWidth();

            // Apply Values to [Instance]
            _matrix matXMEditPosition = XMMatrixTranslationFromVector(XMLoadFloat3(&curInstPos));
            _matrix matXMEditRotation = XMMatrixRotationRollPitchYaw(TO_RAD(curInstRot.x), TO_RAD(curInstRot.y), TO_RAD(curInstRot.z));
            _matrix matXMEditScale = XMMatrixScalingFromVector(XMLoadFloat3(&curInstSca));

            _matrix matXMEditResult = matXMEditScale * matXMEditRotation * matXMEditPosition;
            _float4x4 matEditResult; XMStoreFloat4x4(&matEditResult, matXMEditResult);

            vSInstRight = XMFLOAT4(matEditResult.m[0][0], matEditResult.m[0][1], matEditResult.m[0][2], matEditResult.m[0][3]);
            vSInstUp    = XMFLOAT4(matEditResult.m[1][0], matEditResult.m[1][1], matEditResult.m[1][2], matEditResult.m[1][3]);
            vSInstLook  = XMFLOAT4(matEditResult.m[2][0], matEditResult.m[2][1], matEditResult.m[2][2], matEditResult.m[2][3]);
            vSInstTrans = XMFLOAT4(matEditResult.m[3][0], matEditResult.m[3][1], matEditResult.m[3][2], matEditResult.m[3][3]);

            m_pSelectedInstance->vSInstRight        = vSInstRight ;
            m_pSelectedInstance->vSInstUp           = vSInstUp    ;
            m_pSelectedInstance->vSInstLook         = vSInstLook  ;
            m_pSelectedInstance->vSInstTrans        = vSInstTrans ;
            //m_pSelectedInstance->vSInstCoordX       = vTexcoordX  ;
            //m_pSelectedInstance->vSInstCoordY       = vTexcoordY  ;
            //m_pSelectedInstance->vClipTexcoordX     = vClipTexcoordX  ;
            //m_pSelectedInstance->vClipTexcoordY     = vClipTexcoordY  ;
        }

    }
#pragma endregion

    // ==============================
    // * Additional Desc
    // ==============================
    if (ImGui::CollapsingHeader("[Instance] Desc Change"))
    {
        if (m_pSelectedInstance)
        {
            // vSInstCoordX	    : float2 data, for slicing atlas / sprite style images.
            // vSInstCoordY	    : float2 data, for slicing atlas / sprite style images.
            // vClipTexcoordX   : float2 data, for discarding pixel based on local space. like as HP Bar Value.
            // vClipTexcoordY   : float2 data, for discarding pixel based on local space. like as HP Bar Value.
            ImGui::PushItemWidth(90.f);

            ImGui::Text("Slice by ImagePos");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("for slicing atlas / sprite style images.");
            
            ImGui::Text("Pos X Range | ");
            ImGui::SameLine();
            ImGui::DragFloat("##SliceImagePosStartX", &m_pSelectedInstance->vSInstCoordX.x, 0.001f ,0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::DragFloat("##SliceImagePosEndX", &m_pSelectedInstance->vSInstCoordX.y, 0.001f ,0.0f, 1.0f);
            
            ImGui::Text("Pos Y Range | ");
            ImGui::SameLine();
            ImGui::DragFloat("##SliceImagePosStartY", &m_pSelectedInstance->vSInstCoordY.x, 0.001f, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::DragFloat("##SliceImagePosEndY", &m_pSelectedInstance->vSInstCoordY.y, 0.001f, 0.0f, 1.0f);
            
            
            ImGui::Separator();
            ImGui::Text("Discard by LocalPos");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                ImGui::SetTooltip("for discarding pixel based on local space. like as HP Bar Value.");
            
            ImGui::Text("Pos X Range | ");
            ImGui::SameLine();
            ImGui::DragFloat("##DiscardLocalPosStartX", &m_pSelectedInstance->vClipTexcoordX.x, 0.001f, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::DragFloat("##DiscardLocalPosEndX", &m_pSelectedInstance->vClipTexcoordX.y, 0.001f, 0.0f, 1.0f);
            
            ImGui::Text("Pos Y Range | ");
            ImGui::SameLine();
            ImGui::DragFloat("##DiscardLocalPosStartY", &m_pSelectedInstance->vClipTexcoordY.x, 0.001f, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::DragFloat("##DiscardLocalPosEndY", &m_pSelectedInstance->vClipTexcoordY.y, 0.001f, 0.0f, 1.0f);

            ImGui::PopItemWidth();
        }
        else
        {
            ImGui::Text("Instance not selected.");
        }
    }




    // ==============================
    // * Instance List
    // ==============================
    if (ImGui::CollapsingHeader("Instance List"))
    {
        // ����
        if (pDescs->empty())
            ImGui::Selectable("(Empty)#InstEdit", false);

        for (_uint i = 0; i < pDescs->size(); i++)
        {
            _string strLabel = "Instance [" + to_string(i + 1) + "]";


            // �̰� �ȳ����°� �ذ��غ��� �� ������� �����غ��� ���� ����
            if (ImGui::Selectable(strLabel.c_str(), iInstSelected == i))
            {
                m_pSelectedInstance = &(*pDescs)[i];

                vSInstRight     = m_pSelectedInstance->vSInstRight;
                vSInstUp        = m_pSelectedInstance->vSInstUp;
                vSInstLook      = m_pSelectedInstance->vSInstLook;
                vSInstTrans     = m_pSelectedInstance->vSInstTrans;
                vTexcoordX      = m_pSelectedInstance->vSInstCoordX;
                vTexcoordY      = m_pSelectedInstance->vSInstCoordY;
                vClipTexcoordX  = m_pSelectedInstance->vClipTexcoordX;
                vClipTexcoordY  = m_pSelectedInstance->vClipTexcoordY;

                _matrix matNewTransform = _matrix(
                    XMLoadFloat4(&vSInstRight),
                    XMLoadFloat4(&vSInstUp),
                    XMLoadFloat4(&vSInstLook),
                    XMLoadFloat4(&vSInstTrans)
                );

                _vector		vXMNewObjPosition = {}, vXMNewObjQuaternion = {}, vXMNewObjScale = {};
                _float3		vStoreNewObjPosition = {}, vStoreNewObjRotation = {}, vStoreNewObjScale = {};
                XMMatrixDecompose(&vXMNewObjScale, &vXMNewObjQuaternion, &vXMNewObjPosition, matNewTransform);

                _float4x4	matStoreNewObjQuaternion = {};	// ���ʹϾ�
                XMStoreFloat4x4(&matStoreNewObjQuaternion, QUAT_TO_MAT(vXMNewObjQuaternion));

                XMStoreFloat3(&vStoreNewObjPosition, vXMNewObjPosition);
                vStoreNewObjRotation = MAT_TO_ROT(matStoreNewObjQuaternion);
                XMStoreFloat3(&vStoreNewObjScale, vXMNewObjScale);


                // �����Ͽ� ������
                curInstPos = vStoreNewObjPosition;
                curInstRot = vStoreNewObjRotation;
                curInstSca = vStoreNewObjScale;
            }
        }
    }

    ImGui::End();
}

void CLevel_UI::Update_ObjectParents()
{
    // ==============================
    // * 遺紐??ㅻ툕?앺듃 ?낅뜲?댄듃
    // ==============================

    for (auto& UIObject : m_vecCustomUIs)
    {
        CCustom_UI::CUSTOM_UI_DESC tDesc = UIObject.pCustomUI->Get_UIDesc();
        if (tDesc.strParentName.empty())
        {
            tDesc.pParentObject = nullptr;
            UIObject.pCustomUI->Set_UIDesc(tDesc);

            continue;
        }

        for (auto& compareUIObject : m_vecCustomUIs)
        {
            if (tDesc.strParentName == compareUIObject.pCustomUI->Get_UIDesc().strUIName)
            {
                tDesc.pParentObject = compareUIObject.pCustomUI;
                UIObject.pCustomUI->Set_UIDesc(tDesc);
                break;
            }
        }
    }
}

void CLevel_UI::Update_ObjectChilds()
{
    // ==============================
    // * ?먯떇 ?ㅻ툕?앺듃 ?낅뜲?댄듃
    // (Client瑜??꾪븿, Editor ?먯꽌??誘몄궗?? ????쒖뿉留??ъ슜)
    // ==============================

    for (auto& UIObject : m_vecCustomUIs)
    {
        CCustom_UI::CUSTOM_UI_DESC tDesc = UIObject.pCustomUI->Get_UIDesc();
        vector<_wstring> vecChilds = {};

        // 紐⑤뱺 媛앹껜瑜??쒗쉶?섎ŉ, ?대떦 媛앹껜瑜?遺紐⑤줈 媛吏??먯떇???덈떎硫?
        // ?대떦 ?먯떇???대쫫??遺紐⑥뿉 異붽?.

        for (auto& compareUIObject : m_vecCustomUIs)
        {
            if (tDesc.strUIName == compareUIObject.pCustomUI->Get_UIDesc().strParentName)
                vecChilds.push_back(compareUIObject.pCustomUI->Get_UIDesc().strUIName);
        }

        tDesc.vecChildNames = vecChilds;
        UIObject.pCustomUI->Set_UIDesc(tDesc);
    }
}

void CLevel_UI::Update_SelectedKeyframeDesc()
{
    //_bool isCheck = false;
    //Ȯ���Ұ�;
    
    if (m_pCurObj == nullptr || m_pSelectedKeyFrameDesc == nullptr)
        return;
    CCustom_UI* curObj = dynamic_cast<CCustom_UI*>(m_pCurObj);
    CShader* curObjShader = dynamic_cast<CShader*>(curObj->Get_Component(L"Com_Shader"));
    
    curObjShader->Bind_Value("g_AlphaStrength", &m_pSelectedKeyFrameDesc->fAlpha, sizeof(m_pSelectedKeyFrameDesc->fAlpha));
    curObjShader->Bind_Value("g_ScreenLT", &m_pSelectedKeyFrameDesc->vScreenLT, sizeof(m_pSelectedKeyFrameDesc->vScreenLT));
    curObjShader->Bind_Value("g_ScreenRB", &m_pSelectedKeyFrameDesc->vScreenRB, sizeof(m_pSelectedKeyFrameDesc->vScreenRB));
    curObjShader->Bind_Value("g_BlendToOuterWidth", &m_pSelectedKeyFrameDesc->vBlendToOuterWidth, sizeof(m_pSelectedKeyFrameDesc->vBlendToOuterWidth));
    

}

CLevel_UI* CLevel_UI::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_UI* pInstance = new CLevel_UI(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_UI");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_UI::Free()
{
    for (auto& UIObject : m_vecCustomUIs)
        Safe_Release(UIObject.pCustomUI);

    __super::Free();
}
