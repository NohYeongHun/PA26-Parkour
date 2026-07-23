#pragma once
#include "ClimbState.h"

NS_BEGIN(Client)
class CTraceurClimbHang final : public CClimbState
{
private:
	explicit CTraceurClimbHang() = default;
	virtual ~CTraceurClimbHang() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;
	virtual void Build_IKRequests(vector<IK_REQUEST>& Out) override;

private:
	virtual void Update_Animations(_float fTimeDelta) override;
	virtual void Late_Anim_Update(_float fTimeDelta) override;

private:
	_bool Ready_Hang(void* pArg);

#ifdef _DEBUG
private:
	void Draw_DebugProbes() const;
#endif

private:
	_float m_fSnapElapsed = 0.f;
	_bool  m_isSnapping   = false;

	_bool  m_isHangIK = { false };			// Ready_Hang 성공 시에만 IK 선언 (좌표는 HANG_CONTEXT가 소유)

public:
	static CTraceurClimbHang* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END
