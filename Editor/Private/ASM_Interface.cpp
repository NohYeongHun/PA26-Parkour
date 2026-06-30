#include "EditorPch.h"
#include "ASM_Interface.h"
#pragma region BehaviorTree
#include "BT_Action.h"
#include "BT_Selector.h"
#include "BT_Sequence.h"
#pragma endregion

CASM_Interface::CASM_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CInterface_Edit{pDevice, pContext}
{
}

HRESULT CASM_Interface::Initialize()
{
	//Action Templete
	//m_Templates[0] = {
	//
	//	IM_COL32(160, 160, 180, 255),
	//	IM_COL32(100, 100, 140, 255),
	//	IM_COL32(110, 110, 150, 255),
	//	1,
	//	nullptr,
	//	nullptr,
	//	0,
	//	nullptr,
	//	nullptr
	//};
	//Node tNode = {"Test",0, 0.f, 0.f, false};
	//m_Nodes.push_back(tNode);
	m_BehaviorTreeGraphDelegate.pInterface = this;

	//CBT_Selector* pRoot = CBT_Selector::Create();
	//m_pBehaviorTree = CBehavior_Tree::Create(m_pDevice, m_pContext, nullptr);

	m_pBlackBoard = CBlackBoard::Create();

	//CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	//BTDesc.pBlackBoard = m_pBlackBoard;
	//m_pBehaviorTree->Initialize_Clone(&BTDesc);
	//Safe_AddRef(m_pBlackBoard);

	Initialize_BT();

	return S_OK;
}

void CASM_Interface::Update_ASM(_float fTimeDelta)
{
	ImGui::Begin("Animation State Machine Interface");

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if(ImGui::BeginTabBar("TabBar", tab_bar_flags))
	{
		if(ImGui::BeginTabItem("Behavior Tree"))
		{
			m_eCurrentMenu = ASM_MENU::BEHAVIOR_TREE;
			Menu_BehaviorTree();
			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("Anim Machine"))
		{
			m_eCurrentMenu = ASM_MENU::ANIM_MACHINE;
			Menu_AnimMachine();
			ImGui::EndTabItem();

		}
		ImGui::EndTabBar();
	}

	ImGui::End();

	if(m_eCurrentMenu == ASM_MENU::BEHAVIOR_TREE)
	{
		Graph_BehaviorTree();
	}
	else if(m_eCurrentMenu == ASM_MENU::ANIM_MACHINE)
	{
		Graph_AnimMachine();
	}

#ifdef _DEBUG
	m_pBlackBoard->Bind_Data_to_GUI();
	//m_pBehaviorTree->BlackBoardInfo();
	//ImGui::ShowMetricsWindow();
#endif // _DEBUG
	m_BehaviorTreeGraphDelegate.GetTemplateCount();
}

void CASM_Interface::Menu_BehaviorTree()
{
	ImGui::BeginTable("Property", 2, ImGuiTableFlags_BordersInnerV);
	ImGui::TableNextColumn();

	BehaviorTree_Setting();

	ImGui::TableNextColumn();
	
	BlackBoard_Setting();
	//_float3 a = *(_float3*)(m_ValueContainer["test"].second);
	ImGui::EndTable();
}

void CASM_Interface::Menu_AnimMachine()
{
	ImGui::BeginTable("Property", 2, ImGuiTableFlags_BordersInnerV);
	ImGui::TableNextColumn();
	ImGui::Text("Anim Machine Editor - To Be Continued...");
	ImGui::TableNextColumn();
	if(ImGui::Button("Add State"))
	{
	}
	ImGui::EndTable();
}

void CASM_Interface::Graph_BehaviorTree()
{
	ImGui::Begin("Behavior Tree Graph");
	GraphEditor::Show(m_BehaviorTreeGraphDelegate, m_BehaviorTreeGraphOptions, m_BehaviorTreeViewState, true);
	ImGui::End();
}

void CASM_Interface::Node_Info()
{
	ImGui::Separator();
	ImGui::Text("Selected Node Index : %d", m_iCurrentNodeIndex);
	ImGui::Text(m_Nodes[m_iCurrentNodeIndex].mName);
	ImGui::Text(m_Nodes[m_iCurrentNodeIndex].strName.c_str());
	ImGui::Text("x : %.3f", m_Nodes[m_iCurrentNodeIndex].x);
	ImGui::SameLine();
	ImGui::Text("y : %.3f", m_Nodes[m_iCurrentNodeIndex].y);
	for(const auto& tLink : m_Nodes[m_iCurrentNodeIndex].Transitions)
	{
		ImGui::Text("to: %d slot %d -> from: %d slot %d", tLink.mInputNodeIndex, tLink.mInputSlotIndex, tLink.mOutputNodeIndex, tLink.mOutputSlotIndex);
	}
	if(0 != m_Nodes[m_iCurrentNodeIndex].Conditions.strValue.length() || 
		0 != m_Nodes[m_iCurrentNodeIndex].Conditions.strCondition.length() ||
		0 != m_Nodes[m_iCurrentNodeIndex].Conditions.strConst.length())
	{
		ImGui::Text("%s - %s - %s", m_Nodes[m_iCurrentNodeIndex].Conditions.strValue.c_str(),
									m_Nodes[m_iCurrentNodeIndex].Conditions.strCondition.c_str(),
									m_Nodes[m_iCurrentNodeIndex].Conditions.strConst.c_str());
	}
	ImGui::InputText("NodeName", m_Nodes[m_iCurrentNodeIndex].strName.data(), MAX_PATH);
	if(ImGui::Button("Add Slot"))
	{
		if(BT_TYPE::ACTION != m_Nodes[m_iCurrentNodeIndex].eType)
			m_Templates[m_Nodes[m_iCurrentNodeIndex].mTemplateIndex].mOutputCount++;
	}
	if(BT_TYPE::ACTION == m_Nodes[m_iCurrentNodeIndex].eType)
	{
		ImGui::InputScalar("TargetState", ImGuiDataType_U32, &m_Nodes[m_iCurrentNodeIndex].iTargetState);
		ImGui::Text("%u", m_Nodes[m_iCurrentNodeIndex].iTargetState == 0 ? 0 : (1 << (m_Nodes[m_iCurrentNodeIndex].iTargetState - 1)));
		if(!m_isConditionCreate && ImGui::Button("Create Condition"))
		{
			m_isConditionCreate = true;
		}
		if(m_isConditionCreate && ImGui::Button("Undo"))
		{
			m_isConditionCreate = false;
		}
		if(m_isConditionCreate)
		{
			if(ImGui::RadioButton("_Int", reinterpret_cast<int*>(&m_Nodes[m_iCurrentNodeIndex].eDataType), 0)){}
			ImGui::SameLine();
			if(ImGui::RadioButton("_Float", reinterpret_cast<int*>(&m_Nodes[m_iCurrentNodeIndex].eDataType), 1)){}
			ImGui::SameLine();
			if(ImGui::RadioButton("_Mask", reinterpret_cast<int*>(&m_Nodes[m_iCurrentNodeIndex].eDataType), 2)){}
			ImGui::SameLine();
			if(ImGui::RadioButton("_Bool", reinterpret_cast<int*>(&m_Nodes[m_iCurrentNodeIndex].eDataType), 3)){}
			ImGui::SameLine();
			if(ImGui::RadioButton("_Vec3", reinterpret_cast<int*>(&m_Nodes[m_iCurrentNodeIndex].eDataType), 4)){}
			ImGui::SameLine();
			if(ImGui::RadioButton("_Vec4", reinterpret_cast<int*>(&m_Nodes[m_iCurrentNodeIndex].eDataType), 5)){}

			ImGui::InputText("Value", m_strValueName, MAX_PATH);
			ImGui::InputText("Condition", m_strConditionName, MAX_PATH);
			ImGui::InputText("Const", m_strConstName, MAX_PATH);
			if(ImGui::Button("Add Condition"))
			{
				CONDITION_TAG tCondition;
				tCondition.strValue = m_strValueName;
				if(tCondition.strValue.length() != 0)
					m_RequireValueKey.insert(m_strValueName);
				else
					m_Nodes[m_iCurrentNodeIndex].eDataType = DATA_TYPE::DATA_END;

				tCondition.strCondition = m_strConditionName;
				if(tCondition.strCondition.length() != 0)
					m_RequireConditionKey.insert(m_strConditionName);
				tCondition.strConst = m_strConstName;
				if(tCondition.strConst.length() != 0)
					m_RequireConstKey.insert(m_strConstName);

				m_Nodes[m_iCurrentNodeIndex].Conditions = tCondition;
				m_isConditionCreate = false;
			}
			if(ImGui::Button("Delete Condition"))
			{
				CONDITION_TAG Temp = m_Nodes[m_iCurrentNodeIndex].Conditions;
				m_Nodes[m_iCurrentNodeIndex].Conditions = CONDITION_TAG{};

				if (Temp.strValue.length() != 0)
				{
					_bool isDuplicate{};
					for (const auto tNode : m_Nodes)
						if (0 == tNode.Conditions.strValue.compare(Temp.strValue))
						{
							isDuplicate = true;
							break;
						}
					if(false == isDuplicate)
						m_RequireValueKey.erase(Temp.strValue);
				}
				if (Temp.strCondition.length() != 0)
				{
					_bool isDuplicate{};
					for (const auto tNode : m_Nodes)
						if (0 == tNode.Conditions.strCondition.compare(Temp.strCondition))
						{
							isDuplicate = true;
							break;
						}
					if (false == isDuplicate)
						m_RequireConditionKey.erase(Temp.strCondition);
				}
				if (Temp.strConst.length() != 0)
				{
					_bool isDuplicate{};
					for (const auto tNode : m_Nodes)
						if (0 == tNode.Conditions.strConst.compare(Temp.strConst))
						{
							isDuplicate = true;
							break;
						}
					if (false == isDuplicate)
						m_RequireConstKey.erase(Temp.strConst);
				}

				m_isConditionCreate = false;
			}
		}
	}
	if(ImGui::CollapsingHeader("Require Key"))
	{
		ImGui::BeginTable("Key Property", 3, ImGuiTableFlags_BordersInnerV);
		ImGui::TableNextColumn();
		ImGui::Text("Value");
		for(auto strKey : m_RequireValueKey)
		{
			ImGui::Text(strKey.c_str());
		}
		ImGui::TableNextColumn();
		ImGui::Text("Condition");
		for(auto strKey : m_RequireConditionKey)
		{
			ImGui::Text(strKey.c_str());
		}
		ImGui::TableNextColumn();
		ImGui::Text("Const");
		for(auto strKey : m_RequireConstKey)
		{
			ImGui::Text(strKey.c_str());
		}
		ImGui::EndTable();
	}
	ImGui::Text("end");
}

void CASM_Interface::Delete_Link()
{
	auto iter = m_Links.begin();
	while(iter != m_Links.end())
	{
		if((*iter).mInputNodeIndex == m_iCurrentNodeIndex || (*iter).mOutputNodeIndex == m_iCurrentNodeIndex)
		{
			Delete_Transitions((*iter));
			
			m_Templates[m_Nodes[(*iter).mInputNodeIndex].mTemplateIndex].mOutputCount--;
			if(m_Templates[m_Nodes[(*iter).mInputNodeIndex].mTemplateIndex].mOutputCount < 1)
				m_Templates[m_Nodes[(*iter).mInputNodeIndex].mTemplateIndex].mOutputCount = 1;

			iter = m_Links.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

_bool CASM_Interface::Allowed_LInkEx(GraphEditor::Link& tLink)
{
	for(auto& tTrans : m_Nodes[tLink.mInputNodeIndex].Transitions)
		if(tLink.mInputSlotIndex == tTrans.mInputSlotIndex)
			return false;

	return true;
}

void CASM_Interface::BehaviorTree_Setting()
{
	ImGui::Text("Behavior Tree Editor - To Be Continued...");
	if(m_iCurrentNodeIndex > -1)
	{
		Node_Info();
	}
	ImGui::Separator();
	if(ImGui::RadioButton("Action", reinterpret_cast<int*>(&m_eNodeType), 0)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Selector", reinterpret_cast<int*>(&m_eNodeType), 1)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Sequence", reinterpret_cast<int*>(&m_eNodeType), 2)){}

	if(ImGui::Button("Add Node"))
	{
		_string szNodeName;
		szNodeName = "Count";
		szNodeName += to_string(m_iNodeCount).c_str();
		switch(m_eNodeType)
		{
		case Editor::CASM_Interface::ACTION:
		{
			MyNode tNode = {"Action", m_Templates.size(), ImRect(), false, szNodeName, 0.f, 0.f, m_eNodeType, 0, DATA_TYPE::DATA_END};
			m_Nodes.push_back(tNode);
			break;
		}
		case Editor::CASM_Interface::SELECTOR:
		{
			MyNode tNode = {"Selector", m_Templates.size(), ImRect(), false, szNodeName, 0.f, 0.f, m_eNodeType, 0, DATA_TYPE::DATA_END};
			m_Nodes.push_back(tNode);
			break;
		}
		case Editor::CASM_Interface::SEQUENCE:
		{
			MyNode tNode = {"Sequence", m_Templates.size(), ImRect(), false, szNodeName, 0.f, 0.f, m_eNodeType, 0, DATA_TYPE::DATA_END};
			m_Nodes.push_back(tNode);
			break;
		}
		default:
			break;
		}
		
		//노드 동적 템플릿
		Create_Template(m_eNodeType);
		
		++m_iNodeCount;
	}
	ImGui::SameLine();
	if(ImGui::Button("Delete Node"))
	{
		if(m_iCurrentNodeIndex >= 0 && m_iCurrentNodeIndex < static_cast<_int>(m_Nodes.size()))
		{
			Delete_Link();
			//m_Links[0] == ImGuizmo::;
			m_Templates.erase(m_Templates.begin() + m_iCurrentNodeIndex);
			m_Nodes.erase(m_Nodes.begin() + m_iCurrentNodeIndex);
			size_t iNumNodes = m_Nodes.size();
			for(size_t i = 0; i < iNumNodes; ++i)
			{
				m_Nodes[i].mTemplateIndex = i;
			}

			Rebase_Graph();

			m_iCurrentNodeIndex = -1;
		}
	}
	ImGui::Separator();
	if(ImGui::Button("Save BT"))
	{
		m_isShowSaveFile = true;
	}
	ImGui::SameLine();
	if(ImGui::Button("Load BT"))
	{
		m_isShowLoadFile = true;
	}
	if(ImGui::Button("Bind to Component"))
	{
		m_isShowLoadFile = true;
		m_isLoadtoComponent = true;
	}
	if(m_isShowSaveFile)
		Save_BT_Data();
	if(m_isShowLoadFile)
		Load_BT_Data();
}

void CASM_Interface::BlackBoard_Setting()
{
	ImGui::Text("Black Board Data Setting");

	if(!m_ValueContainer.empty())
	{
		const _char* szPreview = m_strValueKey.c_str();
		if(ImGui::BeginCombo("Select Data", szPreview))
		{
			_int iGuiID{};
			for(auto& Pair : m_ValueContainer)
			{
				ImGui::PushID(iGuiID);
				const _bool isSelected = Pair.first == m_strValueKey;
				if(ImGui::Selectable(Pair.first.c_str(), isSelected))
				{
					m_strValueKey = Pair.first;
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
		switch(m_ValueContainer[m_strValueKey].first)
		{
		case Editor::CASM_Interface::INT:
			ImGui::Text("_Int");
			ImGui::InputInt(m_strValueKey.c_str(), static_cast<_int*>(m_ValueContainer[m_strValueKey].second));
			break;
		case Editor::CASM_Interface::FLOAT:
			ImGui::Text("_Float");
			ImGui::InputFloat(m_strValueKey.c_str(), static_cast<_float*>(m_ValueContainer[m_strValueKey].second));
			break;
		case Editor::CASM_Interface::MASK:
		{
			ImGui::Text("_Mask");
			ImGui::InputScalar(m_strValueKey.c_str(), ImGuiDataType_U32, static_cast<_uint*>(m_ValueContainer[m_strValueKey].second));
			break;
		}
		case Editor::CASM_Interface::BOOL:
			ImGui::Text("_Bool");
			ImGui::Checkbox(m_strValueKey.c_str(), static_cast<_bool*>(m_ValueContainer[m_strValueKey].second));
			break;
		case Editor::CASM_Interface::VECTOR3:
			ImGui::Text("_Float3");
			ImGui::InputFloat3(m_strValueKey.c_str(), static_cast<_float*>(m_ValueContainer[m_strValueKey].second));
			break;
		case Editor::CASM_Interface::VECTOR4:
			ImGui::Text("_Float4");
			ImGui::InputFloat4(m_strValueKey.c_str(), static_cast<_float*>(m_ValueContainer[m_strValueKey].second));
			break;
		default:
			break;
		}
	}
	ImGui::Separator();
#pragma region DATA_SELECT
	if(ImGui::RadioButton("Int", reinterpret_cast<int*>(&m_eDataType), 0)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Float", reinterpret_cast<int*>(&m_eDataType), 1)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Mask", reinterpret_cast<int*>(&m_eDataType), 2)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Bool", reinterpret_cast<int*>(&m_eDataType), 3)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Vec3", reinterpret_cast<int*>(&m_eDataType), 4)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Vec4", reinterpret_cast<int*>(&m_eDataType), 5)){}
#pragma endregion
	ImGui::InputText("Value Name", m_strValueTag.data(), MAX_PATH);
	switch(m_eDataType)
	{
	case Editor::CASM_Interface::INT:
		ImGui::InputInt("int", &m_iInputTemp);
		break;
	case Editor::CASM_Interface::FLOAT:
		ImGui::InputFloat("float", &m_fInputTemp);
		break;
	case Editor::CASM_Interface::MASK:
	{
		ImGui::InputScalar("mask", ImGuiDataType_U32, &m_uInputTemp);
		ImGui::Text("%u", (1 << m_uInputTemp));
		break;
	}
	case Editor::CASM_Interface::BOOL:
		ImGui::Checkbox("bool", &m_bInputTemp);
		break;
	case Editor::CASM_Interface::VECTOR3:
		ImGui::InputFloat3("vector3", reinterpret_cast<_float*>(&m_v3InputTemp));
		break;
	case Editor::CASM_Interface::VECTOR4:
		ImGui::InputFloat4("vector4", reinterpret_cast<_float*>(&m_v4InputTemp));
		break;
	default:
		break;
	}

	if(ImGui::Button("Add Variable"))
	{
		//pair<DATA_TYPE, VAR> _Value;
		pair<DATA_TYPE, void*> _Value;
		_Value.first = m_eDataType;
		switch(m_eDataType)
		{
		case Editor::CASM_Interface::INT:
		{
			_int* iTemp = new _int;
			*iTemp = m_iInputTemp;
			//_Value.second = m_iInputTemp;
			_Value.second = iTemp;
			break;
		}
		case Editor::CASM_Interface::FLOAT:
		{
			_float* fTemp = new _float;
			*fTemp = m_fInputTemp;
			//_Value.second = m_fInputTemp;
			_Value.second = fTemp;
			break;
		}
		case Editor::CASM_Interface::MASK:
		{
			_uint* uTemp = new _uint;
			*uTemp = m_uInputTemp;
			//_Value.second = m_strInputTemp;
			_Value.second = uTemp;
			break;
		}
		case Editor::CASM_Interface::BOOL:
		{
			_bool* bTemp = new _bool;
			*bTemp = m_bInputTemp;
			//_Value.second = m_bInputTemp;
			_Value.second = bTemp;
			break;
		}
		case Editor::CASM_Interface::VECTOR3:
		{
			_float3* v3Temp = new _float3;
			*v3Temp = m_v3InputTemp;
			//_Value.second = m_v3InputTemp;
			_Value.second = v3Temp;
			break;
		}
		case Editor::CASM_Interface::VECTOR4:
		{
			_float4* v4Temp = new _float4;
			*v4Temp = m_v4InputTemp;
			//_Value.second = m_v4InputTemp;
			_Value.second = v4Temp;
		}
			break;
		default:
			ASSERT_CRASH(false);
			break;
		}
		//m_strValueTag를 바로 map 키로 대입하거나 함수 매개변수로 대입하면 size 0인 상황 발생.
		_char szValueTag[MAX_PATH];
		strcpy_s(szValueTag, m_strValueTag.c_str());
		m_ValueContainer.emplace(szValueTag, _Value);
		if(FAILED(m_pBlackBoard->Add_Data(szValueTag, CBlackBoard::DATA_TYPE(m_ValueContainer[szValueTag].first), m_ValueContainer[szValueTag].second)))
			CRASH(m_ValueContainer[szValueTag].first);
		//m_strValueKey = m_strValueTag; 대입 시 m_strValueKey이 size 0 인 상태로 복사됨.
		m_strValueKey = szValueTag;

	}
#ifdef _DEBUG

#endif // _DEBUG

}

void CASM_Interface::Rebase_Graph()
{
	for (auto& Link : m_Links)
	{
		if (Link.mOutputNodeIndex > m_iCurrentNodeIndex)
			Link.mOutputNodeIndex--;
	}
	for (auto& Node : m_Nodes)
	{
		for (auto& Transition : Node.Transitions)
		{
			if (Transition.mOutputNodeIndex > m_iCurrentNodeIndex)
				Transition.mOutputNodeIndex--;
		}
	}
}

void CASM_Interface::Create_Template(BT_TYPE eType, _uint iOutputCount)
{
	/*
		IM_COL32(160, 160, 180, 255),
		IM_COL32(100, 100, 140, 255),
		IM_COL32(110, 110, 150, 255),
		1,
		nullptr,
		nullptr,
		0,
		nullptr,
		nullptr
	*/
	GraphEditor::Template tTemplate{};
	tTemplate.mInputCount = 1;
	switch(eType)
	{
	case Editor::CASM_Interface::ACTION:
	{
		tTemplate.mHeaderColor = IM_COL32(10, 120, 200, 255);
		tTemplate.mBackgroundColor = IM_COL32(0, 80, 160, 255);
		tTemplate.mBackgroundColorOver = IM_COL32(20, 100, 180, 255);
		tTemplate.mOutputCount = 0;
		break;
	}

	case Editor::CASM_Interface::SELECTOR:
	{
		tTemplate.mHeaderColor = IM_COL32(160, 160, 180, 255);
		tTemplate.mBackgroundColor = IM_COL32(100, 100, 140, 255);
		tTemplate.mBackgroundColorOver = IM_COL32(110, 110, 150, 255);
		tTemplate.mOutputCount = iOutputCount;
		break;
	}
	case Editor::CASM_Interface::SEQUENCE:
	{
		tTemplate.mHeaderColor = IM_COL32(10, 200, 10, 255);
		tTemplate.mBackgroundColor = IM_COL32(0, 160, 0, 255);
		tTemplate.mBackgroundColorOver = IM_COL32(20, 180, 20, 255);
		tTemplate.mOutputCount = iOutputCount;
		break;
	}
	default:
		break;
	}
	//tTemplate.mInputNames = nullptr;
	//tTemplate.mInputColors = nullptr;
	//tTemplate.mOutputNames = nullptr;
	//tTemplate.mOutputColors = nullptr;
	m_Templates.push_back(tTemplate);
}

void CASM_Interface::Initialize_BT()
{
	m_Nodes.clear();
	m_Links.clear();
	m_Templates.clear();
	m_iNodeCount = 0;

	_string szNodeName;
	szNodeName = "Count";
	szNodeName += to_string(m_iNodeCount).c_str();
	MyNode tNode = {"Selector", m_Templates.size(), ImRect(), false, szNodeName, 0.f, 0.f, BT_TYPE::SELECTOR};
	m_Nodes.push_back(tNode);

	Create_Template(BT_TYPE::SELECTOR);
	++m_iNodeCount;
}

void CASM_Interface::Save_BT_Data()
{
	IGFD::FileDialogConfig config;
	config.path = "../../Client/Bin/Resource/Model";
	config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;
	ImGuiFileDialog::Instance()->OpenDialog("BT File Save", "Export File", ".json", config);
	ImVec2 vMinSize = ImVec2(600, 400);
	ImVec2 vMaxSize = ImVec2(800, 400);
	if(ImGuiFileDialog::Instance()->Display(
		"BT File Save", ImGuiWindowFlags_NoCollapse
		, vMinSize
		, vMaxSize))
	{
		if(ImGuiFileDialog::Instance()->IsOk())
		{
			_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();

			ofstream file(strFilePath.c_str());
			if(false == file.is_open())
				CRASH("Save File Open");

			_uint Index{};
			Save_Nodes(file, Index);

			file.close();
		}
		ImGuiFileDialog::Instance()->Close();
		m_isShowSaveFile = false;
	}
}

void CASM_Interface::Save_Nodes(ofstream& File, _uint& iIndex)
{
	//노드에서 파일에 쓸 데이터 : 노드타입, 해당 노드 연결관계, 노드에서 사용되는 함수 키워드
	json Output;
	Output["NumNode"] = m_Nodes.size();
	Output["Nodes"] = json::array();
	for(size_t i = 0; i < m_Nodes.size(); ++i)
	{
		json Node;
		Node["eType"] = ENUM_CLASS(m_Nodes[i].eType);
		Node["NumTransition"] = m_Nodes[i].Transitions.size();
		Node["Editor_PosX"] = m_Nodes[i].x;
		Node["Editor_PosY"] = m_Nodes[i].y;
		Node["TargetState"] = m_Nodes[i].iTargetState;
		Node["Transitions"] = json::array();
		for(size_t j = 0; j < m_Nodes[i].Transitions.size(); j++)
		{
			json Transition;
			Transition["InputNodeIndex"] = m_Nodes[i].Transitions[j].mInputNodeIndex;
			Transition["InputSlotIndex"] = m_Nodes[i].Transitions[j].mInputSlotIndex;
			Transition["OutputNodeIndex"] = m_Nodes[i].Transitions[j].mOutputNodeIndex;
			Transition["OutputSlotIndex"] = m_Nodes[i].Transitions[j].mOutputSlotIndex;
			Node["Transitions"].push_back(Transition);
		}

		//함수 키워드
		// To do...
		//Node["NumCondition"] = m_Nodes[i].Conditions.size();
		Node["ValueName"] = m_Nodes[i].Conditions.strValue;
		Node["ConditionName"] = m_Nodes[i].Conditions.strCondition;
		Node["ConstName"] = m_Nodes[i].Conditions.strConst;
		

		Output["Nodes"].push_back(Node);
	}
	Output["A_ValueKey"] = json::array();
	for(auto& strKey : m_RequireValueKey)
	{
		Output["A_ValueKey"].push_back(strKey);
	}
	Output["A_ConditionKey"] = json::array();
	for(auto& strKey : m_RequireConditionKey)
	{
		Output["A_ConditionKey"].push_back(strKey);
	}
	Output["A_ConstKey"] = json::array();
	for(auto& strKey : m_RequireConstKey)
	{
		Output["A_ConstKey"].push_back(strKey);
	}
	File << Output.dump(4);
}

void CASM_Interface::Load_BT_Data()
{
	IGFD::FileDialogConfig config;
	config.path = "../../Client/Bin/Resource/Model";
	config.flags = ImGuiFileDialogFlags_ReadOnlyFileNameField;
	ImGuiFileDialog::Instance()->OpenDialog("BT File Load", "Import File", ".json", config);
	ImVec2 vMinSize = ImVec2(600, 400);
	ImVec2 vMaxSize = ImVec2(800, 400);
	if(ImGuiFileDialog::Instance()->Display(
		"BT File Load", ImGuiWindowFlags_NoCollapse
		, vMinSize
		, vMaxSize))
	{
		_string strFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
		m_strFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

		ifstream file(strFilePath.c_str());
		if(false == file.is_open())
			CRASH("Load File Open");

		m_Nodes.clear();
		m_Links.clear();
		m_Templates.clear();
		m_iNodeCount = 0;
		m_RequireValueKey.clear();
		m_RequireConditionKey.clear();
		m_RequireConstKey.clear();

		json BT_Data;
		file >> BT_Data;

		size_t iNumNodes = BT_Data["NumNode"];
		for(auto& NodeData : BT_Data["Nodes"])
		{
			BT_TYPE eType = NodeData["eType"];
			size_t iNumTransition = NodeData["NumTransition"];
			vector<GraphEditor::Link> Transition;
			for(auto& Transit : NodeData["Transitions"])
			{
				GraphEditor::Link tLink{Transit["InputNodeIndex"],
										Transit["InputSlotIndex"],
										Transit["OutputNodeIndex"],
										Transit["OutputSlotIndex"]};
				Transition.push_back(tLink);
				m_Links.push_back(tLink);
			}

			//size_t iNumCondition = NodeData["NumCondition"];
			CONDITION_TAG Conditions{NodeData["ValueName"], NodeData["ConditionName"], NodeData["ConstName"]};
			
			_float x{NodeData["Editor_PosX"]}, y{NodeData["Editor_PosY"]};
			_string szNodeName;
			_uint iTargetState = NodeData["TargetState"];
			szNodeName = "Count";
			szNodeName += to_string(m_iNodeCount).c_str();
			switch(eType)
			{
			case Editor::CASM_Interface::ACTION:
			{
				MyNode tNode = {"Action", m_Templates.size(), ImRect(), false, szNodeName, x, y, eType, iTargetState};
				tNode.Transitions = Transition;
				tNode.Conditions = Conditions;
				//tNode.iTargetState = NodeData["TargetState"];
				m_Nodes.push_back(tNode);
				break;
			}
			case Editor::CASM_Interface::SELECTOR:
			{
				MyNode tNode = {"Selector", m_Templates.size(), ImRect(), false, szNodeName, x, y, eType, iTargetState};
				tNode.Transitions = Transition;
				m_Nodes.push_back(tNode);
				break;
			}
			case Editor::CASM_Interface::SEQUENCE:
			{
				MyNode tNode = {"Sequence", m_Templates.size(), ImRect(), false, szNodeName, x, y, eType, iTargetState};
				tNode.Transitions = Transition;
				m_Nodes.push_back(tNode);
				break;
			}
			default:
				break;
			}

			Create_Template(eType, iNumTransition);
			++m_iNodeCount;
		}

		for(auto& strKey : BT_Data["A_ValueKey"])
		{
			m_RequireValueKey.insert(strKey);
		}
		for(auto& strKey : BT_Data["A_ConditionKey"])
		{
			m_RequireConditionKey.insert(strKey);
		}
		for(auto& strKey : BT_Data["A_ConstKey"])
		{
			m_RequireConstKey.insert(strKey);
		}

		file.close();
		ImGuiFileDialog::Instance()->Close();
		if(m_isLoadtoComponent)
		{
			if(nullptr != m_pBehaviorTree)
				Safe_Release(m_pBehaviorTree);

			m_pBehaviorTree = CBehavior_Tree::Create(m_pDevice, m_pContext, strFilePath.c_str());
			m_isLoadtoComponent = false;
		}
		m_iCurrentNodeIndex = -1;
		m_isShowLoadFile = false;
	}
}

#ifdef _DEBUG
void CASM_Interface::Safe_Delete_Variable(const _string& strVariableTag)
{
	if(m_ValueContainer.find(strVariableTag) == m_ValueContainer.end())
		return;
	m_pBlackBoard->Unbind_Data(strVariableTag);
	switch(m_ValueContainer[strVariableTag].first)
	{
	case Editor::CASM_Interface::INT:
		delete static_cast<_int*>(m_ValueContainer[strVariableTag].second);
		break;
	case Editor::CASM_Interface::FLOAT:
		delete static_cast<_float*>(m_ValueContainer[strVariableTag].second);
		break;
	case Editor::CASM_Interface::MASK:
		delete static_cast<_uint*>(m_ValueContainer[strVariableTag].second);
		break;
	case Editor::CASM_Interface::BOOL:
		delete static_cast<_bool*>(m_ValueContainer[strVariableTag].second);
		break;
	case Editor::CASM_Interface::VECTOR3:
		delete static_cast<_float3*>(m_ValueContainer[strVariableTag].second);
		break;
	case Editor::CASM_Interface::VECTOR4:
		delete static_cast<_float4*>(m_ValueContainer[strVariableTag].second);
		break;
	default:
		break;
	}
	m_ValueContainer.erase(strVariableTag);
	if(!m_ValueContainer.empty())
		m_strValueKey = m_ValueContainer.begin()->first;
}
#endif // _DEBUG

void CASM_Interface::Delete_Transitions(const GraphEditor::Link& tLink)
{
	for(auto& tNode : m_Nodes)
	{
		auto iter = tNode.Transitions.begin();
		while(iter != tNode.Transitions.end())
		{
			if(tLink.mInputNodeIndex == (*iter).mInputNodeIndex &&
				tLink.mInputSlotIndex == (*iter).mInputSlotIndex &&
				tLink.mOutputNodeIndex == (*iter).mOutputNodeIndex &&
				tLink.mOutputSlotIndex == (*iter).mOutputSlotIndex)
				iter = tNode.Transitions.erase(iter);
			else
				iter++;
		}
	}
}

void CASM_Interface::Graph_AnimMachine()
{
	ImGui::Begin("Anim Machine Graph");

	ImGui::End();
}

void CASM_Interface::Clear_Container()
{
	for(auto& Pair : m_ValueContainer)
	{
		switch(Pair.second.first)
		{
		case Editor::CASM_Interface::INT:
			delete static_cast<_int*>(Pair.second.second);
			break;
		case Editor::CASM_Interface::FLOAT:
			delete static_cast<_float*>(Pair.second.second);
			break;
		case Editor::CASM_Interface::MASK:
			delete static_cast<_uint*>(Pair.second.second);
			break;
		case Editor::CASM_Interface::BOOL:
			delete static_cast<_bool*>(Pair.second.second);
			break;
		case Editor::CASM_Interface::VECTOR3:
			delete static_cast<_float3*>(Pair.second.second);
			break;
		case Editor::CASM_Interface::VECTOR4:
			delete static_cast<_float4*>(Pair.second.second);
			break;
		default:
			break;
		}
		Pair.second.second = nullptr;
	}
	m_ValueContainer.clear();
}

CASM_Interface* CASM_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CASM_Interface* pInstance = new CASM_Interface(pDevice, pContext);
	if(FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CASM_Interface");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CASM_Interface::Free()
{
	__super::Free();
	Safe_Release(m_pBehaviorTree);
	Safe_Release(m_pBlackBoard);
	Clear_Container();
}

#pragma region GraphEdit_Definition
bool CASM_Interface::tagBTDelegate::AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to)
{
	if(from == to)
		return false;

	for(const auto& tLink : pInterface->m_Links)
	{
		if(tLink.mInputNodeIndex == to)
		{
			if(tLink.mOutputNodeIndex == from)
				return false;
		}
	}

	return true;
}

void CASM_Interface::tagBTDelegate::SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected)  
{  
   // Access the m_Nodes vector through the parent class instance  
   pInterface->m_Nodes[nodeIndex].mSelected = selected;
   pInterface->m_iCurrentNodeIndex = selected ? nodeIndex : -1;
}

void CASM_Interface::tagBTDelegate::MoveSelectedNodes(const ImVec2 delta)
{
	
	for(auto& node : pInterface->m_Nodes)
	{
		if(!node.mSelected)
			continue;

		node.x += delta.x;
		node.y += delta.y;
	}
}

void CASM_Interface::tagBTDelegate::AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex)
{
	//Link 구조체 변경으로 수정작업 필요
	GraphEditor::Link tLink = {inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex};
	if(false == pInterface->Allowed_LInkEx(tLink))
		return;
	pInterface->m_Links.push_back(tLink);
	pInterface->m_Nodes[inputNodeIndex].Transitions.push_back(tLink);
}

void CASM_Interface::tagBTDelegate::DelLink(GraphEditor::LinkIndex linkIndex)
{
	//링크의 인풋 슬롯 인덱스가 작을 때, 인덱스 매칭이 이뤄지지 않아서 에러 발생
	const GraphEditor::Link tLink = pInterface->m_Links[linkIndex];
	pInterface->Delete_Transitions(tLink);
	
	pInterface->m_Templates[pInterface->m_Nodes[tLink.mInputNodeIndex].mTemplateIndex].mOutputCount--;
	if(pInterface->m_Templates[pInterface->m_Nodes[tLink.mInputNodeIndex].mTemplateIndex].mOutputCount < 1)
		pInterface->m_Templates[pInterface->m_Nodes[tLink.mInputNodeIndex].mTemplateIndex].mOutputCount = 1;

	pInterface->m_Links.erase(pInterface->m_Links.begin() + linkIndex);
}

void CASM_Interface::tagBTDelegate::CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex)
{
	drawList->AddLine(rectangle.Min, rectangle.Max, IM_COL32(0, 0, 0, 255));
	drawList->AddText((rectangle.Min + rectangle.Max) * 0.5f, IM_COL32(255, 128, 64, 255), pInterface->m_Nodes[nodeIndex].strName.c_str());
}

void CASM_Interface::tagBTDelegate::RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput)
{
}

const size_t CASM_Interface::tagBTDelegate::GetTemplateCount()
{
	//return sizeof(pInterface->m_Templates.data()) / sizeof(GraphEditor::Template);
	return pInterface->m_Templates.size();
}

const GraphEditor::Template CASM_Interface::tagBTDelegate::GetTemplate(GraphEditor::TemplateIndex index)
{
	const auto& tTemplete = pInterface->m_Templates[index];
	return GraphEditor::Template{tTemplete};
}

const size_t CASM_Interface::tagBTDelegate::GetNodeCount()
{
	return pInterface->m_Nodes.size();
}

const GraphEditor::Node CASM_Interface::tagBTDelegate::GetNode(GraphEditor::NodeIndex index)
{
	const auto& tNode = pInterface->m_Nodes[index];
	return GraphEditor::Node
	{
		tNode.mName,
		tNode.mTemplateIndex,
		ImRect(ImVec2(tNode.x, tNode.y), ImVec2(tNode.x + 200, tNode.y + 200)),
		tNode.mSelected
	};
}

const size_t CASM_Interface::tagBTDelegate::GetLinkCount()
{
	return pInterface->m_Links.size();
}

const GraphEditor::Link CASM_Interface::tagBTDelegate::GetLink(GraphEditor::LinkIndex index)
{
	const auto& tLink = pInterface->m_Links[index];

	// 노드 인덱스 유효성
	//IM_ASSERT(tLink.mInputNodeIndex >= 0 && tLink.mInputNodeIndex < pInterface->m_Nodes.size());
	//IM_ASSERT(tLink.mOutputNodeIndex >= 0 && tLink.mOutputNodeIndex < pInterface->m_Nodes.size());

	// 템플릿에서 슬롯 개수 얻기
	//const auto& inNode = pInterface->m_Nodes[tLink.mInputNodeIndex];
	//const auto& outNode = pInterface->m_Nodes[tLink.mOutputNodeIndex];
	//
	//const auto& inTmpl = pInterface->m_Templates[inNode.mTemplateIndex];
	//const auto& outTmpl = pInterface->m_Templates[outNode.mTemplateIndex];
	//
	//IM_ASSERT(tLink.mInputSlotIndex >= 0 && tLink.mInputSlotIndex < inTmpl.mOutputCount);
	//IM_ASSERT(tLink.mOutputSlotIndex >= 0 && tLink.mOutputSlotIndex < outTmpl.mInputCount);

	return GraphEditor::Link
	{
		tLink.mInputNodeIndex,
		tLink.mInputSlotIndex,
		tLink.mOutputNodeIndex,
		tLink.mOutputSlotIndex
	};
	//return {0,0,1,0};
}
#pragma endregion