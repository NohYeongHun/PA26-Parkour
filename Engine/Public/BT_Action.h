#pragma once
#include "BT_Node.h"

NS_BEGIN(Engine)
class ENGINE_DLL CBT_Action final : public CBT_Node
{
	CBT_Action(function<BT_STATE(class CGameObject* pGameObject, CBlackBoard* pBlackBoard)> Action) :m_Action{ Action } {}
	CBT_Action(const CBT_Action& Prototype);
	virtual ~CBT_Action() = default;

public:
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize_Clone(void* pArg) override;

	virtual BT_STATE tick(class CGameObject* pGameObject, class CBlackBoard* pBlackBoard) override;

private:
	function<BT_STATE(CGameObject* pGameObject, CBlackBoard* pBlackBoard)> m_Action;

public:
	static CBT_Action* Create(function<BT_STATE(CGameObject* pGameObject, CBlackBoard* pBlackBoard)> Action);
	CBT_Node* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
