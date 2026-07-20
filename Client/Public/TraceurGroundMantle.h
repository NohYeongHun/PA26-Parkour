#pragma once
#include "GroundState.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CTraceurGroundMantle final : public CGroundState
{
public:
	explicit CTraceurGroundMantle() = default;
	virtual ~CTraceurGroundMantle() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	void Update_Animations(_float fTimeDelta) override;
	void Late_Anim_Update(_float fTimeDelta) override;

private:
	_bool Ready_Enter(void* pArg);

#ifdef _DEBUG
private:
	void Draw_Debug();
#endif

public:
	static CTraceurGroundMantle* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END
