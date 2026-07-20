#pragma once
#include "ClimbState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurClimbExit final : public CClimbState
{
public:
	explicit CTraceurClimbExit() = default;
	virtual ~CTraceurClimbExit() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Update_Animations(_float fTimeDelta) override;
	void Late_Anim_Update(_float fTimeDelta) override;
	void Check_State() override;

	_bool Is_MantleAnim() const;

#ifdef _DEBUG
private:
	void Draw_DebugCurve();
#endif

private:
	void End_Traversal();

public:
	static CTraceurClimbExit* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END
