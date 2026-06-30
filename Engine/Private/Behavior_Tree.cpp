#include "EnginePch.h"
#include "Behavior_Tree.h"
#include "BT_Action.h"
#include "BT_Selector.h"
#include "BT_Sequence.h"
#include "GameObject.h"

CBehavior_Tree::CBehavior_Tree(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CComponent { pDevice , pContext }
{
}

CBehavior_Tree::CBehavior_Tree(const CBehavior_Tree& Prototype)
	:CComponent { Prototype }
	,m_pRoot { Prototype.m_pRoot }
	,m_RequireKey { Prototype.m_RequireKey }
{
	Safe_AddRef(m_pRoot);
}

#ifdef _DEBUG
HRESULT CBehavior_Tree::Initialize_Prototype(CBT_Node* pRoot)
{
    m_pRoot = pRoot;
    
	return S_OK;
}
#endif // _DEBUG

HRESULT CBehavior_Tree::Initialize_Prototype(const _char* BehaviorTreeDataPath)
{

	Load_Tree_Graph(BehaviorTreeDataPath);
	m_pRoot = Create_Node(0);
	if(nullptr == m_pRoot)
		return E_FAIL;
	m_NodesDatas.clear();
	return S_OK;
}

HRESULT CBehavior_Tree::Initialize_Clone(void* pArg)
{
	BEHAVIOR_TREE_DESC* pDesc = static_cast<BEHAVIOR_TREE_DESC*>(pArg);
	if(pDesc == nullptr)
		return E_FAIL;

	m_pBlackBoard = pDesc->pBlackBoard;
	if(m_pBlackBoard == nullptr)
		return E_FAIL;

	if(false == m_pBlackBoard->Necessary_Key_Check(m_RequireKey))
	{
		CRASH(m_RequireKey.data())
		return E_FAIL;
	}
	m_RequireKey.clear();
	return S_OK;
}

void CBehavior_Tree::tick(CGameObject* pGameObject)
{
    m_pRoot->tick(pGameObject, m_pBlackBoard);
}

#ifdef _DEBUG
void CBehavior_Tree::BlackBoardInfo()
{
    m_pBlackBoard->Bind_Data_to_GUI();
}
#endif // _DEBUG

void CBehavior_Tree::Load_Tree_Graph(const _char* BehaviorTreeDataPath)
{

	ifstream File(BehaviorTreeDataPath);
	if(false == File.is_open())
		CRASH("Load File Open");
	json BT_Data;
	File >> BT_Data;

	size_t iNumNodes = BT_Data["NumNode"];
	for(auto& NodeData : BT_Data["Nodes"])
	{
		BT_TYPE eType = NodeData["eType"];
		size_t iNumTransition = NodeData["NumTransition"];
		_uint iTargetState =  NodeData["TargetState"];
		vector<Link> Transition;
		for(auto& Transit : NodeData["Transitions"])
		{
			Link tLink{Transit["InputNodeIndex"],
									Transit["InputSlotIndex"],
									Transit["OutputNodeIndex"],
									Transit["OutputSlotIndex"]};
			Transition.push_back(tLink);
		}

		//size_t iNumCondition = NodeData["NumCondition"];
		CONDITION_TAG Condition {NodeData["ValueName"], NodeData["ConditionName"], NodeData["ConstName"]};

		NodeDat tNode;
		tNode.eType = eType;
		tNode.Transition = Transition;
		tNode.Conditions = Condition;
		tNode.iTargetState = iTargetState == 0 ? 0 : (1 << (iTargetState - 1));
		m_NodesDatas.push_back(tNode);
	}

	for(auto& strKey : BT_Data["A_ValueKey"])
	{
		m_RequireKey.push_back(strKey);
	}
	m_RequireKey.push_back("iState");

	for(auto& strKey : BT_Data["A_ConditionKey"])
	{
		m_RequireKey.push_back(strKey);
	}
	m_RequireKey.push_back("isAnimationRunning");

	for(auto& strKey : BT_Data["A_ConstKey"])
	{
		m_RequireKey.push_back(strKey);
	}

	File.close();
}

CBT_Node* CBehavior_Tree:: Create_Node(_uint iIndex)
{
	CBT_Node* BT_Node = nullptr;
	switch(m_NodesDatas[iIndex].eType)
	{
	case ACTION:
	{
		const NodeDat tData = m_NodesDatas[iIndex];
		
		if(0 == tData.Conditions.ConditionName.length())
			//Idle
			if(0 == tData.iTargetState)
			{
				BT_Node = CBT_Action::Create([tData](CGameObject* pGameObject, CBlackBoard* pBlackBoard) ->CBT_Node::BT_STATE{
					return CBT_Node::BT_STATE::SUCCESS;
					});
			}
			//dead
			else
			{
				BT_Node = CBT_Action::Create([tData](CGameObject* pGameObject, CBlackBoard* pBlackBoard) ->CBT_Node::BT_STATE{
					if(pBlackBoard->Get_Condition("isAnimationRunning"))
						return CBT_Node::BT_STATE::RUNNING;
					_uint* pState = static_cast<_uint*>(pBlackBoard->Get_Data("iState"));
					if(*pState & tData.iTargetState)
					{
						pGameObject->SetActivate(false);
//#ifdef _DEBUG
//						cout << "State: " << *pState << ", Condition: " << tData.Conditions.ConditionName.c_str() << endl;
//#endif // _DEBUG		
						return CBT_Node::BT_STATE::SUCCESS;
					}
					return CBT_Node::BT_STATE::FAILURE;
				});
			}
		else
			BT_Node = CBT_Action::Create([tData](CGameObject* pGameObject, CBlackBoard* pBlackBoard) ->CBT_Node::BT_STATE{
			
			if(pBlackBoard->Get_Condition("isAnimationRunning"))
				return CBT_Node::BT_STATE::RUNNING;
			
			if(false == pBlackBoard->Get_Condition(tData.Conditions.ConditionName))
				return CBT_Node::BT_STATE::FAILURE;

			_uint* pState = static_cast<_uint*>(pBlackBoard->Get_Data("iState"));
			*pState |= tData.iTargetState;
//#ifdef _DEBUG
//			cout << "State: " << *pState << ", Condition: " << tData.Conditions.ConditionName.c_str() << endl;
//#endif // _DEBUG
			return CBT_Node::BT_STATE::SUCCESS;

			});
		break;
	}
	case SELECTOR:
	{
		BT_Node = CBT_Selector::Create();
		for(auto& tLink : m_NodesDatas[iIndex].Transition)
		{
			dynamic_cast<CBT_Selector*>(BT_Node)->Add_Child(Create_Node(tLink.iOutNodeIndex));
		}
		break;
	}
	case SEQUENCE:
	{
		BT_Node = CBT_Sequence::Create();
		for(auto& tLink : m_NodesDatas[iIndex].Transition)
		{
			dynamic_cast<CBT_Sequence*>(BT_Node)->Add_Child(Create_Node(tLink.iOutNodeIndex));
		}
		break;
	}
	default:
		CRASH(m_NodesDatas[iIndex].eType)
		break;
	}


	return BT_Node;
}

#ifdef _DEBUG
CBehavior_Tree* CBehavior_Tree::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CBT_Node* pRoot)
{
    CBehavior_Tree* pInstance = new CBehavior_Tree(pDevice, pContext);
    if (FAILED(pInstance->Initialize_Prototype(pRoot)))
    {
        MSG_BOX("Failed to Created : CBehavior_Tree");
        Safe_Release(pInstance);
    }
    return pInstance;
}
#endif // _DEBUG

CBehavior_Tree* CBehavior_Tree::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* BehaviorTreeDataPath)
{
	CBehavior_Tree* pInstance = new CBehavior_Tree(pDevice, pContext);
	if(FAILED(pInstance->Initialize_Prototype(BehaviorTreeDataPath)))
	{
		MSG_BOX("Failed to Created : CBehavior_Tree");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CBehavior_Tree::Clone(void* pArg)
{
    CBehavior_Tree* pInstance = new CBehavior_Tree(*this);
    if (FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Cloned : CBehavior_Tree");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CBehavior_Tree::Free()
{
	__super::Free();
	Safe_Release(m_pRoot);
    Safe_Release(m_pBlackBoard);
}
