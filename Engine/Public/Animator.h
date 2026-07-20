#pragma once
#include "Base.h"
#include "Model.h" 

NS_BEGIN(Engine)
class CModel;
class CTransform;
class CComputeShader;

class ENGINE_DLL CAnimator final : public CBase
{
public:
	struct PARAM_SNAPSHOT
	{
		map<_string, _float>  Floats;
		map<_string, _float2> Float2s;
		map<_string, _float3> Float3s;

		_float  Get_Float(const _string& strName, _float fDefault = 0.f) const;
		_float2 Get_Float2(const _string& strName) const;
		_float3 Get_Float3(const _string& strName) const;
	};

private:
	explicit CAnimator() = default;
	virtual ~CAnimator() = default;

public:
	HRESULT Initialize(CModel* pModel);

	void Bind_Parameter(const _string& strName, const _float* pValue);
	void Bind_Parameter2D(const _string& strName, const _float2* pValue);
	void Bind_Parameter3D(const _string& strName, const _float3* pValue);
	const PARAM_SNAPSHOT& Get_ParamSnapshot() const { return m_Snapshot; }

	void Set_OwnerWorld(const _float4x4* pWorldMatrix) { m_pOwnerWorld = pWorldMatrix; }

public:
	_bool	Play_Animation_CPU(const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isBlend = true, _bool isRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _float fRootMotionRate = 0.1f);
	_bool	Play_Animation_CPU(const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_BlendSpace_CPU(const BLENDSPACE_1D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_BlendSpace2D_CPU(const BLENDSPACE_2D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);

	void	Set_NextBlendOverride(_float fDuration) { m_fBlendOverride = fDuration; }

	void	Update_Phase();
	void	Evaluate_Phase();

	void	Clear_Animation(const _string& strAnimationName, _float fTrackPosition = 0.f);

	const ROOT_MOTION_DELTA& Get_RootMotionDelta();
	const _bool	Is_MotionWarping() { return m_WarpState.isActive; }
	void	Begin_MotionWarp(const _float3& vTargetPos, const _float4* pTargetRot,
	                       _float fWindowEndTrackPos, _bool bTrans, _bool bRot, _bool isGravity = false);
	void	End_MotionWarp();
	void	Sync_RootNode(CTransform* pOwnerTransform, _float fTimeDelta);

	_bool	Play_Animation_GPU(CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_Animation_GPU(CComputeShader* pComputeShaderCom, CComputeShader* pMorphComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_NonRibAnimation_GPU(CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta);
	_bool	Play_FlyAnimation_GPU(CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, const GPU_BLEND_INFO& gpuBlendInfo, _float fTimeDelta);
	_bool	Play_FlyAnimation_GPU(CComputeShader* pComputeShaderCom, CComputeShader* pMorphComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, const GPU_BLEND_INFO& gpuBlendInfo, _float fTimeDelta);

	CModel* Get_Model() const { return m_pModel; }

private:
	void	Snapshot_Params();

	void	Freeze_TransitionSource();
	void	Handle_PlaybackChange(ETransitionSourceType eNewType, const _string& strNewKey, _float fNewBlendIn);

	_float	Advance_TransitionSource(_float fTimeDelta);
	void	Sample_TransitionSource();

	struct BS1_PICK { class CAnimation* pA = nullptr; class CAnimation* pB = nullptr; _float fU = 0.f; };
	struct BS2_PICK { class CAnimation* pI = nullptr; class CAnimation* pX = nullptr; class CAnimation* pY = nullptr; _float wI = 0.f, wX = 0.f, wY = 0.f; };
	_bool	Pick_BlendSpace1D(const BLENDSPACE_1D_DESC& desc, _float fParam, BS1_PICK& Out);
	_bool	Pick_BlendSpace2D(const BLENDSPACE_2D_DESC& desc, _float fParamX, _float fParamY, BS2_PICK& Out);
	void	Advance_BlendSpacePhase1D(const BLENDSPACE_1D_DESC& desc, _float fParam, _float& fPhase, _float fTimeDelta);
	void	Advance_BlendSpacePhase2D(const BLENDSPACE_2D_DESC& desc, _float fParamX, _float fParamY, _float& fPhase, _float fTimeDelta);
	void	Sample_BlendSpace1D(const BLENDSPACE_1D_DESC& desc, _float fParam, _float fPhase, _float fLayerWeight);
	void	Sample_BlendSpace2D(const BLENDSPACE_2D_DESC& desc, _float fParamX, _float fParamY, _float fPhase, _float fLayerWeight);
	void	Compute_RootAnimation(_float fRootMotionRate, _bool isRootMotionRotation, _bool isRootMotionTranslate, _bool IsRootMotionEnable = true);
	void	HandleAnimationChange(const _string& strAnimationName);
	_bool	Update_TrackPosition(class CAnimation* pAnimation, _float* pTrackPosition, _float fTimeDelta);
	_matrix	Compute_MotionWarpMatrix(CTransform* pOwnerTransform, _float fTimeDelta);

	void	FetchLocalMatrices_FromCompute(CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName);
	void	FetchLocalMatrices_FromComputeFly(CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName, const GPU_BLEND_INFO& gpuBlendInfo);
	void	FetchLocalMatrices_FromComputeNonRib(CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName);
	void	Update_NonRibAnimConstantBuffer(const _string& strAnimationName, _float fTrackPosition);
	void	Update_AnimConstantBuffer(const _string& strAnimationName, _float fTrackPosition);
	void	Update_FlyAnimConstantBuffer(const GPU_BLEND_INFO& gpuBlendInfo, const _string& strAnimationName, _float fTrackPosition);
	void	Bind_AnimationResource(CComputeShader* pComputeShaderCom);
	void	Bind_FlyAnimationResource(CComputeShader* pComputeShaderCom);
	void	Readback_BoneMatrices();
	void	Update_MorphAnimation(class CAnimation* pAnimation, CComputeShader* pMorphComputeShaderCom, _float fTimeDelta, _bool isFacial);

private:
	CModel* m_pModel = nullptr;

	map<_string, const _float*>  m_BoundFloats;
	map<_string, const _float2*> m_BoundFloat2s;
	map<_string, const _float3*> m_BoundFloat3s;
	PARAM_SNAPSHOT   m_Snapshot;
	const _float4x4* m_pOwnerWorld = nullptr;
	_float4x4        m_SnapWorldMatrix{};

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

	enum class EPlayRequest { NONE, CLIP, BLENDSPACE_1D, BLENDSPACE_2D };
	EPlayRequest              m_eRequest = EPlayRequest::NONE;
	ANIMATION_PLAY_DESC       m_ReqClipDesc{};        
	const BLENDSPACE_1D_DESC* m_pReqBS1 = nullptr;    
	const BLENDSPACE_2D_DESC* m_pReqBS2 = nullptr;
	ROOTMOTION_DESC           m_ReqRootDesc{};
	_float                    m_fReqDt = 0.f;

	_float m_fFrameWeight = 1.f;      
	_float m_fFrameTrack = 0.f;       
	_bool  m_isFrameAnimEnd = false;
	_float m_fSrcSampleTrack = 0.f;   
	_float m_fFrameBSParamX = 0.f;    
	_float m_fFrameBSParamY = 0.f;

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
