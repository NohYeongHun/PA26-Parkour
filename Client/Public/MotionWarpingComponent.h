#pragma once
#include "Component.h"

namespace Engine { class CAnimator; }

NS_BEGIN(Client)
class CMotionWarpingComponent final : public Engine::CComponent
{
public:
	typedef struct tagMotionWarpDesc : Engine::CComponent::COMPONENT_DESC
	{
	}MOTION_WARP_DESC;

protected:
	explicit CMotionWarpingComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMotionWarpingComponent(const CMotionWarpingComponent& Prototype);
	virtual ~CMotionWarpingComponent() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize_Clone(void* pArg) override;

public:
	void Set_WarpTarget(const _string& strName, const _float3& vPos);
	void Set_WarpTarget(const _string& strName, const _float3& vPos, const _float4& qRot);
	void Clear_WarpTargets();
	void Abort_Warp();
	void On_WarpNotify(const _string& strName, _bool isStart,
	                   _float fWindowEndTrackPos, _bool bTrans, _bool bRot);

	// 루트모션 워프 직접 시작 — 노티파이 없이 상태 코드가 창 전체를 지정
	void Begin_RootWarp(const _float3& vTargetPos, const _float4* pTargetRot,
	                    _float fWindowEndTrackPos, _bool bTrans, _bool bRot);
	void End_RootWarp();

	void Begin_CurveWarp(_fvector vStart, _fvector vEnd, _float fApexOffsetY,
	                     _fvector vLookStart, _fvector vLookTarget);
	void Update_CurveWarp(_float fCurveT);
	void End_CurveWarp() { m_isCurveWarping = false; }
	_bool Is_CurveWarping() const { return m_isCurveWarping; }

#ifdef _DEBUG
	void Update_DebugTrail();
	void Reset_DebugTrail();
#endif

private:
	map<_string, WARP_TARGET> m_WarpTargets;
	Engine::CAnimator* m_pAnimator          = { nullptr };
	class CTransform*  m_pOwnerTransformCom = { nullptr };
	class CCollider*  m_pOwnerColliderCom  = { nullptr };

	_float3 m_vCurveP0 = {}, m_vCurveP1 = {}, m_vCurveP2 = {};
	_float3 m_vLookStart = {}, m_vLookTarget = {};
	_bool   m_isCurveWarping = false;

#ifdef _DEBUG
	void Draw_DebugTrail();
	void Draw_DebugCurveWarp();
	vector<_float3>   m_DebugTrail;
#endif

public:
	static CMotionWarpingComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
