#pragma once
#include "Character.h"

namespace Engine { class CAnimationController; }

NS_BEGIN(Client)
class CTraceur final : public CCharacter
{
#pragma region 기본 함수들.
protected:
	explicit CTraceur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CTraceur(const CTraceur& Prototype);
	virtual ~CTraceur() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;

public:
	_vector Get_CamForward() const;
	_vector Get_CamRight()   const;
	const BODY_PROFILE* Get_BodyProfile() const { return &m_BodyProfile; }
	HANG_CONTEXT& Get_HangContext() { return m_HangCtx; }

public:
	void Notify_StateFlag(const _string& strFlag, _bool isOn);
	void On_IK_Notify(const vector<IK_BINDING>& bindings, _float blendSec, _bool isBegin);

public:
	void OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	void OnCollider_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

#ifdef _DEBUG
	_bool Show_DebugTrajectory() const { return m_IsShowTrajectory; } 
#endif // _DEBUG


private:
	// SingleTon
	class CGameSystem* m_pGameSystem = { nullptr };

private:
	// GameObjects
	class CSpringCamera* m_pSpringCamera = { nullptr };

private:
	// Components
	class CInputController* m_pInputControllerCom = { nullptr };
	class CRigidbody*	 m_pRigidbodyCom = { nullptr };
	class CCollider*	 m_pColliderCom = { nullptr };
	class CStateMachine* m_pStateMachineCom = { nullptr };
	class CEnvironmentQueryComponent* m_pEnvQueryCom = { nullptr };
	class CParkourDeciderComponent* m_pParkourDeciderCom = { nullptr };
	class CMotionWarpingComponent* m_pMotionWarpCom = { nullptr };
	

	class CMeshAlignComponent*  m_pMeshAlignCom = { nullptr };
	class CStateBlackboard*     m_pStateBlackboardCom = { nullptr };
	class CTransitionEvaluator* m_pTransitionEvalCom = { nullptr };
	class CClimbEvaluator*      m_pClimbEvalCom = { nullptr };

	class CIKComponent*			m_pIKCom = { nullptr };
	class CIKDriver*			m_pIKDriverCom = { nullptr };

private:
	BODY_PROFILE m_BodyProfile{};
	HANG_CONTEXT m_HangCtx{};


#ifdef _DEBUG
	// 루트모션 궤적 디버그
	_bool     m_IsShowTrajectory = { false };
	_float    m_fTrajectoryTimeStep = { 0.1f };
	_ubyte    m_iTrajectoryToggleKey = { DIK_5 }; 
	_float4x4 m_TrajectoryAnchor = {};
	_string   m_strTrajectoryAnchorAnim = {};
#endif

private:
	// Priority Update
	void PreUpdate_Input(_float fTimeDelta);
	void Save_PreviousPosition();
	void Handle_Input(_float fTimeDelta);

private:
	void Update_Physics(_float fTimeDelta);
	void Update_EnvQuery(_float fTimeDelta);
	void Sync_Camera(_float fTimeDelta);
	void Collect_StateFlags();
	void Bind_CollectSlots();


	struct COLLECT_SLOTS
	{
		_uint Grounded = UINT_MAX, Supported = UINT_MAX, Unsupported = UINT_MAX,
		      Falling = UINT_MAX, Airborne = UINT_MAX;
		_uint Move = UINT_MAX, Run = UINT_MAX, Jump = UINT_MAX,
		      Forward = UINT_MAX, Down = UINT_MAX,
		      Left = UINT_MAX, Right = UINT_MAX, JumpPress = UINT_MAX;
		_uint CmdLowVault = UINT_MAX, CmdHighVault = UINT_MAX, CmdHighMantle = UINT_MAX,
			  CmdLowMantle = UINT_MAX, CmdClimb = UINT_MAX, CmdHang = UINT_MAX,
			  CmdWallRun = UINT_MAX;
		_uint EvalFall = UINT_MAX, EvalLand = UINT_MAX, EvalArrive = UINT_MAX,
		      EvalClimbMantle = UINT_MAX, EvalKneeHit = UINT_MAX;
	} m_CollectSlots;

private:
	// Late Update
	void Sync_Transform();
	void Drive_IK(_float fTimeDelta);
	void Ready_Render();


private:
	HRESULT Ready_Components(const CHARACTER_DESC* pDesc);
	HRESULT Ready_EnvQueryComponents(const CHARACTER_DESC* pDesc);

	HRESULT Ready_Variables(const CHARACTER_DESC* pDesc);
	HRESULT Bind_Matrices();

private:
	// Helper
	void Update_Collider(_float fTimeDelta);

public:
	static		CTraceur* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

