#pragma once
#include "Base.h"

NS_BEGIN(Engine)
class ENGINE_DLL CBT_Node abstract : public CBase
{
public:
	enum BT_STATE
{
	FAILURE,
	RUNNING,
	SUCCESS
 };
protected:
	CBT_Node();
	CBT_Node(const CBT_Node& Prototype);
	virtual ~CBT_Node() = default;

#ifdef _DEBUG
public:
	_int Get_Type() { return m_iType; }
#endif // _DEBUG

public:
	virtual HRESULT Initialize_Prototype() = 0;
	virtual HRESULT Initialize_Clone(void* pArg) = 0;

public:
	virtual BT_STATE tick(class CGameObject* pGameObject, class CBlackBoard* pBlackBoard) = 0;

#ifdef _DEBUG
protected:
	_int m_iType{ -1 };
#endif // _DEBUG

public:
	virtual CBT_Node* Clone(void* pArg) = 0;
	virtual void Free() override;
};
NS_END
