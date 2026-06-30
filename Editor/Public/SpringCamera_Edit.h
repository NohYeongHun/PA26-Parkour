#pragma once
#include "Camera.h"

NS_BEGIN(Engine)
class CCollider;
class CRigidbody;
NS_END

NS_BEGIN(Editor)

class CSpringCamera_Edit final : public CCamera
{
public:
	enum class CAMERA_STATE { TARGET, SPRING, LOCKON, ACTION, CUTSCENE };
private:
	explicit CSpringCamera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSpringCamera_Edit(const CSpringCamera_Edit& Prototype);
	virtual ~CSpringCamera_Edit() = default;

public:
	void							Set_Distance(_float fDistance) { m_fDistance += fDistance; }
	void							Set_FixedDistance(_float fFixedDistance) { m_fFixedDistance = fFixedDistance; }

	// SetUp Target Pos, Offset Y
	void							Update_Target(const _fvector& TargetPos, _float fOffsetY)
	{ XMStoreFloat4(&m_vTargetPosition, TargetPos); m_fOffsetY = fOffsetY; };
	// Spring (Distance Adjust) - Lerp
	// ��ǥ Distance, ���� �ð�
	void							Use_Spring(_float fDestination, _float fDuration)
	{
		if (CAMERA_STATE::SPRING == m_eCameraState)
			return;
		m_fDestination = fDestination;
		m_fSpringDuration = fDuration;
		m_eCameraState = CAMERA_STATE::SPRING;
	}
	// Lock-On
	void							Lock_On()
	{
		//if (CAMERA_STATE::LOCKON == m_eCameraState)
		//{
		//	m_eCameraState = CAMERA_STATE::TARGET;
		//	m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::NONE));
		//}
		//else if (CAMERA_STATE::TARGET == m_eCameraState)
		//{
		//	m_eCameraState = CAMERA_STATE::LOCKON;
		//	m_pRigidbodyCom->Change_Layer(ENUM_CLASS(COLLISIONLAYER::CAMERA));
		//}
	}

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;

	void							OnCollide_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

private:
	CAMERA_STATE			m_eCameraState = { CAMERA_STATE::TARGET };
	_float4						m_vLookPosition = {};
	// Detect Collider
	CRigidbody*				m_pRigidbodyCom = { nullptr };

	_float4						m_vTargetPosition = {};		// Target Pos
	_float							m_fOffsetY = {};				// Target Pos Y + OffsetY <= Look

	// Distance Lerp
	_float							m_fFixedDistance = {};
	_float							m_fLerpSpeed = {};
	_float							m_fMinDistance = {};

	// Spring
	_float							m_fStiffness = {};		// Spring Force
	_float							m_fDestination = {};	// Spring Destination
	_float							m_fSpringDuration = {};

	// Lock-On
	vector<CTransform*>		m_TargetTransforms;
	CTransform*				m_pTargetTransform = { nullptr };
	_float							m_fLockOnOffsetY = {};

	// Action
	vector<CAMERA_FRAME>	m_Frames;
	_float								m_fFirstFrame = {};
	_int								m_iFrameIndex = { -1 };
	_float								m_fTrackPerSec = { 10.f };
	_bool								m_isRecovery = { false };
	_float4							m_vPreQuaternion = {};
	_float3							m_vPreTranslation = {};
	_float								m_fPreFovy = {};
	_float4							m_vEndQuaternion = {};
	_float3							m_vEndTranslation = {};
	_float								m_fPreFixedDistance = {};
	_float								m_fTrackPosition = {};
	_float								m_fDuration = {};
	_bool								m_isMaintain = { false };
	_bool								m_isLerp = { true };
	_bool								m_isEscape = { false };

private:
	// Default
	void							Lerp_Distance(_float fTimeDelta);
	void							Mouse_Scroll(_float fTimeDelta);
	
	// Spring
	void							Spring(_float fTimeDelta);
	
	// Target
	void							Compute_CamPos();
	void							Check_Ray();

	// LockOn
	void							Lerp_Move(_float fTimeDelta);				// Quat Lerp
	void							Sorting_Target();								// Target Transforms Sort (Distance Less)
	void							Dual_Targeting(_float fTimeDelta);			// Dual Target Compute
	void							Dynamic_Distance();

	// Action
	void							Action(_float fTimeDelta);
	void							Recovery(_float fTimeDelta);
	void							SetUp_Recovery();

private:
	void							Ready_Component();
	void							Ready_Event();

public:
	static		CSpringCamera_Edit*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*				Clone(void* pArg) override;
	virtual		void							Free() override;
};

NS_END