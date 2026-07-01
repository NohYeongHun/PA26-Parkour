#pragma once
#include "Camera.h"

NS_BEGIN(Client)

class CSpringCamera final : public CCamera
{
public:
	enum class CAMERA_STATE { TARGET, SPRING, LOCKON, ACTION };
private:
	explicit CSpringCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSpringCamera(const CSpringCamera& Prototype);
	virtual ~CSpringCamera() = default;

public:
	void							Set_Distance(_float fDistance) { m_fDistance += fDistance; }
	void							Set_FixedDistance(_float fFixedDistance) { m_fFixedDistance = fFixedDistance; }

	// SetUp Target Pos, Offset Y
	void							Update_Target(const _fvector& TargetPos, _float fOffsetY);

	// Spring (Distance Adjust) - Lerp
	// 목표 Distance, 지속 시간
	void							Use_Spring(_float fDestination, _float fDuration)
	{
		if (CAMERA_STATE::SPRING == m_eCameraState)
			return;
		m_fDestination = fDestination;
		m_fSpringDuration = fDuration;
		m_eCameraState = CAMERA_STATE::SPRING;
	}
	// Lock-On
	void							Lock_On(class CTransform* pTargetTransform, const _float4x4* pBoneMatrix, _bool IsLockOn);

public:
	_vector Get_LookVector();
	_vector Get_LookVector_NoPitch();
	_vector Get_RightVector_NoPitch();

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;

private:
	class CGameSystem*		m_pGameSystem = { nullptr };
	CAMERA_STATE			m_eCameraState = { CAMERA_STATE::TARGET };
	_float4						m_vLookPosition = {};

	_float4						m_vTargetPosition = {};		// Target Pos
	_float							m_fOffsetY = {};				// Target Pos Y + OffsetY <= Look

	// Distance Lerp
	_float							m_fFixedDistance = {};
	_float							m_fLerpSpeed = {};
	_float							m_fMinDistance = {};
	_float							m_fLockOnMinDistance = {};
	_float							m_fMaxDistance = {};

	// Spring
	_float							m_fStiffness = {};		// Spring Force
	_float							m_fDestination = {};	// Spring Destination
	_float							m_fSpringDuration = {};

	// Lock-On
	CTransform*				m_pTargetTransform = { nullptr };
	_float4						m_LockOnPosition = {};
	_float							m_fLockOnOffsetY = {};
	_float							m_fLockOnDistanceOffset = {};
	_float							m_fRatio = {};

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
	_float4x4							m_OwnerMatrix = {};
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
	void							Dual_Targeting(_float fTimeDelta);			// Dual Target Compute
	void							Dynamic_Distance();
	void							Adjust_LockOn_Distance();
	void							Dynamic_Fov(_float fTimeDelta);

	// Action
	void							Action(_float fTimeDelta);
	void							Recovery(_float fTimeDelta);
	void							SetUp_Recovery();

private:
	void							Ready_Event();

public:
	static		CSpringCamera*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END