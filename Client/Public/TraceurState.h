#pragma once
#include "State.h"
#include "StateMachine.h"
#include "Client_Struct.h"

namespace Engine { class CAnimationController; }

NS_BEGIN(Client)

class CTraceurState abstract : public Engine::CState
{
protected:
	explicit CTraceurState() = default;
	virtual ~CTraceurState() = default;

public:
	virtual HRESULT Initialize(class CTraceur* pOwner);
	virtual void OnEnter(void* pArg = nullptr) override;
	virtual void OnUpdate(_float fTimeDelta) override;
	virtual void OnExit() override;

public:
	void Set_SelfKey(const Engine::StateKey& Key) { m_SelfKey = Key; }

public:
	_bool IsVault() const;
	virtual _bool Play_Animation(_float fTimeDelta);

protected:
	void  Request_Anim(_uint iAnimId);
	_uint Get_CurrentAnim() const;
	virtual _uint Get_CurrentAnimIndex() override;

protected:
	void  Set_Flag(const _string& strName, _bool isOn);
	_bool Get_Flag(const _string& strName) const;
	void  Clear_Flags();

protected:
	virtual void Check_State() {}
	virtual void Update_Animations(_float fTimeDelta) {}
	virtual void Check_Physics(_float fTimeDelta) {}
	virtual void Late_Anim_Update(_float fTimeDelta) {}
	virtual void SetUp_Animations() {}

	// 이번 프레임에 원하는 IK를 선언한다. IK가 필요 없는 상태는 오버라이드하지 않는다.
	// OnExit에서 IK를 정리할 필요 없음 — 요청이 사라지면 드라이버가 자동 블렌드 아웃.
public:
	virtual void Build_IKRequests(vector<IK_REQUEST>& Out) {}

protected:

protected:
	Engine::StateKey m_SelfKey{ 0, 0 };

#ifdef _DEBUG
	_float4x4 m_StartMatrix = {};
	_bool m_IsShowTrajectory = { false };
#endif // _DEBUG


#ifdef _DEBUG
protected:
	void Debug_PrintFlag();
#endif // _DEBUG




protected:
	class CTraceur*                    m_pOwner              = { nullptr };
	class CModel*                      m_pModelCom           = { nullptr };
	class CTransform*                  m_pTransformCom       = { nullptr };
	class CMeshAlignComponent*         m_pMeshAlignCom       = { nullptr };
	class CCollider*                   m_pColliderCom        = { nullptr };
	class CStateMachine*               m_pStateMachinCom     = { nullptr };
	class CInputController*            m_pInputControllerCom = { nullptr };
	class CEnvironmentQueryComponent*  m_pEnvQueryCom        = { nullptr };
	class CParkourDeciderComponent*    m_pDeciderCom         = { nullptr };
	class CMotionWarpingComponent*     m_pMotionWarpCom      = { nullptr };
	class CMovementComponent*          m_pMoveCom            = { nullptr };
	Engine::CAnimationController*      m_pAnimCtrlCom        = { nullptr };
	class CStateBlackboard*            m_pBlackboardCom      = { nullptr };
	class CClimbEvaluator*             m_pClimbEvalCom       = { nullptr };
	class CIKDriver*				   m_pIKDriverCom		 = { nullptr };

	const ENV_PERCEPTION&   Enter_Perception(void* pArg) const;
	const PARKOUR_DECISION& Enter_Decision(void* pArg) const;

protected:
	_uint m_iMoveKey = {};

public:
	virtual void Free() override;
};

NS_END
