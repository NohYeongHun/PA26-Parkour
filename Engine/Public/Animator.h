#pragma once
#include "Base.h"
#include "Model.h" 

NS_BEGIN(Engine)
class CModel;
class CTransform;
class CComputeShader;

class ENGINE_DLL CAnimator final : public CBase
{
private:
	explicit CAnimator() = default;
	virtual ~CAnimator() = default;

public:
	HRESULT Initialize(CModel* pModel);

public:
	_bool	Play_Animation_CPU(const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isBlend = true, _bool isRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _float fRootMotionRate = 0.1f);
	_bool	Play_Animation_CPU(const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_BlendSpace_CPU(const BLENDSPACE_1D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_BlendSpace2D_CPU(const BLENDSPACE_2D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);

	void	Set_NextBlendOverride(_float fDuration) { m_fBlendOverride = fDuration; }

	void	Clear_Animation(const _string& strAnimationName, _float fTrackPosition = 0.f);

	const ROOT_MOTION_DELTA& Get_RootMotionDelta();
	const _bool	Is_MotionWarping() { return m_WarpState.isActive; }
	void	Begin_MotionWarp(const _float3& vTargetPos, const _float4* pTargetRot,
	                       _float fWindowEndTrackPos, _bool bTrans, _bool bRot, _bool isGravity = false);
	void	End_MotionWarp();
	void	Sync_RootNode(CTransform* pOwnerTransform, _float fTimeDelta);

	CModel* Get_Model() const { return m_pModel; }

private:
	void	Freeze_TransitionSource();
	void	Handle_PlaybackChange(ETransitionSourceType eNewType, const _string& strNewKey, _float fNewBlendIn);
	_float	Update_TransitionSource(_float fTimeDelta);
	void	Layer_BlendSpace1D(const BLENDSPACE_1D_DESC& desc, _float fParam, _float& fPhase, _float fLayerWeight, _float fTimeDelta);
	void	Layer_BlendSpace2D(const BLENDSPACE_2D_DESC& desc, _float fParamX, _float fParamY, _float& fPhase, _float fLayerWeight, _float fTimeDelta);
	void	Compute_RootAnimation(_float fRootMotionRate, _bool isRootMotionRotation, _bool isRootMotionTranslate, _bool IsRootMotionEnable = true);
	void	HandleAnimationChange(const _string& strAnimationName);
	_bool	Update_TrackPosition(class CAnimation* pAnimation, _float* pTrackPosition, _float fTimeDelta);
	_matrix	Compute_MotionWarpMatrix(CTransform* pOwnerTransform, _float fTimeDelta);

private:
	CModel* m_pModel = nullptr;   
	ETransitionSourceType     m_eCurPlayType = ETransitionSourceType::NONE;
	_string                   m_strCurPlayKey;
	_float                    m_fCurPlayTrackPos = 0.f;
	_float                    m_fCurPlaySpeed = 1.f;
	_float                    m_fCurBlendOut = 0.2f;
	const BLENDSPACE_1D_DESC* m_pCurBS1 = nullptr;
	const BLENDSPACE_2D_DESC* m_pCurBS2 = nullptr;
	TRANSITION_SOURCE         m_TransitionSource;
	_float                    m_fBlendOverride = -1.f;
	_string                   m_strPreAnimation;
	_float                    m_fBlendSpacePhase = 0.f;
	_bool                     m_isChangeAnimation = false;

	ROOT_MOTION_DELTA m_RootMotionDelta;
	ROOT_MOTION_DELTA m_SectionRootMotionDelta;
	MOTION_WARP_STATE m_WarpState{};
	_float            m_fCurRootMotionRate = 1.f;

	_matrix  m_RootMatrix = XMMatrixIdentity();
	_float4  m_vPreRootRotation = { 0.f, 0.f, 0.f, 1.f };
	_float4  m_vPreRootPosition = { 0.f, 0.f, 0.f, 1.f };

public:
	static CAnimator* Create(CModel* pModel);
	virtual void Free() override;
};
NS_END
