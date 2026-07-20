#pragma once
#include "Component.h"
#include "Client_Struct.h"
#include "StateMachine.h"

NS_BEGIN(Client)

class CParkourDeciderComponent final : public Engine::CComponent
{
public:
	typedef struct tagParkourDeciderDesc : Engine::CComponent::COMPONENT_DESC
	{
		const BODY_PROFILE* pBodyProfile = nullptr;
	}PARKOUR_DECIDER_DESC;

protected:
	explicit CParkourDeciderComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CParkourDeciderComponent(const CParkourDeciderComponent& Prototype);
	virtual ~CParkourDeciderComponent() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize_Clone(void* pArg) override;

public:
	void Decide(const ENV_PERCEPTION& Perception, _float fTimeDelta);
	const PARKOUR_DECISION& Get_Decision() const { return m_Decision; }

	void Request_ScanDir(_fvector vDir, const Engine::StateKey& Requester);
	void Clear_ScanDir();

#ifdef _DEBUG
	void Print_Debug(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo);
#endif

private:
	void Judge(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo);
	ACTION_VERDICT Judge_Vault(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo, _float fApproachDot) const;
	ACTION_VERDICT Judge_Mantle(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo, _float fApproachDot) const;
	ACTION_VERDICT Judge_Climb(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo) const;
	ACTION_VERDICT Judge_Hang(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo) const;
	ACTION_VERDICT Judge_WallRun(const OBSTACLE_SCAN& Scan, const OBSTACLE_GEOMETRY& Geo, _float fApproachDot) const;
	_uint Get_ObjectFlagMask(const OBSTACLE_SCAN& Scan) const;

	void Update_Intent();
	void Update_Context(_float fTimeDelta);
	void Arbitrate();

private:
	const BODY_PROFILE*        m_pBodyProfile    = nullptr;
	class CParkourTuningTable* m_pTuning         = nullptr;
	class CInputController*    m_pInputCom       = nullptr;
	class CCollider*           m_pColliderCom    = nullptr;
	class CStateMachine*              m_pStateMachineCom = nullptr;
	class CEnvironmentQueryComponent* m_pEnvQueryCom     = nullptr;

	PARKOUR_DECISION m_Decision{};

	_bool            m_hasScanDirRequest = false;
	Engine::StateKey m_ScanDirRequester  = { 0, 0 };
	_float3          m_vRequestedScanDir = {};

	_float           m_fFallTime         = 0.f;
	_float           m_fWallRunCooldown  = 0.f;
	Engine::StateKey m_PrevStateKey      = { 0, 0 };

public:
	static CParkourDeciderComponent* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
