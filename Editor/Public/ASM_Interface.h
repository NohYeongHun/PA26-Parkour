#pragma once
#include "Interface_Edit.h"
NS_BEGIN(Engine)
class CBehavior_Tree;
NS_END

NS_BEGIN(Editor)
class CASM_Interface final : public CInterface_Edit
{
	enum ASM_MENU
	{
		BEHAVIOR_TREE,
		ANIM_MACHINE
	};
	enum BT_TYPE
	{
		ACTION, SELECTOR, SEQUENCE
	};
	enum DATA_TYPE
	{
		INT,
		FLOAT,
		MASK,
		BOOL,
		VECTOR3,
		VECTOR4,
		DATA_END
	};

	//typedef std::variant<std::monostate, _int, _float, _string, _bool, _float3, _float4> VAR;

	typedef struct tagValue
	{
		DATA_TYPE eType;
		//VAR pValue;
		void* pValue;
	}ASM_VALUE;

	typedef struct ConditionTag
	{
		_string strValue;
		_string strCondition;
		_string strConst;
	}CONDITION_TAG;

#pragma region GraphEditor_Definition

	typedef struct tagBTDelegate : public GraphEditor::Delegate
	{
		CASM_Interface* pInterface {nullptr};

		bool AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) override;
		void SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected) override;
		void MoveSelectedNodes(const ImVec2 delta) override;
		void AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) override;
		void DelLink(GraphEditor::LinkIndex linkIndex) override;
		void CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) override;
		void RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput) override;
		const size_t GetTemplateCount() override;
		const GraphEditor::Template GetTemplate(GraphEditor::TemplateIndex index) override;
		const size_t GetNodeCount() override;
		const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override;
		const size_t GetLinkCount() override;
		const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override;
	}BT_DELEGATE;

	//struct MyLink : public GraphEditor::Link
	//{
	//	_string strCondition;
	//};

	struct MyNode : public GraphEditor::Node
	{
		_string strName;
		float x, y;
		BT_TYPE eType;
		_uint iTargetState;
		DATA_TYPE eDataType;
		vector<GraphEditor::Link> Transitions;
		CONDITION_TAG Conditions;
	};
#pragma endregion

private:
	explicit CASM_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CASM_Interface() = default;

public:
	virtual	HRESULT		Initialize();
	void				Update_ASM(_float fTimeDelta);

	

private:
	ASM_MENU			m_eCurrentMenu = { ASM_MENU::BEHAVIOR_TREE };
	_bool					m_isShowLoadFile = {false};
	_bool					m_isShowSaveFile = {false};
	_bool					m_isLoadtoComponent = {false};
	_string					m_strFileName;

#pragma region BehaviorTree_GraphEdit
	BT_DELEGATE						m_BehaviorTreeGraphDelegate;
	GraphEditor::Options			m_BehaviorTreeGraphOptions;
	GraphEditor::ViewState			m_BehaviorTreeViewState;

	vector<MyNode>					m_Nodes;
	std::vector<GraphEditor::Link>	m_Links;
	//GraphEditor::Template	m_Templates[4]{};
	vector<GraphEditor::Template>	m_Templates;

	_int							m_iCurrentNodeIndex { -1 };
	_uint							m_iNodeCount{};
	BT_TYPE							m_eNodeType{};
#pragma endregion

	CBehavior_Tree*					m_pBehaviorTree = { nullptr };
	CBlackBoard*					m_pBlackBoard = {nullptr};

	DATA_TYPE						m_eDataType{};
	_bool							m_isConditionCreate{};

#pragma region INPUT_VALUE
	//map<const _string, pair<DATA_TYPE, VAR>> m_ValueContainer;
	_string m_strValueKey;
	map<const _string, pair<DATA_TYPE, void*>> m_ValueContainer;
	_string m_strValueTag;
	_int m_iInputTemp{};
	_float m_fInputTemp{};
	_uint m_uInputTemp;
	_bool m_bInputTemp{};
	_float3 m_v3InputTemp{};
	_float4 m_v4InputTemp{};

	_char m_strValueName[MAX_PATH];
	_char m_strConditionName[MAX_PATH];
	_char m_strConstName[MAX_PATH];

	set<_string> m_RequireValueKey;
	set<_string> m_RequireConditionKey;
	set<_string> m_RequireConstKey;
#pragma endregion

private:
	void				Menu_BehaviorTree();
	void				Graph_BehaviorTree();
	void				Node_Info();
	void				Delete_Link();
	_bool				Allowed_LInkEx(GraphEditor::Link& tLink);
	void				Delete_Transitions(const GraphEditor::Link& tLink);
	void				BehaviorTree_Setting();
	void				BlackBoard_Setting();
	void				Rebase_Graph();

	void				Create_Template(BT_TYPE eType, _uint iOutputCount = 1);

	void				Initialize_BT();
	void				Save_BT_Data();
	void				Save_Nodes(ofstream& File, _uint& iIndex);
	void				Load_BT_Data();

#ifdef _DEBUG
	void				Safe_Delete_Variable(const _string& strVariableTag);
#endif

	void				Menu_AnimMachine();
	void				Graph_AnimMachine();

	void				Clear_Container();

public:
	static		CASM_Interface* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};
NS_END
