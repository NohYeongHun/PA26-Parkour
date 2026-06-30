#pragma once
#include "BT_Node.h"

NS_BEGIN(Engine)
class ENGINE_DLL CBT_Selector final : public CBT_Node
{
private:
	CBT_Selector();
	CBT_Selector(const CBT_Selector& Prototype);
	virtual ~CBT_Selector() = default;

public:
	HRESULT Initialize_Prototype() override;
	HRESULT Initialize_Clone(void* pArg) override;

	virtual BT_STATE tick(class CGameObject* pGameObject, class CBlackBoard* pBlackBoard) override;

public:
	void Add_Child(CBT_Node* pPattern) { m_Children.push_back(pPattern); }

private:
	vector<CBT_Node*> m_Children;

public:
	static CBT_Selector* Create();
	CBT_Node* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
