#include "EditorPch.h"
#include "Sequencer.h"

#include "Event_Scene_Edit.h"

#include "Sequence.h"

#include "SQ_Camera_Edit.h"

CSequencer::CSequencer()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

void CSequencer::Get(_int index, _int** start, _int** end, _int* type, _uint* color)
{
	if (0 == m_Items.size())
		return;

	SEQUENCE_ITEM& item = m_Items[index];
	if (nullptr != color)
		*color = 0xFFAA8080;
	if (nullptr != start)
		*start = &item.iFrameStart;
	if(nullptr != end)
		*end = &item.iFrameEnd;
	if (nullptr != type)
		*type = ENUM_CLASS(item.eType);
}

void CSequencer::Add(_int iType)
{
	switch (iType)
	{
	case ENUM_CLASS(ITEM_TYPE::ACTION):
		m_Items.push_back(SEQUENCE_ITEM{ iType, 10, 30, true, "Action" });
		break;
	case ENUM_CLASS(ITEM_TYPE::SCENE):
		m_Items.push_back(SEQUENCE_ITEM{ iType, 10, 30, true, "Scene" });
		break;
	case ENUM_CLASS(ITEM_TYPE::SOUND):
		m_Items.push_back(SEQUENCE_ITEM{ iType, 10, 30, false, "Sound" });
		break;
	case ENUM_CLASS(ITEM_TYPE::SFX):
		m_Items.push_back(SEQUENCE_ITEM{ iType, 10, 30, false, "SFX" });
		break;
	case ENUM_CLASS(ITEM_TYPE::ACTOR):
		m_Items.push_back(SEQUENCE_ITEM{ iType, 10, 30, true, "Actor" });
		break;
	case ENUM_CLASS(ITEM_TYPE::EFFECT):
		m_Items.push_back(SEQUENCE_ITEM{ iType, 10, 30, true, "Effect" });
		break;
	}

}

const _char* CSequencer::GetItemTypeName(_int iIndex) const
{
	switch (iIndex)
	{
	case ENUM_CLASS(ITEM_TYPE::ACTION):
		return "Action";
	case ENUM_CLASS(ITEM_TYPE::SCENE):
		return "Scene";
	case ENUM_CLASS(ITEM_TYPE::SOUND):
		return "Sound";
	case ENUM_CLASS(ITEM_TYPE::SFX):
		return "SFX";
	case ENUM_CLASS(ITEM_TYPE::ACTOR):
		return "Actor";
	case ENUM_CLASS(ITEM_TYPE::EFFECT):
		return "Effect";
	}
}

void CSequencer::CustomDraw(RampEdit& delegate, _int iIndex, const ImRect& customRect, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect)
{
	if (m_iSelectedEntry != iIndex)
		return;

	delegate.mMin = ImVec2(static_cast<_float>(m_iFrameMin), 0.f);
	delegate.mMax = ImVec2(static_cast<_float>(m_iFrameMax), 1.f);
	m_pDrawList->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);

	for (_int i = 0; i < 3; ++i)
	{
		ImVec2 ptA(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
		ImVec2 ptB(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
		m_pDrawList->AddText(ptA, delegate.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, m_pCustomDrawLabel[i]);
		if (ImRect(ptA, ptB).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
			delegate.mbVisible[i] = !delegate.mbVisible[i];
	}

	m_pDrawList->PopClipRect();

	ImGui::SetCursorScreenPos(customRect.Min);
	ImCurveEdit::Edit(delegate, customRect.Max - customRect.Min, 137 + iIndex, &clippingRect);

	// Custom Rect ���� MousePos Compute
	_float fMouseX = (m_fFramePixelWidth * m_iFirstFrame + io.MousePos.x - customRect.Min.x) / m_fFramePixelWidth;
	_float fMouseY = 1.f - (io.MousePos.y - customRect.Min.y) / (customRect.Max.y - customRect.Min.y);
	// Point Click
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::DOWN)
	{

		for (_uint i = 0; i < 3; ++i)
		{
			for (size_t j = 0; j < delegate.GetPointCount(i); ++j)
			{
				ImVec2 vPoint = delegate.GetPoints(i)[j];
				if (fabsf(vPoint.x - fMouseX) < 1.f && fabsf(vPoint.y - fMouseY) < 0.1f)
				{
					delegate.miSelectCurve = i;
					delegate.miSelectPoint = j;
					return;
				}
			}
		}
	}
	// Point Click Off
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::RB) == KEYSTATE::DOWN)
	{
		delegate.miSelectCurve = -1;
		delegate.miSelectPoint = -1;
	}
	// Add Point
	if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::WB) == KEYSTATE::DOWN)
	{
		delegate.AddPoint(ENUM_CLASS(m_Items[m_iSelectedEntry].eType), ImVec2(fMouseX, fMouseY));
	}
}

void CSequencer::CustomDrawCompact(RampEdit& delegate, _int iIndex, const ImRect& customRect, const ImRect& clippingRect)
{
	if (m_iSelectedEntry != iIndex)
		return;

	delegate.mMin = ImVec2(static_cast<_float>(m_iFrameMin), 0.f);
	delegate.mMax = ImVec2(static_cast<_float>(m_iFrameMax), 1.f);
	m_pDrawList->PushClipRect(clippingRect.Min, clippingRect.Max, true);
	for (_int i = 0; i < 3; ++i)
	{
		for (_uint j = 0; j < delegate.mPoints.size(); ++j)
		{
			_float fFrame = delegate.mPoints[j].x;
			cout << fFrame << endl;
			if (fFrame < m_Items[iIndex].iFrameStart || fFrame > m_Items[iIndex].iFrameEnd)
				continue;
			_float fRatio = (fFrame - m_iFrameMin) / static_cast<_float>(m_iFrameMax - m_iFrameMin);
			_float fX = ImLerp(customRect.Min.x, customRect.Max.x, fRatio);
			m_pDrawList->AddLine(ImVec2(fX, customRect.Min.y + 6), ImVec2(fX, customRect.Max.y - 4), 0xAA000000, 4.f);
		}
	}
	m_pDrawList->PopClipRect();
}

HRESULT CSequencer::Initialize()
{
	m_iFrameMin = 0;
	m_iFrameMax = 100;

	m_iSequenceOption = ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME;

	// Camera
	CCamera::CAMERA_DESC CameraDesc = {};
	CameraDesc.fFovy = XMConvertToRadians(60.f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.f;
	CameraDesc.vEye = _float4(-1.019107, 5.458634, -15.936163, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CameraDesc.fMouseSensor = 0.004f;
	if (FAILED(m_pGameInstance->Add_Camera(ENUM_CLASS(LEVEL::CAMERA), TEXT("Scene"), ENUM_CLASS(LEVEL::CAMERA), TEXT("Prototype_GameObject_SceneCamera"), &CameraDesc)))
		CRASH("SceneCamera");

	return S_OK;
}

void CSequencer::Update(_float fTimeDelta)
{
	ImGui::Begin("Sequence");

	ImGui::PushItemWidth(300);
	ImGui::Text("Frame : "); ImGui::SameLine(); ImGui::InputInt("##1", &m_iCurrentFrame); ImGui::SameLine();
	ImGui::Text("/ Frame Min : ");  ImGui::SameLine(); ImGui::InputInt("##2", &m_iFrameMin); ImGui::SameLine();
	ImGui::Text("/ Frame Max : ");  ImGui::SameLine(); ImGui::InputInt("##3", &m_iFrameMax);
	ImGui::Text("TPS : ");  ImGui::SameLine(); ImGui::InputFloat("##4", &m_fTrackPerSec);
	ImGui::PopItemWidth();

	io = ImGui::GetIO();

	m_isRet = false;
	Drawing();

	ImGui::End();

	// Sequence System (Register / Play)
	Sequence_System(fTimeDelta);

	// Select Item Setting
	Selectable_Item();

	if(m_pGameInstance->Get_DIKeyState(DIK_N) == KEYSTATE::DOWN)
		Sorting_Item();

	if (m_pGameInstance->Get_DIKeyState(DIK_SPACE) == KEYSTATE::DOWN)
	{
		if(false == m_isPlay)
			m_fTrackAcc = static_cast<_float>(m_iCurrentFrame);
		m_isPlay = !m_isPlay;
	}
}

void CSequencer::Play(_float fTimeDelta)
{
	if (m_fTrackAcc >= m_iFrameMax)
	{
		m_isPlay = false;
		m_fTrackAcc = static_cast<_float>(m_iFrameMin);
		return;
	}

	if (0.f == m_fTrackAcc)
		m_pGameInstance->Play_Sequence(StringToWString(m_szSequenceTag));

	m_fTrackAcc += fTimeDelta * m_fTrackPerSec;
	m_iCurrentFrame = m_fTrackAcc;
}

void CSequencer::Sequence_System(_float fTimeDelta)
{
	ImGui::Begin("[Sequence System]");

	ImGui::Text("Tag :"); ImGui::SameLine();
	ImGui::InputText("##", m_szSequenceTag, MAX_PATH);

	if (ImGui::Button("Register"))
	{
		vector<SEQUENCE_ITEM_INFO> ItemInfos;
		vector<SEQUENCE_ITEM_DATA*> ItemDatas;

		for (size_t i = 0; i < m_Items.size(); ++i)
		{
			SEQUENCE_ITEM_INFO Info = {};
			Info.fStartFrame = static_cast<_float>(m_Items[i].iFrameStart);
			Info.fEndFrame = static_cast<_float>(m_Items[i].iFrameEnd);
			Info.strItemTag = StringToWString(m_Items[i].szItemLabel);
			Info.eType = m_Items[i].eType;
			ItemInfos.push_back(Info);

			switch (m_Items[i].eType)
			{
			case ITEM_TYPE::SCENE:
				{
					SQ_CAMERA_DATA* SceneData = new SQ_CAMERA_DATA(Info.fStartFrame, Info.fEndFrame, m_fTrackPerSec, m_Items[i].mRampEdit.mSQCameraDatas);
					ItemDatas.push_back(SceneData);
				}
				break;
			case ITEM_TYPE::ACTOR:
				break;
			case ITEM_TYPE::SFX:
				break;
			case ITEM_TYPE::EFFECT:
				break;
			case ITEM_TYPE::SOUND:
				break;
			}
		}

		CSequence::SEQUENCE_DESC SequenceDesc = {};
		SequenceDesc.fTrackPerSec = m_fTrackPerSec;
		SequenceDesc.fDuration = static_cast<_float>(m_iFrameMax);

		m_pGameInstance->Register_Sequence(StringToWString(m_szSequenceTag), ItemInfos, ItemDatas, &SequenceDesc);
	}

	if (ImGui::Button("Save"))
		m_isSaveSequence = !m_isSaveSequence;
	if (true == m_isSaveSequence)
		Save_Sequence();

	if (ImGui::Button("Load"))
		m_isLoadSequence = !m_isLoadSequence;
	if (true == m_isLoadSequence)
		Load_Sequence();

	ImGui::End();

	if (true == m_isPlay)
		Play(fTimeDelta);
}

void CSequencer::Save_Sequence()
{
	IGFD::FileDialogConfig config;

	config.path = "../../Client/Bin/Resource/Sequence/Scene/";
	config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

	ImGuiFileDialog::Instance()->OpenDialog("SceneSave", "Save File", ".json", config);

	if (ImGuiFileDialog::Instance()->Display("SceneSave")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			ofstream OutputFile(strFilePath);

			json SequenceJson;

			SequenceJson["Duration"] = m_iFrameMax;
			SequenceJson["TrackPerSec"] = m_fTrackPerSec;

			SequenceJson["Item"] = json::array();

			for (auto& Item : m_Items)
			{
				switch (Item.eType)
				{
				case ITEM_TYPE::ACTOR:
					break;
				case ITEM_TYPE::SCENE:
					Save_Scene(SequenceJson, Item);
					break;
				case ITEM_TYPE::SOUND:
					break;
				case ITEM_TYPE::SFX:
					break;
				case ITEM_TYPE::EFFECT:
					break;
				}
			}

			OutputFile << SequenceJson.dump(4);

			OutputFile.close();
		}
		m_isSaveSequence = false;
		ImGuiFileDialog::Instance()->Close();
	}
}

void CSequencer::Load_Sequence()
{
	IGFD::FileDialogConfig config;

	config.path = "../../Client/Bin/Resource/Sequence/Scene/";
	config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

	ImGuiFileDialog::Instance()->OpenDialog("SequenceLoad", "Load File", ".json", config);

	if (ImGuiFileDialog::Instance()->Display("SequenceLoad")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			ifstream InputFile(strFilePath);

			json SequenceJson;

			InputFile >> SequenceJson;

			m_iFrameMax = SequenceJson["Duration"];
			m_fTrackPerSec = SequenceJson["TrackPerSec"];

			for (auto& Item : SequenceJson["Item"])
			{
				_string strType = Item["Type"];

				if ("Actor" == strType)
				{

				}
				else if ("Scene" == strType)
					Load_Scene(Item);
			}
		}
		m_isLoadSequence = false;
		ImGuiFileDialog::Instance()->Close();
	}
}

void CSequencer::Save_Scene(json& Output, SEQUENCE_ITEM& item)
{
	json SceneJson;

	SceneJson["Type"] = "Scene";
	SceneJson["Tag"] = item.szItemLabel;
	SceneJson["FrameStart"] = item.iFrameStart;
	SceneJson["FrameEnd"] = item.iFrameEnd;
	SceneJson["TrackPerSec"] = m_fTrackPerSec;

	SceneJson["Frame"] = json::array();

	vector<SCENE_CAMERA_FRAME>& Frames = item.mRampEdit.mSQCameraDatas;
	for (size_t i = 0; i < Frames.size(); ++i)
	{
		json FrameJson;
		FrameJson["Start"] = Frames[i].fStartFrame;

		FrameJson["Quaternion"] = json::array();
		FrameJson["Quaternion"].push_back(Frames[i].vQuaternion.x);
		FrameJson["Quaternion"].push_back(Frames[i].vQuaternion.y);
		FrameJson["Quaternion"].push_back(Frames[i].vQuaternion.z);
		FrameJson["Quaternion"].push_back(Frames[i].vQuaternion.w);

		FrameJson["Position"] = json::array();
		FrameJson["Position"].push_back(Frames[i].vPosition.x);
		FrameJson["Position"].push_back(Frames[i].vPosition.y);
		FrameJson["Position"].push_back(Frames[i].vPosition.z);

		FrameJson["FOV"] = Frames[i].fFovy;

		FrameJson["Lerp"] = Frames[i].isLerp;

		SceneJson["Frame"].push_back(FrameJson);
	}

	Output["Item"].push_back(SceneJson);
}

void CSequencer::Load_Scene(json& Input)
{
	SEQUENCE_ITEM Item = {};

	Item.eType = ITEM_TYPE::SCENE;
	Item.isExpanded = true;

	_string strTag = Input["Tag"];
	strcpy_s(Item.szItemLabel, strTag.c_str());
	Item.iFrameStart = Input["FrameStart"];
	Item.iFrameEnd = Input["FrameEnd"];

	for (auto& FrameJson : Input["Frame"])
	{
		SCENE_CAMERA_FRAME Frame = {};
		Frame.fStartFrame = FrameJson["Start"];

		Frame.vQuaternion = _float4(FrameJson["Quaternion"][0], FrameJson["Quaternion"][1], FrameJson["Quaternion"][2], FrameJson["Quaternion"][3]);
		Frame.vPosition = _float3(FrameJson["Position"][0], FrameJson["Position"][1], FrameJson["Position"][2]);

		Frame.fFovy = FrameJson["FOV"];

		Frame.isLerp = FrameJson["Lerp"];

		Item.mRampEdit.mPoints.push_back(ImVec2(Frame.fStartFrame, 0.5f));
		Item.mRampEdit.mSQCameraDatas.push_back(Frame);
	}

	m_Items.push_back(Item);
}

void CSequencer::Selectable_Item()
{
	if (-1 < m_iSelectedEntry)
	{
		ImGui::Begin("Select");

		_char szEntry[MAX_PATH] = {};
		sprintf_s(szEntry, MAX_PATH, "[Entry] : %d", m_iSelectedEntry);
		ImGui::Text(szEntry);

		SEQUENCE_ITEM& item = m_Items[m_iSelectedEntry];

		ImGui::Text("[Frame]");
		_char szFrame[MAX_PATH] = {};
		sprintf_s(szFrame, MAX_PATH, "Start : %d / End : %d", item.iFrameStart, item.iFrameEnd);
		ImGui::Text(szFrame);

		ImGui::Text("[Label] : "); ImGui::SameLine();
		ImGui::PushID(10);
		ImGui::InputText("##", item.szItemLabel, MAX_PATH);
		ImGui::PopID();

		ImGui::Text("==================================");

		// Item SetUp
		switch (item.eType)
		{
		case ITEM_TYPE::ACTION:
			SetUp_Camera(item);
			break;
		case ITEM_TYPE::SCENE:
			SetUp_Scene(item);
			break;
		}

		// Point SetUp
		if (item.mRampEdit.miSelectCurve > -1 && item.mRampEdit.miSelectPoint > -1)
		{
			switch (item.eType)
			{
			case ITEM_TYPE::ACTION:
				SetUp_Camera_Point(item);
				break;
			case ITEM_TYPE::SCENE:
				SetUp_Scene_Point(item);
				break;
			}
		}

		ImGui::End();

		item.mRampEdit.Update_Frame(item.eType);
	}
}

void CSequencer::SetUp_Camera_Point(SEQUENCE_ITEM& item)
{
	ImGui::Begin("Point Setting");

	// Translation
	_int iSelectIndex = item.mRampEdit.miSelectPoint;
	iSelectIndex = min(iSelectIndex, static_cast<_int>(item.mRampEdit.GetPointCount(0)));

	CAMERA_FRAME& CameraFrame = item.mRampEdit.mTargetCameraFrames[iSelectIndex];

	_char szFrame[MAX_PATH] = {};
	sprintf_s(szFrame, MAX_PATH, "[Frame] : %.2f", CameraFrame.fStartFrame);
	ImGui::Text(szFrame);

	ImGui::Text("[Distance]");
	ImGui::PushID(100);
	ImGui::InputFloat("##", &CameraFrame.fDistance);
	ImGui::PopID();

	ImGui::Text("[Rotation]");
	_char szQuat[MAX_PATH] = {};
	sprintf_s(szQuat, MAX_PATH, "X : %.4f\nY : %.4f\nZ : %.4f\bW : %.4f", CameraFrame.vRotation.x, CameraFrame.vRotation.y, CameraFrame.vRotation.z, CameraFrame.vRotation.w);
	ImGui::Text(szQuat);
	if (ImGui::Button("Rotation Sync"))
	{
		_matrix CameraWoldMatrx = m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW);
		_vector vScale{}, vQuat{}, vTrans{};
		XMMatrixDecompose(&vScale, &vQuat, &vTrans, CameraWoldMatrx);
		XMStoreFloat4(&CameraFrame.vRotation, vQuat);
	}

	ImGui::Text("[Translation]");
	ImGui::PushID(102);
	ImGui::InputFloat3("##", reinterpret_cast<_float*>(&CameraFrame.vTranslation));
	ImGui::PopID();

	ImGui::Text("[Fov]");
	ImGui::PushID(103);
	ImGui::InputFloat("##", &CameraFrame.fFovy);
	ImGui::PopID();

	if (ImGui::RadioButton("[Lerp]", CameraFrame.isLerp))
		CameraFrame.isLerp = !CameraFrame.isLerp;

	ImGui::SameLine();
	if (ImGui::Button("Delete"))
	{
		item.mRampEdit.mPoints.erase(item.mRampEdit.mPoints.begin() + iSelectIndex);
		item.mRampEdit.mTargetCameraFrames.erase(item.mRampEdit.mTargetCameraFrames.begin() + iSelectIndex);
		if (item.mRampEdit.miSelectPoint >= item.mRampEdit.GetPointCount(0))
			item.mRampEdit.miSelectPoint = -1;
	}

	ImGui::End();
}

void CSequencer::SetUp_Scene_Point(SEQUENCE_ITEM& item)
{
	ImGui::Begin("Point Setting");

	// Translation
	_int iSelectIndex = item.mRampEdit.miSelectPoint;
	iSelectIndex = min(iSelectIndex, static_cast<_int>(item.mRampEdit.GetPointCount(0)));

	SCENE_CAMERA_FRAME& CameraFrame = item.mRampEdit.mSQCameraDatas[iSelectIndex];

	_char szFrame[MAX_PATH] = {};
	sprintf_s(szFrame, MAX_PATH, "[Frame] : %.2f", CameraFrame.fStartFrame);
	ImGui::Text(szFrame);

	ImGui::Text("[Rotation]");
	_char szQuat[MAX_PATH] = {};
	sprintf_s(szQuat, MAX_PATH, "X : %.4f\nY : %.4f\nZ : %.4f\bW : %.4f", CameraFrame.vQuaternion.x, CameraFrame.vQuaternion.y, CameraFrame.vQuaternion.z, CameraFrame.vQuaternion.w);
	ImGui::Text(szQuat);
	if (ImGui::Button("Rotation Sync"))
	{
		_matrix CameraWoldMatrx = m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW);
		_vector vScale{}, vQuat{}, vTrans{};
		XMMatrixDecompose(&vScale, &vQuat, &vTrans, CameraWoldMatrx);
		XMStoreFloat4(&CameraFrame.vQuaternion, vQuat);
	}

	ImGui::Text("[Translation]");
	ImGui::PushID(102);
	ImGui::InputFloat3("##", reinterpret_cast<_float*>(&CameraFrame.vPosition));
	ImGui::PopID();

	ImGui::Text("[Fov]");
	ImGui::PushID(103);
	ImGui::InputFloat("##", &CameraFrame.fFovy);
	ImGui::PopID();

	if (ImGui::RadioButton("[Lerp]", CameraFrame.isLerp))
		CameraFrame.isLerp = !CameraFrame.isLerp;

	ImGui::SameLine();
	if (ImGui::Button("Delete"))
	{
		item.mRampEdit.mPoints.erase(item.mRampEdit.mPoints.begin() + iSelectIndex);
		item.mRampEdit.mSQCameraDatas.erase(item.mRampEdit.mSQCameraDatas.begin() + iSelectIndex);
		if (item.mRampEdit.miSelectPoint >= item.mRampEdit.GetPointCount(0))
			item.mRampEdit.miSelectPoint = -1;
	}

	ImGui::End();
}

void CSequencer::SetUp_Camera(SEQUENCE_ITEM& item)
{
	ImGui::Text("[Camera Action Setting]");

	if (ImGui::Button("Action", ImVec2(100.f, 20.f)))
	{
		CAMERA_ACTION_EVENT event { item.mRampEdit.mTargetCameraFrames, true, item.iFrameStart, item.iFrameEnd, m_isEscape };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), TEXT("Event_Camera_Action"), event);
	}
	if (ImGui::Button("Reset", ImVec2(100.f, 20.f)))
	{
		CAMERA_ACTION_EVENT event{ item.mRampEdit.mTargetCameraFrames, false, item.iFrameStart, item.iFrameEnd };
		m_pGameInstance->Publish(ENUM_CLASS(STATIC::NONE), TEXT("Event_Camera_Action"), event);
	}

	ImGui::Text("======================");

	if (ImGui::RadioButton("Escape", m_isEscape))
		m_isEscape = !m_isEscape;

	if (ImGui::Button("Save"))
		m_isSave = !m_isSave;
	if (m_isSave)
		Save_CameraAction();

	ImGui::SameLine();
	if (ImGui::Button("Load"))
		m_isLoad = !m_isLoad;
	if (m_isLoad)
		Load_CameraAction();
}

void CSequencer::SetUp_Scene(SEQUENCE_ITEM& item)
{
	ImGui::Text("[Scene Camera Setting]");
}

void CSequencer::Sorting_Item()
{
	if (0 == m_Items.size())
		return;

	sort(m_Items.begin(), m_Items.end(), [this](const SEQUENCE_ITEM& srcItem, const SEQUENCE_ITEM& dstItem)->_bool {
		return srcItem.iFrameStart < dstItem.iFrameStart;
		});
}

void CSequencer::Save_CameraAction()
{
	IGFD::FileDialogConfig config;

	config.path = "../../Client/Bin/Resource/Sequence/Action/";
	config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;

	ImGuiFileDialog::Instance()->OpenDialog("CameraActionSave", "Save File", ".json", config);

	if (ImGuiFileDialog::Instance()->Display("CameraActionSave")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			ofstream OutputFile(strFilePath);

			json ActionJson;

			SEQUENCE_ITEM& item = m_Items[m_iSelectedEntry];

			ActionJson["Duration"] = item.iFrameEnd - item.iFrameStart;

			ActionJson["Frame"] = json::array();

			vector<CAMERA_FRAME>& Frames = item.mRampEdit.mTargetCameraFrames;
			for (size_t i = 0; i < Frames.size(); ++i)
			{
				json FrameJson;
				FrameJson["Start"] = Frames[i].fStartFrame;
				FrameJson["Distance"] = Frames[i].fDistance;

				FrameJson["Rotation"] = json::array();
				FrameJson["Rotation"].push_back(Frames[i].vRotation.x);
				FrameJson["Rotation"].push_back(Frames[i].vRotation.y);
				FrameJson["Rotation"].push_back(Frames[i].vRotation.z);
				FrameJson["Rotation"].push_back(Frames[i].vRotation.w);

				FrameJson["Translation"] = json::array();
				FrameJson["Translation"].push_back(Frames[i].vTranslation.x);
				FrameJson["Translation"].push_back(Frames[i].vTranslation.y);
				FrameJson["Translation"].push_back(Frames[i].vTranslation.z);

				FrameJson["FOV"] = Frames[i].fFovy;

				FrameJson["Lerp"] = Frames[i].isLerp;

				ActionJson["Frame"].push_back(FrameJson);
			}

			OutputFile << ActionJson.dump(4);

			OutputFile.close();
		}
		m_isSave = false;
		ImGuiFileDialog::Instance()->Close();
	}
}

void CSequencer::Load_CameraAction()
{
	IGFD::FileDialogConfig config;

	config.path = "../../Client/Bin/Resource/Sequence/Action/";
	config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;

	ImGuiFileDialog::Instance()->OpenDialog("CameraActionLoad", "Load File", ".json", config);

	if (ImGuiFileDialog::Instance()->Display("CameraActionLoad")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			ifstream InputFile(strFilePath);

			json ActionJson;

			InputFile >> ActionJson;

			if (ITEM_TYPE::ACTION == m_Items[m_iSelectedEntry].eType)
			{
				SEQUENCE_ITEM& item = m_Items[m_iSelectedEntry];
				item.iFrameStart = 0;
				item.iFrameEnd = static_cast<_int>(ActionJson["Duration"]);

				for (auto& Frame : ActionJson["Frame"])
				{
					CAMERA_FRAME CameraFrame = {};
					CameraFrame.fStartFrame = Frame["Start"];
					CameraFrame.vRotation = _float4(Frame["Rotation"][0], Frame["Rotation"][1], Frame["Rotation"][2], Frame["Rotation"][3]);
					CameraFrame.vTranslation = _float3(Frame["Translation"][0], Frame["Translation"][1], Frame["Translation"][2]);
					CameraFrame.fDistance = Frame["Distance"];
					CameraFrame.fFovy = Frame["FOV"];
					if (Frame.contains("Lerp"))
						CameraFrame.isLerp = Frame["Lerp"];

					item.mRampEdit.mPoints.push_back(ImVec2(CameraFrame.fStartFrame, 0.5f));
					item.mRampEdit.mTargetCameraFrames.push_back(CameraFrame);
				}
			}
			InputFile.close();
		}
		m_isLoad = false;
		ImGuiFileDialog::Instance()->Close();
	}
}

void CSequencer::Drawing()
{
	// Get DrawList
	m_pDrawList = ImGui::GetWindowDrawList();
	m_vCanvasPos = ImGui::GetCursorScreenPos();			// ImDrawList�� Screen ��ǥ�� ���
	m_vCanvasSize = ImGui::GetContentRegionAvail();		// Canvas Size


	//if (0 == m_Items.size())
	//	return;

	_int iControlHeight = m_Items.size() * m_iItemHeight;
	for (_int i = 0; i < m_Items.size(); ++i)
		iControlHeight += GetCustomHeight(i);

	m_iFrameCnt = max(m_iFrameMax - m_iFrameMin, 1);

	ImGui::BeginGroup();

	m_iVisibleFrameCnt = static_cast<_int>(floorf((m_vCanvasSize.x - m_iLegendWidth) / m_fFramePixelWidth));
	m_fBarWidthRatio = min(m_iVisibleFrameCnt / static_cast<_float>(m_iFrameCnt), 1.f);
	m_fBarWidthInPixels = m_fBarWidthRatio * (m_vCanvasSize.x - m_iLegendWidth);
	m_CustomDraws.clear();
	m_CompactCustomDraws.clear();

	Panning(m_iVisibleFrameCnt);
	Expand(iControlHeight);

	ImGui::EndGroup();
}

void CSequencer::Panning(const _int iVisibleFrameCnt)
{
	// Panning (Alt + Wheel Click -> Drag => ȭ�� �¿� �̵�)
	if (ImGui::IsWindowFocused() && m_pGameInstance->Get_DIKeyState(DIK_LALT) == KEYSTATE::PRESS && m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::WB) == KEYSTATE::PRESS)
	{
		if (false == m_isPanningView)
		{
			m_vPanningViewSource = io.MousePos;
			m_isPanningView = true;
			m_iPanningViewFrame = m_iFirstFrame;
		}
		m_iFirstFrame = m_iPanningViewFrame - static_cast<_int>((io.MousePos.x - m_vPanningViewSource.x) / m_fFramePixelWidth);
		m_iFirstFrame = clamp(m_iFirstFrame, m_iFrameMin, 0 > m_iFrameMax - iVisibleFrameCnt ? m_iFrameMax : m_iFrameMax - iVisibleFrameCnt);
	}
	// Panning Off
	if (true == m_isPanningView && m_pGameInstance->Get_DIKeyState(DIK_LALT) == KEYSTATE::UP)
		m_isPanningView = false;

	m_fFramePixelWidthTarget = clamp(m_fFramePixelWidthTarget, 0.1f, 50.f);
	m_fFramePixelWidth = ImLerp(m_fFramePixelWidth, m_fFramePixelWidthTarget, 0.33f);
	m_iFrameCnt = m_iFrameMax - m_iFrameMin;

	// ������ �Ǵ� Frame ������ �� Frame �������� ������ FirstFrame Min���� ����
	if (iVisibleFrameCnt >= m_iFrameCnt)
		m_iFirstFrame = m_iFrameMin;
}

void CSequencer::Expand(_int iControllHeight)
{
	if (false == m_isExpanded)
	{
		ImGui::InvisibleButton("Canvas", ImVec2(m_vCanvasSize.x - m_vCanvasPos.x, static_cast<_float>(m_iItemHeight)));
		m_pDrawList->AddRectFilled(m_vCanvasPos, ImVec2(m_vCanvasSize.x + m_vCanvasPos.x, m_vCanvasPos.y + m_iItemHeight), 0xFF3D3837, 0);
		_char szTemp[MAX_PATH] = {};
		sprintf_s(szTemp, MAX_PATH, GetCollapseFmt(), m_iFrameCnt, m_Items.size());
		m_pDrawList->AddText(ImVec2(m_vCanvasPos.x + 26, m_vCanvasPos.y + 2), 0xFFFFFFFF, szTemp);
	}
	else
	{
		_bool HasScrollBar = { true };

		ImVec2 vHeaderSize(m_vCanvasSize.x, static_cast<_float>(m_iItemHeight));
		ImVec2 vScrollBarSize(m_vCanvasSize.x, 14.f);
		// Top Bar
		ImGui::InvisibleButton("topBar", vHeaderSize);
		m_pDrawList->AddRectFilled(m_vCanvasPos, m_vCanvasPos + vHeaderSize, 0xFFFF000, 0);

		// Child Frame
		m_vChildFramePos = ImGui::GetCursorScreenPos();
		m_vChildFrameSize = ImVec2(m_vCanvasSize.x, m_vCanvasSize.y - 8.f - vHeaderSize.y - (HasScrollBar ? vScrollBarSize.y : 0));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
		ImGui::BeginChild(889, m_vChildFrameSize, ImGuiChildFlags_FrameStyle);
		focused = ImGui::IsWindowFocused();

		// Content Bar
		ImGui::InvisibleButton("contentBar", ImVec2(m_vCanvasSize.x, 0 == iControllHeight ? 10 : static_cast<_float>(iControllHeight)));
		m_vContentMin = ImGui::GetItemRectMin();
		m_vContentMax = ImGui::GetItemRectMax();
		m_ContentRect.Min = m_vContentMin;
		m_ContentRect.Max = m_vContentMax;
		m_fContentHeight = m_vContentMax.y - m_vContentMin.y;

		DrawFrame();

		ImGui::EndChild();
		ImGui::PopStyleColor();

		ScrollBar();
	}
}

void CSequencer::DrawFrame()
{
	// BackGround
	m_pDrawList->AddRectFilled(m_vCanvasPos, m_vCanvasPos + m_vCanvasSize, 0xFF242424, 0);

	ImRect topRect(ImVec2(m_vCanvasPos.x + m_iLegendWidth, m_vCanvasPos.y), ImVec2(m_vCanvasPos.x + m_vCanvasSize.x, m_vCanvasPos.y + m_iItemHeight));

	// Moving Frame
	if (false == m_isMovingCurrentFrame && false == m_isMovingScrollBar && -1 == m_iMovingEntry &&
		m_iSequenceOption & ImSequencer::SEQUENCER_CHANGE_FRAME && m_iCurrentFrame >= 0 &&
		topRect.Contains(io.MousePos) && m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::PRESS)
		m_isMovingCurrentFrame = true;

	if (true == m_isMovingCurrentFrame)
	{
		if (0 < m_iFrameCnt)
		{
			m_iCurrentFrame = static_cast<_int>((io.MousePos.x - topRect.Min.x) / m_fFramePixelWidth) + m_iFirstFrame;
			if (m_iCurrentFrame < m_iFrameMin)
				m_iCurrentFrame = m_iFrameMin;
			if (m_iCurrentFrame > m_iFrameMax)
				m_iCurrentFrame = m_iFrameMax;
		}
		if (m_pGameInstance->Get_DIMouseState(MOUSEKEYSTATE::LB) == KEYSTATE::UP)
			m_isMovingCurrentFrame = false;
	}

	// Header
	m_pDrawList->AddRectFilled(m_vCanvasPos, ImVec2(m_vCanvasSize.x + m_vCanvasPos.x, m_vCanvasPos.y + m_iItemHeight), 0xFF3D3837, 0);
	if (m_iSequenceOption & ImSequencer::SEQUENCER_ADD)
	{
		if (true == SequencerAddDelButton(ImVec2(m_vCanvasPos.x + m_iLegendWidth - m_iItemHeight, m_vCanvasPos.y + 2), true))
			ImGui::OpenPopup("Add Entry");

		if (ImGui::BeginPopup("Add Entry"))
		{
			for(_uint i = 0; i < ENUM_CLASS(ITEM_TYPE::END); ++i)
				if (ImGui::Selectable(GetItemTypeName(i)))
				{
					// TODO
					Add(i);
					m_iSelectedEntry = GetItemCount() - 1;
				}

			ImGui::EndPopup();
			m_isPopUp = true;
		}
	}

	// Header Frame Number and Lines
	m_iModFrameCnt = 10;
	m_iFrameStep = 1;
	while (m_iModFrameCnt * m_fFramePixelWidth < 150)
	{
		m_iModFrameCnt *= 2;
		m_iFrameStep *= 2;
	}
	m_iHalfModFrameCnt = m_iModFrameCnt / 2;

	for (_int i = m_iFrameMin; i <= m_iFrameMax; i += m_iFrameStep)
		DrawLine(i, m_iItemHeight);

	DrawLine(m_iFrameMin, m_iItemHeight);
	DrawLine(m_iFrameMax, m_iItemHeight);

	m_pDrawList->PushClipRect(m_vChildFramePos, m_vChildFramePos + m_vChildFrameSize, true);

	DrawLegend();

	m_pDrawList->PopClipRect();

	// Custom Draw
	for (auto& customDraw : m_CustomDraws)
		CustomDraw(m_Items[customDraw.iIndex].mRampEdit, customDraw.iIndex, customDraw.CustomRect, customDraw.LegendRect, customDraw.ClippingRect, customDraw.LegendClippingRect);

	//for (auto& customDraw : m_CompactCustomDraws)
	//	CustomDrawCompact(m_Items[customDraw.iIndex].mRampEdit, customDraw.iIndex, customDraw.CustomRect, customDraw.ClippingRect);


	CopyPaste();
}

void CSequencer::DrawLegend()
{
	size_t iCustomHeight = { 0 };
	size_t iDefaultGap = { 2 };

	// Draw Item Name
	for (size_t i = 0; i < m_Items.size(); ++i)
	{
		_int iType = {};
		Get(i, nullptr, nullptr, &iType, nullptr);
		ImVec2 vItemPos(m_vContentMin.x + 3, m_vContentMin.y + i * m_iItemHeight + iDefaultGap + iCustomHeight);
		m_pDrawList->AddText(vItemPos, 0xFFFFFFFF, m_Items[i].szItemLabel);

		if (m_iSequenceOption & ImSequencer::SEQUENCER_DEL)
		{
			if (true == SequencerAddDelButton(ImVec2(m_vContentMin.x + m_iLegendWidth - m_iItemHeight + iDefaultGap - 10, vItemPos.y + iDefaultGap), false))
				m_iDelEntry = i;
			if (true == SequencerAddDelButton(ImVec2(m_vContentMin.x + m_iLegendWidth - (m_iItemHeight * 2) + iDefaultGap - 10, vItemPos.y + iDefaultGap), true))
				m_iDupEntry = i;
		}
		iCustomHeight += GetCustomHeight(i);
	}

	ItemDupDel();

	//ImGuiIO& io = ImGui::GetIO();
	// Slot BackGround
	for (size_t i = 0; i < m_Items.size(); ++i)
	{
		_uint iCol = (i % 1) ? 0xFF3A3636 : 0xFF413D3D;

		size_t iLocalCustomHeight = GetCustomHeight(i);
		ImVec2 vPos = ImVec2(m_vContentMin.x + m_iLegendWidth, m_vContentMin.y + m_iItemHeight * i + 1 + iCustomHeight);
		ImVec2 vSize = ImVec2(m_vCanvasSize.x + m_vCanvasPos.x, vPos.y + m_iItemHeight - 1 + iLocalCustomHeight);
		if (false == m_isPopUp && io.MousePos.y < vPos.y + (m_iItemHeight + iLocalCustomHeight) && -1 == m_iMovingEntry &&
			io.MousePos.x > m_vContentMin.x && io.MousePos.x < m_vContentMin.x + m_vCanvasSize.x)
		{
			iCol += 0x80201008;
			vPos.x -= m_iLegendWidth;
		}
		m_pDrawList->AddRectFilled(vPos, vSize, iCol, 0);
		iCustomHeight += iLocalCustomHeight;
	}
	m_pDrawList->PushClipRect(m_vChildFramePos + ImVec2(static_cast<_float>(m_iLegendWidth), 0.f), m_vChildFramePos + m_vChildFrameSize, true);

	// Vertical Frame Lines in Content Area
	for (_int i = m_iFrameMin; i <= m_iFrameMax; i += m_iFrameStep)
		DrawLineContent(i, static_cast<_int>(m_fContentHeight));
	DrawLineContent(m_iFrameMin, static_cast<_int>(m_fContentHeight));
	DrawLineContent(m_iFrameMax, static_cast<_int>(m_fContentHeight));

	DrawSlot();
	Moving();
	Cursor();

	m_pDrawList->PopClipRect();
}

void CSequencer::ItemDupDel()
{
	// Item Duplicate
	if (m_iDupEntry > -1)
	{
		// TODO

	}

	// Item Delete
	if (m_iDelEntry > -1)
	{
		m_Items.erase(m_Items.begin() + m_iDelEntry);
		if (m_iSelectedEntry >= m_Items.size())
			m_iSelectedEntry = m_Items.size() - 1;
		m_iDelEntry = -1;
	}
}

void CSequencer::DrawSlot()
{
	_int iCustomHeight = {};
	_int iDefaultGap = { 2 };

	_bool isSelected = 0 <= m_iSelectedEntry;
	// Selection
	if (true == isSelected)
	{
		iCustomHeight = 0;
		for (_int i = 0; i < m_iSelectedEntry; ++i)
			iCustomHeight += GetCustomHeight(i);
		m_pDrawList->AddRectFilled(ImVec2(m_vContentMin.x, m_vContentMin.y + m_iItemHeight * m_iSelectedEntry + iCustomHeight), ImVec2(m_vContentMin.x + m_vCanvasSize.x, m_vContentMin.y + m_iItemHeight * (m_iSelectedEntry + 1) + iCustomHeight), 0x801080FF, 1.f);
	}

	iCustomHeight = 0;
	for (size_t i = 0; i < m_Items.size(); ++i)
	{
		_int* pStart = { nullptr };
		_int* pEnd = { nullptr };
		_uint iColor = {};
		Get(i, &pStart, &pEnd, nullptr, &iColor);
		size_t iLocalCustomHeight = GetCustomHeight(i);

		ImVec2 vPos = ImVec2(m_vContentMin.x + m_iLegendWidth - m_iFirstFrame * m_fFramePixelWidth, m_vContentMin.y + m_iItemHeight * i + 1 + iCustomHeight);
		ImVec2 vSlotP1(vPos.x + *pStart * m_fFramePixelWidth, vPos.y + iDefaultGap);
		ImVec2 vSlotP2(vPos.x + *pEnd * m_fFramePixelWidth + m_fFramePixelWidth, vPos.y + m_iItemHeight - iDefaultGap);
		ImVec2 vSlotP3(vPos.x + *pEnd * m_fFramePixelWidth + m_fFramePixelWidth, vPos.y + m_iItemHeight - iDefaultGap + iLocalCustomHeight);
		_uint iSlotColor = iColor | 0xFF000000;
		_uint iSlotColorHalf = (iColor & 0xFFFFFF) | 0x40000000;

		if (vSlotP1.x <= (m_vCanvasSize.x + m_vContentMin.x) && vSlotP2.x >= (m_vContentMin.x + m_iLegendWidth))
		{
			m_pDrawList->AddRectFilled(vSlotP1, vSlotP3, iSlotColorHalf, 2);
			m_pDrawList->AddRectFilled(vSlotP1, vSlotP2, iSlotColor, 2);
		}
		if (true == ImRect(vSlotP1, vSlotP2).Contains(io.MousePos) && true == io.MouseDoubleClicked[0])
		{
			// TODO
			// DoubleClick?
		}

		// Ensure Grabbable Handles
		const _float fMaxHandleWidth = vSlotP2.x - vSlotP1.x / 3.f;
		const _float fMinHandleWidth = min(10.f, fMaxHandleWidth);
		const _float fHandleWidth = clamp(m_fFramePixelWidth * 0.5f, fMinHandleWidth, fMaxHandleWidth);
		ImRect Rects[3] = {
			ImRect(vSlotP1, ImVec2(vSlotP1.x + fHandleWidth, vSlotP2.y)),
			ImRect(ImVec2(vSlotP2.x - fHandleWidth, vSlotP1.y), vSlotP2),
			ImRect(vSlotP1, vSlotP2)
		};

		const _uint iQuadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, iSlotColor + (true == isSelected ? 0 : 0x202020) };
		if (-1 == m_iMovingEntry && (m_iSequenceOption & ImSequencer::SEQUENCER_EDIT_STARTEND))
		{
			for (_int j = 2; j >= 0; --j)
			{
				ImRect& rc = Rects[j];
				if (false == rc.Contains(io.MousePos))
					continue;
				m_pDrawList->AddRectFilled(rc.Min, rc.Max, iQuadColor[j], 2);
			}

			for (_int j = 0; j < 3; ++j)
			{
				ImRect& rc = Rects[j];
				if (false == rc.Contains(io.MousePos))
					continue;
				if (false == ImRect(m_vChildFramePos, m_vChildFramePos + m_vChildFrameSize).Contains(io.MousePos))
					continue;
				if (ImGui::IsMouseClicked(0) && false == m_isMovingScrollBar && false == m_isMovingCurrentFrame)
				{
					m_iMovingEntry = i;
					m_iMovingPos = io.MousePos.x;
					m_iMovingPart = j + 1;
					BeginEdit(m_iMovingEntry);
					break;
				}
			}
		}

		// Custom Draw
		if (0 < iLocalCustomHeight)
		{
			ImVec2 vRP(m_vCanvasPos.x, m_vContentMin.y + m_iItemHeight * i + 1 + iCustomHeight);
			ImRect CustomRect(
				vRP + ImVec2(m_iLegendWidth - (m_iFirstFrame - m_iFrameMin - 0.5f) * m_fFramePixelWidth, static_cast<_float>(m_iItemHeight)),
				vRP + ImVec2(m_iLegendWidth + (m_iFrameMax - m_iFirstFrame - 0.5f + 2.f) * m_fFramePixelWidth, static_cast<_float>(iLocalCustomHeight + m_iItemHeight))
				);
			ImRect ClippingRect(vRP + ImVec2(static_cast<_float>(m_iLegendWidth), static_cast<_float>(m_iItemHeight)), vRP + ImVec2(m_vCanvasSize.x, static_cast<_float>(iLocalCustomHeight + m_iItemHeight)));

			ImRect LegendRect(vRP + ImVec2(0.f, static_cast<_float>(m_iItemHeight)), vRP + ImVec2(static_cast<_float>(m_iLegendWidth), static_cast<_float>(iLocalCustomHeight)));
			ImRect LegendClippingRect(m_vCanvasPos + ImVec2(0.f, static_cast<_float>(m_iItemHeight)), m_vCanvasPos + ImVec2(static_cast<_float>(m_iLegendWidth), static_cast<_float>(iLocalCustomHeight + m_iItemHeight)));
			m_CustomDraws.push_back({ static_cast<_int>(i), CustomRect, LegendRect, ClippingRect, LegendClippingRect });
		}
		else
		{
			ImVec2 vRP(m_vCanvasPos.x, m_vContentMin.y + m_iItemHeight * i + iCustomHeight);
			ImRect CustomRect(
				vRP + ImVec2(m_iLegendWidth - (m_iFirstFrame - m_iFrameMin - 0.5f) * m_fFramePixelWidth, 0.f),
				vRP + ImVec2(m_iLegendWidth + (m_iFrameMax - m_iFirstFrame - 0.5f + 2.f) * m_fFramePixelWidth, static_cast<_float>(m_iItemHeight))
			);
			ImRect ClippingRect(vRP + ImVec2(static_cast<_float>(m_iLegendWidth), 0.f), vRP + ImVec2(m_vCanvasSize.x, static_cast<_float>(m_iItemHeight)));

			m_CompactCustomDraws.push_back({ static_cast<_int>(i), CustomRect, ImRect(), ClippingRect, ImRect() });
		}
		iCustomHeight += iLocalCustomHeight;
	}
}

void CSequencer::Moving()
{
	if (0 <= m_iMovingEntry)
	{
		ImGui::SetNextFrameWantCaptureMouse(true);

		_int iDiffFrame = static_cast<_int>((io.MousePos.x - m_iMovingPos) / m_fFramePixelWidth);
		if (abs(iDiffFrame) > 0)
		{
			_int* pStart = { nullptr };
			_int* pEnd = { nullptr };
			Get(m_iMovingEntry, &pStart, &pEnd, nullptr, nullptr);
			m_iSelectedEntry = m_iMovingEntry;
			_int& iLeft = *pStart;
			_int& iRight = *pEnd;

			if (m_iMovingPart & 1)
				iLeft += iDiffFrame;
			if (m_iMovingPart & 2)
				iRight += iDiffFrame;
			
			if (iLeft < 0)
			{
				if (m_iMovingPart & 2)
					iRight -= iLeft;
				iLeft = 0;
			}
			if (m_iMovingPart & 1 && iLeft > iRight)
				iLeft = iRight;
			if (m_iMovingPart & 2 && iRight < iLeft)
				iRight = iLeft;
			m_iMovingPos += static_cast<_int>(iDiffFrame * m_fFramePixelWidth);
		}
		if (false == io.MouseDown[0])
		{
			// Single Select
			if (!iDiffFrame && m_iMovingPart)
			{
				m_iSelectedEntry = m_iMovingEntry;
				m_isRet = true;
			}
			m_iMovingEntry = -1;
			EndEdit();
		}
	}
}

void CSequencer::Cursor()
{
	if (m_iCurrentFrame >= m_iFirstFrame && m_iCurrentFrame <= m_iFrameMax)
	{
		_float fCursorOffset = m_vContentMin.x + m_iLegendWidth + (m_iCurrentFrame - m_iFirstFrame) * m_fFramePixelWidth + m_fFramePixelWidth * 0.5f - m_fCursorWidth * 0.5f;
		m_pDrawList->AddLine(ImVec2(fCursorOffset, m_vCanvasPos.y), ImVec2(fCursorOffset, m_vContentMax.y), 0xA02A2AFF, m_fCursorWidth);
		_char szTemp[MAX_PATH] = {};
		sprintf_s(szTemp, MAX_PATH, "%d", m_iCurrentFrame);
		m_pDrawList->AddText(ImVec2(fCursorOffset + 10, m_vCanvasPos.y + 2), 0xFF2A2AFF, szTemp);
	}
}

void CSequencer::CopyPaste()
{
	if (m_iSequenceOption & ImSequencer::SEQUENCER_COPYPASTE)
	{
		ImRect rectCopy(ImVec2(m_vContentMin.x + 90, m_vCanvasPos.y + 2)
			, ImVec2(m_vContentMin.x + 90 + 30, m_vCanvasPos.y + m_iItemHeight - 2));
		_bool isRectCopy = rectCopy.Contains(io.MousePos);
		_uint iCopyColor = isRectCopy ? 0xFF1080FF : 0xFF000000;
		m_pDrawList->AddText(rectCopy.Min, iCopyColor, "Copy");

		ImRect rectPaste(ImVec2(m_vContentMin.x + 130, m_vCanvasPos.y + 2)
			, ImVec2(m_vContentMin.x + 130 + 30, m_vCanvasPos.y + m_iItemHeight - 2));
		_bool isRectPaste = rectPaste.Contains(io.MousePos);
		_uint iPasteColor = isRectPaste ? 0xFF1080FF : 0xFF000000;
		m_pDrawList->AddText(rectPaste.Min, iPasteColor, "Paste");

		if (isRectCopy && io.MouseReleased[0])
			Copy();
		
		if (isRectPaste && io.MouseReleased[0])
			Paste();
	}
}

void CSequencer::ScrollBar()
{
	if (true == m_isScrollBar)
	{
		ImVec2 vScrollBarSize(m_vCanvasSize.x, 14.f);
		ImGui::InvisibleButton("ScrollBar", vScrollBarSize);

		ImVec2 vScrollBarMin = ImGui::GetItemRectMin();
		ImVec2 vScrollBarMax = ImGui::GetItemRectMax();

		_float fStartFrameOffset = static_cast<_float>((m_iFirstFrame - m_iFrameMin) / static_cast<_float>(m_iFrameCnt)) * (m_vCanvasSize.x - m_iLegendWidth);
		ImVec2 vScrollBarA(vScrollBarMin.x + m_iLegendWidth, vScrollBarMin.y - 2);
		ImVec2 vScrollBarB(vScrollBarMin.x + m_vCanvasSize.x, vScrollBarMax.y - 1);
		m_pDrawList->AddRectFilled(vScrollBarA, vScrollBarB, 0xFF222222, 0);

		ImRect ScrollBarRect(vScrollBarA, vScrollBarB);
		_bool isInScrollBar = ScrollBarRect.Contains(io.MousePos);

		m_pDrawList->AddRectFilled(vScrollBarA, vScrollBarB, 0xFF101010, 8);

		ImVec2 vScrollBarC(vScrollBarMin.x + m_iLegendWidth + fStartFrameOffset, vScrollBarMin.y);
		ImVec2 vScrollBarD(vScrollBarMin.x + m_iLegendWidth + m_fBarWidthInPixels + fStartFrameOffset, vScrollBarMax.y - 2);
		m_pDrawList->AddRectFilled(vScrollBarC, vScrollBarD, (isInScrollBar || m_isMovingScrollBar) ? 0xFF606060 : 0xFF505050, 6);

		ImRect BarHandleLeft(vScrollBarC, ImVec2(vScrollBarC.x + 14, vScrollBarD.y));
		ImRect BarHandleRight(ImVec2(vScrollBarD.x - 14, vScrollBarC.y), vScrollBarD);

		_bool isOnLeft = BarHandleLeft.Contains(io.MousePos);
		_bool isOnRight = BarHandleRight.Contains(io.MousePos);

		m_pDrawList->AddRectFilled(BarHandleLeft.Min, BarHandleLeft.Max, (isOnLeft || m_isSizingLeftBar) ? 0xFFAAAAAA : 0xFF666666, 6);
		m_pDrawList->AddRectFilled(BarHandleRight.Min, BarHandleRight.Max, (isOnRight || m_isSizingRightBar) ? 0xFFAAAAAA : 0xFF666666, 6);

		ImRect ScrollBarThumb(vScrollBarC, vScrollBarD);
		m_fMinBarWidth = 44.f;
		if (true == m_isSizingRightBar)
		{
			if (false == io.MouseDown[0])
				m_isSizingRightBar = false;
			else
			{
				_float fBarNewWidth = max(m_fBarWidthInPixels + io.MouseDelta.x, m_fMinBarWidth);
				_float fBarRatio = fBarNewWidth / m_fBarWidthInPixels;
				m_fFramePixelWidthTarget = m_fFramePixelWidth = m_fFramePixelWidth / fBarRatio;
				_int iNewVisibleFrameCnt = static_cast<_int>((m_vCanvasSize.x - m_iLegendWidth) / m_fFramePixelWidthTarget);
				_int iLastFrame = m_iFirstFrame + iNewVisibleFrameCnt;
				if (iLastFrame > m_iFrameMax)
					m_fFramePixelWidthTarget = m_fFramePixelWidth = (m_vCanvasSize.x - m_iLegendWidth) / static_cast<_float>(m_iFrameMax - m_iFirstFrame);
			}
		}
		else if (true == m_isSizingLeftBar)
		{
			if (false == io.MouseDown[0])
				m_isSizingLeftBar = false;
			else
			{
				if (fabsf(io.MouseDelta.x) > FLT_EPSILON)
				{
					_float fBarNewWidth = max(m_fBarWidthInPixels - io.MouseDelta.x, m_fMinBarWidth);
					_float fBarRatio = fBarNewWidth / m_fBarWidthInPixels;
					_float fPreviousFramePixelWidthTarget = m_fFramePixelWidthTarget;
					m_fFramePixelWidthTarget = m_fFramePixelWidth = m_fFramePixelWidth / fBarRatio;
					_int iNewVisibleFrameCnt = static_cast<_int>(m_iVisibleFrameCnt / fBarRatio);
					_int iNewFirstFrame = m_iFirstFrame + iNewVisibleFrameCnt - m_iVisibleFrameCnt;
					iNewFirstFrame = clamp(iNewFirstFrame, m_iFrameMin, max(m_iFrameMax - m_iVisibleFrameCnt, m_iFrameMin));
					if (iNewFirstFrame == m_iFirstFrame)
						m_fFramePixelWidth = m_fFramePixelWidthTarget = fPreviousFramePixelWidthTarget;
					else
						m_iFirstFrame = iNewFirstFrame;
				}
			}
		}
		else
		{
			if (true == m_isMovingScrollBar)
			{
				if (false == io.MouseDown[0])
					m_isMovingScrollBar = false;
				else
				{
					_float fFramesPerPixelInBar = m_fBarWidthInPixels / static_cast<_float>(m_iVisibleFrameCnt);
					m_iFirstFrame = static_cast<_int>((io.MousePos.x - m_vPanningViewSource.x) / fFramesPerPixelInBar) - m_iPanningViewFrame;
					m_iFirstFrame = clamp(m_iFirstFrame, m_iFrameMin, max(m_iFrameMax - m_iVisibleFrameCnt, m_iFrameMin));
				}
			}
			else
			{
				if (ScrollBarThumb.Contains(io.MousePos) && ImGui::IsMouseClicked(0) && false == m_isMovingCurrentFrame && -1 == m_iMovingEntry)
				{
					m_isMovingScrollBar = true;
					m_vPanningViewSource = io.MousePos;
					m_iPanningViewFrame = -m_iFirstFrame;
				}
				if (false == m_isSizingRightBar && isOnRight && ImGui::IsMouseClicked(0))
					m_isSizingRightBar = true;
				if (false == m_isSizingLeftBar && isOnLeft && ImGui::IsMouseClicked(0))
					m_isSizingLeftBar = true;
			}
		}
	}
	
}

void CSequencer::DrawLine(_int iFrame, _int iRegionHeight)
{
	_bool isBaseIndex = ((iFrame % m_iModFrameCnt) == 0) || (iFrame == m_iFrameMax || iFrame == m_iFrameMin);
	_bool isHalfIndex = (iFrame % m_iHalfModFrameCnt) == 0;
	_int iPX = static_cast<_int>(m_vCanvasPos.x) + static_cast<_int>(iFrame * m_fFramePixelWidth) + m_iLegendWidth - static_cast<_int>(m_iFirstFrame * m_fFramePixelWidth);
	_int iTiretStart = true == isBaseIndex ? 4 : (true == isHalfIndex ? 10 : 14);
	_int iTiretEnd = true == isBaseIndex ? iRegionHeight : m_iItemHeight;

	if (iPX <= (m_vCanvasPos.x + m_vCanvasSize.x) && iPX >= (m_vCanvasPos.x + m_iLegendWidth))
	{
		m_pDrawList->AddLine(ImVec2(static_cast<_float>(iPX), m_vCanvasPos.y + static_cast<_float>(iTiretStart)), ImVec2(static_cast<_float>(iPX), m_vCanvasPos.y + static_cast<_float>(iTiretEnd) - 1), 0xFF606060, 1);
		m_pDrawList->AddLine(ImVec2(static_cast<_float>(iPX), m_vCanvasPos.y + static_cast<_float>(m_iItemHeight)), ImVec2(static_cast<_float>(iPX), m_vCanvasPos.y + static_cast<_float>(iRegionHeight) - 1), 0x30606060, 1);
	}

	if (true == isBaseIndex && iPX > (m_vCanvasPos.x + m_iLegendWidth))
	{
		_char szTemp[MAX_PATH] = {};
		sprintf_s(szTemp, MAX_PATH, "%d", iFrame);
		m_pDrawList->AddText(ImVec2(static_cast<_float>(iPX) + 3.f, m_vCanvasPos.y), 0xFFBBBBBB, szTemp);
	}
}

void CSequencer::DrawLineContent(_int iFrame, _int iRegionHeight)
{
	_int iPX = static_cast<_int>(m_vCanvasPos.x) + static_cast<_int>(iFrame * m_fFramePixelWidth) + m_iLegendWidth - static_cast<_int>(m_iFirstFrame * m_fFramePixelWidth);
	_int iTiretStart = static_cast<_int>(m_vContentMin.y);
	_int iTiretEnd = static_cast<_int>(m_vContentMax.y);

	if (iPX <= (m_vCanvasPos.x + m_vCanvasSize.x) && iPX >= (m_vCanvasPos.x + m_iLegendWidth))
		m_pDrawList->AddLine(ImVec2(static_cast<_float>(iPX), static_cast<_float>(iTiretStart)), ImVec2(static_cast<_float>(iPX), static_cast<_float>(iTiretEnd)), 0x30606060, 1);
}

_bool CSequencer::SequencerAddDelButton(ImVec2 vPos, _bool isAdd)
{
	//ImGuiIO& io = ImGui::GetIO();

	ImRect BtnRect(vPos, ImVec2(vPos.x + 16, vPos.y + 16));
	_bool isOverBtn = BtnRect.Contains(io.MousePos);
	_bool isContainedClick = true == isOverBtn && BtnRect.Contains(io.MouseClickedPos[0]);
	_bool isClickedBtn = true == isContainedClick && io.MouseReleased[0];
	_int iBtnColor = true == isOverBtn ? 0xAAEAFFAA : 0x77A3B2AA;
	if (true == isContainedClick && io.MouseDownDuration[0] > 0)
		BtnRect.Expand(2.0f);

	_float fMidY = vPos.y + 16 * 0.5f - 0.5f;
	_float fMidX = vPos.x + 16 * 0.5f - 0.5f;
	m_pDrawList->AddRect(BtnRect.Min, BtnRect.Max, iBtnColor, 4);
	m_pDrawList->AddLine(ImVec2(BtnRect.Min.x + 3, fMidY), ImVec2(BtnRect.Max.x - 3, fMidY), iBtnColor, 2);
	if (true == isAdd)
		m_pDrawList->AddLine(ImVec2(fMidX, BtnRect.Min.y + 3), ImVec2(fMidX, BtnRect.Max.y - 3), iBtnColor, 2);

	return isClickedBtn;
}

CSequencer* CSequencer::Create()
{
	CSequencer* pInstance = new CSequencer();

	if (FAILED(pInstance->Initialize()))
		CRASH("Sequence Create");

	return pInstance;
}

void CSequencer::Free()
{
	__super::Free();

	m_pDrawList = nullptr;

	Safe_Release(m_pGameInstance);
}
