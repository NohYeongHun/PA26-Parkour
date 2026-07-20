#pragma once
#include "ClimbState.h"
#include "TraceurState_Enum.h"

NS_BEGIN(Client)
class CTraceurClimbHop final : public CClimbState
{
private:
	struct HANG_TARGET
	{
		_float3 vGrabEdgePos{};
		_float3 vWallNormal{};
		BodyID  GrabBodyID{};
	};

private:
	explicit CTraceurClimbHop() = default;
	virtual ~CTraceurClimbHop() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner) override;
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnExit() override;

private:
	virtual void Update_Animations(_float fTimeDelta) override;
	virtual void Late_Anim_Update(_float fTimeDelta) override;

private:
	_bool Find_HangTarget(ETraceurClimbHop eDir, HANG_TARGET& Out) const;
	_bool Can_StandTop(_float3& vOutStandPos) const;
	void  Fallback(ETraceurClimbHop eDir);

public:
	// 현재 잡은 바디(GrabBodyID)의 AABB 면에서 시작하는 방향별 탐색 구간 (anchor)
	static _bool Calc_ProbeSegment(const HANG_CONTEXT& Ctx, const struct HANG_TUNING& Tuning,
		ETraceurClimbHop eDir, _vector& vOutStart, _vector& vOutDir, _float& fOutDist);

#ifdef _DEBUG
private:
	void Draw_DebugProbe() const;

	mutable _bool   m_hasDebugProbe   = false;
	mutable _bool   m_isDebugProbeHit = false;
	mutable _float3 m_vDebugProbeStart{};
	mutable _float3 m_vDebugProbeEnd{};
	mutable _float3 m_vDebugProbeHitPos{};
#endif

public:
	static CTraceurClimbHop* Create(class CTraceur* pOwner);
	virtual void Free() override;
};
NS_END
