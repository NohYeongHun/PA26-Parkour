#include "EnginePch.h"
#include "Animator.h"
#include "Model.h"
#include "Transform.h"
#include "Bone.h"
#include "Animation.h"
#include "ComputeShader.h"
#include "Mesh.h"

namespace {
	constexpr Engine::_float MW_EPS = 1e-4f;
}

NS_BEGIN(Engine)

HRESULT CAnimator::Initialize(CModel* pModel)
{
	if (nullptr == pModel)
		return E_FAIL;

	m_pModel = pModel;
	Safe_AddRef(m_pModel);

	return S_OK;
}

_float CAnimator::PARAM_SNAPSHOT::Get_Float(const _string& strName, _float fDefault) const
{
	auto iter = Floats.find(strName);
	return (iter != Floats.end()) ? iter->second : fDefault;
}

_float2 CAnimator::PARAM_SNAPSHOT::Get_Float2(const _string& strName) const
{
	auto iter = Float2s.find(strName);
	return (iter != Float2s.end()) ? iter->second : _float2{ 0.f, 0.f };
}

_float3 CAnimator::PARAM_SNAPSHOT::Get_Float3(const _string& strName) const
{
	auto iter = Float3s.find(strName);
	return (iter != Float3s.end()) ? iter->second : _float3{ 0.f, 0.f, 0.f };
}

void CAnimator::Bind_Parameter(const _string& strName, const _float* pValue)
{
	m_BoundFloats[strName] = pValue;
}

void CAnimator::Bind_Parameter2D(const _string& strName, const _float2* pValue)
{
	m_BoundFloat2s[strName] = pValue;
}

void CAnimator::Bind_Parameter3D(const _string& strName, const _float3* pValue)
{
	m_BoundFloat3s[strName] = pValue;
}

void CAnimator::Snapshot_Params()
{
	for (auto& Pair : m_BoundFloats)
		m_Snapshot.Floats[Pair.first] = *Pair.second;
	for (auto& Pair : m_BoundFloat2s)
		m_Snapshot.Float2s[Pair.first] = *Pair.second;
	for (auto& Pair : m_BoundFloat3s)
		m_Snapshot.Float3s[Pair.first] = *Pair.second;

	if (nullptr != m_pOwnerWorld)
		m_SnapWorldMatrix = *m_pOwnerWorld;
}

void CAnimator::Clear_Animation(const _string& strAnimationName, _float fTrackPosition)
{
	if (strAnimationName == "")
		return;

	m_isChangeAnimation = true;
	m_vPreRootRotation = _float4(0.f, 0.f, 0.f, 1.f);
	m_vPreRootPosition = _float4(0.f, 0.f, 0.f, 1.f);
	m_RootMatrix = XMMatrixIdentity();
	End_MotionWarp();

	m_pModel->Clear_Animation(strAnimationName, fTrackPosition);
}

_bool CAnimator::Play_Animation_CPU(const _string& strAnimationName, _float fTimeDelta, _float* pTrackPosition, _bool isBlend, _bool isRootMotion, _bool IsRootMotionRotate, _bool IsRootMotionTranslate, _float fRootMotionRate)
{
	auto iter = m_pModel->m_Animations.find(strAnimationName);
	if (iter == m_pModel->m_Animations.end())
		return false;

	_float fTrackPosition = {};

	if (m_strPreAnimation != strAnimationName)
	{
		m_isChangeAnimation = true;
		m_strPreAnimation = strAnimationName;
		Clear_Animation(strAnimationName);
	}

	_bool IsAnimationEnd = iter->second->Update_TransformationMatrices_All(fTimeDelta, m_pModel->m_Bones, &fTrackPosition);
	if (nullptr != pTrackPosition)
		*pTrackPosition = fTrackPosition;

	if (true == isRootMotion)
		Compute_RootAnimation(fRootMotionRate, IsRootMotionRotate, IsRootMotionTranslate);

	if (IsAnimationEnd)
	{
		Clear_Animation(strAnimationName);
		return true;
	}

	for (auto& pBone : m_pModel->m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);

	return false;
}

_bool CAnimator::Play_Animation_CPU(const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	auto iter = m_pModel->m_Animations.find(playDesc.strAnimationName);
	if (iter == m_pModel->m_Animations.end())
		return false;

	m_eRequest = EPlayRequest::CLIP;
	m_ReqClipDesc = playDesc;
	m_ReqRootDesc = rootMotionDesc;
	m_fReqDt = fTimeDelta;

	Update_Phase();
	Evaluate_Phase();

	return m_isFrameAnimEnd;
}

_bool CAnimator::Play_BlendSpace_CPU(const BLENDSPACE_1D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	if (nullptr == desc.pParam || desc.Samples.size() < 2)
		return false;

	m_eRequest = EPlayRequest::BLENDSPACE_1D;
	m_pReqBS1 = &desc;
	m_ReqRootDesc = rootMotionDesc;
	m_fReqDt = fTimeDelta;

	Update_Phase();
	Evaluate_Phase();

	return false;
}

_bool CAnimator::Play_BlendSpace2D_CPU(const BLENDSPACE_2D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	if (nullptr == desc.pParam || desc.Samples.size() < 3)
		return false;

	m_eRequest = EPlayRequest::BLENDSPACE_2D;
	m_pReqBS2 = &desc;
	m_ReqRootDesc = rootMotionDesc;
	m_fReqDt = fTimeDelta;

	Update_Phase();
	Evaluate_Phase();

	return false;
}

// Update phase: last point that reads game state. Snapshots parameters, decides
// playback change, advances tracks/phases/crossfade and fires notifies.
// No bone is written here.
void CAnimator::Update_Phase()
{
	Snapshot_Params();
	m_isFrameAnimEnd = false;
	m_fFrameWeight = 1.f;

	switch (m_eRequest)
	{
	case EPlayRequest::CLIP:
	{
		auto iter = m_pModel->m_Animations.find(m_ReqClipDesc.strAnimationName);
		if (iter == m_pModel->m_Animations.end())
		{
			m_eRequest = EPlayRequest::NONE;
			return;
		}

		Handle_PlaybackChange(ETransitionSourceType::CLIP, m_ReqClipDesc.strAnimationName, m_ReqClipDesc.fBlendIn);
		m_fCurBlendOut = m_ReqClipDesc.fBlendOut;
		m_fCurPlaySpeed = m_ReqClipDesc.fSpeed;
		m_pCurBS1 = nullptr;
		m_pCurBS2 = nullptr;

		m_fFrameWeight = Advance_TransitionSource(m_fReqDt);

		m_isFrameAnimEnd = iter->second->Update_TrackPosition(m_ReqClipDesc.fSpeed * m_fReqDt, &m_fFrameTrack);

		if (nullptr != m_ReqClipDesc.pTrackPosition)
			*m_ReqClipDesc.pTrackPosition = m_fFrameTrack;
		m_fCurPlayTrackPos = m_fFrameTrack;
		break;
	}
	case EPlayRequest::BLENDSPACE_1D:
	{
		Handle_PlaybackChange(ETransitionSourceType::BLENDSPACE_1D, m_pReqBS1->Samples[0].strAnimationName, m_pReqBS1->fBlendIn);
		m_fCurBlendOut = m_pReqBS1->fBlendOut;
		m_fCurPlaySpeed = m_pReqBS1->fPlayRate;
		m_pCurBS1 = m_pReqBS1;
		m_pCurBS2 = nullptr;

		m_fFrameWeight = Advance_TransitionSource(m_fReqDt);

		m_fFrameBSParamX = *m_pReqBS1->pParam;
		Advance_BlendSpacePhase1D(*m_pReqBS1, m_fFrameBSParamX, m_fBlendSpacePhase, m_fReqDt);
		break;
	}
	case EPlayRequest::BLENDSPACE_2D:
	{
		Handle_PlaybackChange(ETransitionSourceType::BLENDSPACE_2D, m_pReqBS2->Samples[0].strAnimationName, m_pReqBS2->fBlendIn);
		m_fCurBlendOut = m_pReqBS2->fBlendOut;
		m_fCurPlaySpeed = m_pReqBS2->fPlayRate;
		m_pCurBS1 = nullptr;
		m_pCurBS2 = m_pReqBS2;

		m_fFrameWeight = Advance_TransitionSource(m_fReqDt);

		m_fFrameBSParamX = m_pReqBS2->pParam->x;
		m_fFrameBSParamY = m_pReqBS2->pParam->y;
		Advance_BlendSpacePhase2D(*m_pReqBS2, m_fFrameBSParamX, m_fFrameBSParamY, m_fBlendSpacePhase, m_fReqDt);
		break;
	}
	default:
		break;
	}
}

void CAnimator::Evaluate_Phase()
{
	switch (m_eRequest)
	{
	case EPlayRequest::CLIP:
	{
		auto iter = m_pModel->m_Animations.find(m_ReqClipDesc.strAnimationName);
		if (iter == m_pModel->m_Animations.end())
			break;

		if (m_fFrameWeight < 1.f)
		{
			Sample_TransitionSource();
			iter->second->Blend_AtTrackPosition(m_fFrameTrack, m_pModel->m_Bones, m_fFrameWeight);
			iter->second->Sample_BoneAtTrackPosition(m_fFrameTrack, m_pModel->m_Bones, m_pModel->m_iRootBoneIndex);
		}
		else if (false == m_isFrameAnimEnd)
		{
			iter->second->Sample_AtTrackPosition(m_fFrameTrack, m_pModel->m_Bones);
		}

		Compute_RootAnimation(m_ReqRootDesc.fRate, m_ReqRootDesc.isRotate, m_ReqRootDesc.isTranslate, m_ReqRootDesc.isEnable);

		if (m_isFrameAnimEnd)
		{
			Clear_Animation(m_ReqClipDesc.strAnimationName);
			m_TransitionSource.eType = ETransitionSourceType::NONE;
			break;
		}

		for (auto& pBone : m_pModel->m_Bones)
			pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);
		break;
	}
	case EPlayRequest::BLENDSPACE_1D:
	{
		if (m_fFrameWeight < 1.f)
			Sample_TransitionSource();
		Sample_BlendSpace1D(*m_pReqBS1, m_fFrameBSParamX, m_fBlendSpacePhase, m_fFrameWeight);

		Compute_RootAnimation(m_ReqRootDesc.fRate,
			m_ReqRootDesc.isEnable && m_ReqRootDesc.isRotate,
			m_ReqRootDesc.isEnable && m_ReqRootDesc.isTranslate);

		for (auto& pBone : m_pModel->m_Bones)
			pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);
		break;
	}
	case EPlayRequest::BLENDSPACE_2D:
	{
		if (m_fFrameWeight < 1.f)
			Sample_TransitionSource();
		Sample_BlendSpace2D(*m_pReqBS2, m_fFrameBSParamX, m_fFrameBSParamY, m_fBlendSpacePhase, m_fFrameWeight);

		Compute_RootAnimation(m_ReqRootDesc.fRate,
			m_ReqRootDesc.isEnable && m_ReqRootDesc.isRotate,
			m_ReqRootDesc.isEnable && m_ReqRootDesc.isTranslate);

		for (auto& pBone : m_pModel->m_Bones)
			pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);
		break;
	}
	default:
		break;
	}

	m_eRequest = EPlayRequest::NONE;
}

void CAnimator::Freeze_TransitionSource()
{
	m_TransitionSource.FrozenPose.resize(m_pModel->m_Bones.size());
	for (size_t i = 0; i < m_pModel->m_Bones.size(); ++i)
		m_TransitionSource.FrozenPose[i] = *m_pModel->m_Bones[i]->Get_TransformationMatrix();
	m_TransitionSource.eType = ETransitionSourceType::FROZEN;
}

void CAnimator::Handle_PlaybackChange(ETransitionSourceType eNewType, const _string& strNewKey, _float fNewBlendIn)
{
	if (m_eCurPlayType == eNewType && m_strCurPlayKey == strNewKey)
	{
		m_fBlendOverride = -1.f;
		return;
	}

	_float fDuration;
	if (m_fBlendOverride >= 0.f)
	{
		fDuration = m_fBlendOverride;
		m_fBlendOverride = -1.f;
	}
	else
		fDuration = min(m_fCurBlendOut, fNewBlendIn);

	if (ETransitionSourceType::NONE == m_eCurPlayType || fDuration <= 0.f)
	{
		m_TransitionSource.eType = ETransitionSourceType::NONE;
	}
	else if (ETransitionSourceType::NONE != m_TransitionSource.eType)
	{
		Freeze_TransitionSource();
		m_TransitionSource.fElapsed = 0.f;
		m_TransitionSource.fDuration = fDuration;
	}
	else
	{
		m_TransitionSource.eType = m_eCurPlayType;
		m_TransitionSource.fElapsed = 0.f;
		m_TransitionSource.fDuration = fDuration;
		switch (m_eCurPlayType)
		{
		case ETransitionSourceType::CLIP:
			m_TransitionSource.strClip = m_strCurPlayKey;
			m_TransitionSource.fTrackPos = m_fCurPlayTrackPos;
			m_TransitionSource.fSpeed = m_fCurPlaySpeed;
			break;
		case ETransitionSourceType::BLENDSPACE_1D:
			m_TransitionSource.BS1 = *m_pCurBS1;
			m_TransitionSource.fFrozenParamX = *m_pCurBS1->pParam;
			m_TransitionSource.fPhase = m_fBlendSpacePhase;
			break;
		case ETransitionSourceType::BLENDSPACE_2D:
			m_TransitionSource.BS2 = *m_pCurBS2;
			m_TransitionSource.fFrozenParamX = m_pCurBS2->pParam->x;
			m_TransitionSource.fFrozenParamY = m_pCurBS2->pParam->y;
			m_TransitionSource.fPhase = m_fBlendSpacePhase;
			break;
		}
	}

	if (ETransitionSourceType::CLIP == eNewType)
		Clear_Animation(strNewKey);
	else
	{
		m_fBlendSpacePhase = 0.f;
		m_isChangeAnimation = true;
		m_vPreRootRotation = _float4(0.f, 0.f, 0.f, 1.f);
		m_vPreRootPosition = _float4(0.f, 0.f, 0.f, 1.f);
		m_RootMatrix = XMMatrixIdentity();
	}

	m_eCurPlayType = eNewType;
	m_strCurPlayKey = strNewKey;
}

_float CAnimator::Advance_TransitionSource(_float fTimeDelta)
{
	if (ETransitionSourceType::NONE == m_TransitionSource.eType)
		return 1.f;

	m_TransitionSource.fElapsed += fTimeDelta;
	if (m_TransitionSource.fElapsed >= m_TransitionSource.fDuration)
	{
		m_TransitionSource.eType = ETransitionSourceType::NONE;
		return 1.f;
	}

	// Ease In / Out
	_float t = Saturate(m_TransitionSource.fElapsed / m_TransitionSource.fDuration);
	_float fWeight = t * t * (3.f - 2.f * t);

	switch (m_TransitionSource.eType)
	{
	case ETransitionSourceType::CLIP:
	{
		auto iter = m_pModel->m_Animations.find(m_TransitionSource.strClip);
		if (iter == m_pModel->m_Animations.end())
		{
			Freeze_TransitionSource();
			break;
		}
		m_fSrcSampleTrack = m_TransitionSource.fTrackPos;
		m_TransitionSource.fTrackPos += iter->second->Get_TickPerSecond() * m_TransitionSource.fSpeed * fTimeDelta;
		break;
	}
	case ETransitionSourceType::BLENDSPACE_1D:
		Advance_BlendSpacePhase1D(m_TransitionSource.BS1, m_TransitionSource.fFrozenParamX,
			m_TransitionSource.fPhase, fTimeDelta);
		break;
	case ETransitionSourceType::BLENDSPACE_2D:
		Advance_BlendSpacePhase2D(m_TransitionSource.BS2, m_TransitionSource.fFrozenParamX,
			m_TransitionSource.fFrozenParamY, m_TransitionSource.fPhase, fTimeDelta);
		break;
	case ETransitionSourceType::FROZEN:
		break;
	}

	return fWeight;
}

void CAnimator::Sample_TransitionSource()
{
	switch (m_TransitionSource.eType)
	{
	case ETransitionSourceType::CLIP:
	{
		auto iter = m_pModel->m_Animations.find(m_TransitionSource.strClip);
		if (iter == m_pModel->m_Animations.end())
			break;
		iter->second->Sample_AtTrackPosition(m_fSrcSampleTrack, m_pModel->m_Bones);
		break;
	}
	case ETransitionSourceType::BLENDSPACE_1D:
		Sample_BlendSpace1D(m_TransitionSource.BS1, m_TransitionSource.fFrozenParamX,
			m_TransitionSource.fPhase, 1.f);
		break;
	case ETransitionSourceType::BLENDSPACE_2D:
		Sample_BlendSpace2D(m_TransitionSource.BS2, m_TransitionSource.fFrozenParamX,
			m_TransitionSource.fFrozenParamY, m_TransitionSource.fPhase, 1.f);
		break;
	case ETransitionSourceType::FROZEN:
		for (size_t i = 0; i < m_pModel->m_Bones.size(); ++i)
			m_pModel->m_Bones[i]->Set_TransformationMatrix(XMLoadFloat4x4(&m_TransitionSource.FrozenPose[i]));
		break;
	}
}

_bool CAnimator::Pick_BlendSpace1D(const BLENDSPACE_1D_DESC& desc, _float fParam, BS1_PICK& Out)
{
	if (desc.Samples.size() < 2)
		return false;

	_uint iA = 0;
	_uint iB = 1;
	const _uint iCount = static_cast<_uint>(desc.Samples.size());
	for (_uint i = 0; i + 1 < iCount; ++i)
	{
		if (fParam <= desc.Samples[i + 1].fXParamValue)
		{
			iA = i;
			iB = i + 1;
			break;
		}
		iA = i;
		iB = i + 1;
	}

	auto iterA = m_pModel->m_Animations.find(desc.Samples[iA].strAnimationName);
	auto iterB = m_pModel->m_Animations.find(desc.Samples[iB].strAnimationName);
	if (iterA == m_pModel->m_Animations.end() || iterB == m_pModel->m_Animations.end())
		return false;

	_float fRange = desc.Samples[iB].fXParamValue - desc.Samples[iA].fXParamValue;
	_float fU = (fRange > 0.f) ? ((fParam - desc.Samples[iA].fXParamValue) / fRange) : 0.f;
	fU = max(0.f, min(1.f, fU));

	Out.pA = iterA->second;
	Out.pB = iterB->second;
	Out.fU = fU;
	return true;
}

void CAnimator::Advance_BlendSpacePhase1D(const BLENDSPACE_1D_DESC& desc, _float fParam, _float& fPhase, _float fTimeDelta)
{
	BS1_PICK Pick{};
	if (false == Pick_BlendSpace1D(desc, fParam, Pick))
		return;

	_float fDurA = Pick.pA->Get_Duration();
	_float fDurB = Pick.pB->Get_Duration();
	_float fTickA = Pick.pA->Get_TickPerSecond();
	_float fTickB = Pick.pB->Get_TickPerSecond();
	_float fRealDurA = (fTickA > 0.f) ? (fDurA / fTickA) : fDurA;
	_float fRealDurB = (fTickB > 0.f) ? (fDurB / fTickB) : fDurB;
	_float fBlendedDur = fRealDurA + Pick.fU * (fRealDurB - fRealDurA);
	if (fBlendedDur > 0.f)
		fPhase += (fTimeDelta * desc.fPlayRate) / fBlendedDur;
	if (fPhase >= 1.f)
		fPhase -= 1.f;
}

void CAnimator::Sample_BlendSpace1D(const BLENDSPACE_1D_DESC& desc, _float fParam, _float fPhase, _float fLayerWeight)
{
	if (fLayerWeight <= 0.f)
		return;

	BS1_PICK Pick{};
	if (false == Pick_BlendSpace1D(desc, fParam, Pick))
		return;

	_float fTrackA = fPhase * Pick.pA->Get_Duration();
	_float fTrackB = fPhase * Pick.pB->Get_Duration();

	_float wA = fLayerWeight * (1.f - Pick.fU);
	_float wB = fLayerWeight * Pick.fU;
	_float S = 1.f - fLayerWeight;
	if (wA > 0.f) { Pick.pA->Blend_AtTrackPosition(fTrackA, m_pModel->m_Bones, wA / (S + wA)); S += wA; }
	if (wB > 0.f) { Pick.pB->Blend_AtTrackPosition(fTrackB, m_pModel->m_Bones, wB / (S + wB)); S += wB; }
}

_bool CAnimator::Pick_BlendSpace2D(const BLENDSPACE_2D_DESC& desc, _float fParamX, _float fParamY, BS2_PICK& Out)
{
	if (desc.Samples.size() < 3)
		return false;

	_float fX = max(-1.f, min(1.f, fParamX));
	_float fY = max(-1.f, min(1.f, fParamY));

	_float sx = (fX >= 0.f) ? 1.f : -1.f;
	_float sy = (fY >= 0.f) ? 1.f : -1.f;
	_float tx = fabsf(fX);
	_float ty = fabsf(fY);

	auto Find = [&](_float x, _float y) -> const BLENDSPACE_SAMPLE* {
		for (auto& s : desc.Samples)
			if (fabsf(s.fXParamValue - x) < 1e-4f && fabsf(s.fYParamValue - y) < 1e-4f)
				return &s;
		return nullptr;
	};
	const BLENDSPACE_SAMPLE* pIdle = Find(0.f, 0.f);
	const BLENDSPACE_SAMPLE* pX = Find(sx, 0.f);
	const BLENDSPACE_SAMPLE* pY = Find(0.f, sy);
	if (!pIdle || !pX || !pY) return false;

	auto iterI = m_pModel->m_Animations.find(pIdle->strAnimationName);
	auto iterX = m_pModel->m_Animations.find(pX->strAnimationName);
	auto iterY = m_pModel->m_Animations.find(pY->strAnimationName);
	if (iterI == m_pModel->m_Animations.end() || iterX == m_pModel->m_Animations.end() || iterY == m_pModel->m_Animations.end())
		return false;

	_float fSum = tx + ty;
	if (fSum < 1e-5f) { Out.wI = 1.f; Out.wX = 0.f; Out.wY = 0.f; }
	else { Out.wX = (tx * tx) / fSum; Out.wY = (ty * ty) / fSum; Out.wI = 1.f - Out.wX - Out.wY; }

	Out.pI = iterI->second;
	Out.pX = iterX->second;
	Out.pY = iterY->second;
	return true;
}

void CAnimator::Advance_BlendSpacePhase2D(const BLENDSPACE_2D_DESC& desc, _float fParamX, _float fParamY, _float& fPhase, _float fTimeDelta)
{
	BS2_PICK Pick{};
	if (false == Pick_BlendSpace2D(desc, fParamX, fParamY, Pick))
		return;

	auto RealDur = [](CAnimation* pAnim) {
		_float dur = pAnim->Get_Duration(), tick = pAnim->Get_TickPerSecond();
		return (tick > 0.f) ? (dur / tick) : dur;
	};
	_float fBlendedDur = Pick.wI * RealDur(Pick.pI) + Pick.wX * RealDur(Pick.pX) + Pick.wY * RealDur(Pick.pY);
	if (fBlendedDur > 0.f)
		fPhase += (fTimeDelta * desc.fPlayRate) / fBlendedDur;
	if (fPhase >= 1.f)
		fPhase -= 1.f;
}

void CAnimator::Sample_BlendSpace2D(const BLENDSPACE_2D_DESC& desc, _float fParamX, _float fParamY, _float fPhase, _float fLayerWeight)
{
	if (fLayerWeight <= 0.f)
		return;

	BS2_PICK Pick{};
	if (false == Pick_BlendSpace2D(desc, fParamX, fParamY, Pick))
		return;

	_float trackI = fPhase * Pick.pI->Get_Duration();
	_float trackX = fPhase * Pick.pX->Get_Duration();
	_float trackY = fPhase * Pick.pY->Get_Duration();

	_float wIdle = Pick.wI * fLayerWeight;
	_float wX = Pick.wX * fLayerWeight;
	_float wY = Pick.wY * fLayerWeight;
	_float S = 1.f - fLayerWeight;
	if (wIdle > 0.f) { Pick.pI->Blend_AtTrackPosition(trackI, m_pModel->m_Bones, wIdle / (S + wIdle)); S += wIdle; }
	if (wX > 0.f)    { Pick.pX->Blend_AtTrackPosition(trackX, m_pModel->m_Bones, wX / (S + wX));       S += wX; }
	if (wY > 0.f)    { Pick.pY->Blend_AtTrackPosition(trackY, m_pModel->m_Bones, wY / (S + wY));       S += wY; }
}

const ROOT_MOTION_DELTA& CAnimator::Get_RootMotionDelta()
{
	return m_RootMotionDelta;
}

void CAnimator::Begin_MotionWarp(const _float3& vTargetPos, const _float4* pTargetRot,
	_float fWindowEndTrackPos, _bool bTrans, _bool bRot, _bool isGravity)
{
	m_WarpState.isActive           = true;
	m_WarpState.vTargetPos         = vTargetPos;
	m_WarpState.hasTargetRot       = (nullptr != pTargetRot);
	m_WarpState.qTargetRot         = (nullptr != pTargetRot) ? *pTargetRot : _float4{ 0.f, 0.f, 0.f, 1.f };
	m_WarpState.fWindowEndTrackPos = fWindowEndTrackPos;
	m_WarpState.fStartTrackPos = m_fCurPlayTrackPos;
	m_WarpState.vStartPos = _float3{};
	m_WarpState.fPrevTrackPos      = m_fCurPlayTrackPos;
	m_WarpState.isWarpTranslation   = bTrans;
	m_WarpState.isWarpRotation      = bRot;
	m_WarpState.isStartCaptured     = false;
	m_WarpState.isGravity = isGravity;
}

void CAnimator::End_MotionWarp()
{
	m_WarpState = MOTION_WARP_STATE{};
}

void CAnimator::Sync_RootNode(CTransform* pOwnerTransform, _float fTimeDelta)
{
	_matrix matWorld     = pOwnerTransform->Get_WorldMatrix();
	_matrix ResultMatrix = m_RootMatrix * matWorld;
	if (m_WarpState.isActive)
		ResultMatrix = Compute_MotionWarpMatrix(pOwnerTransform, fTimeDelta);
	pOwnerTransform->Set_WorldMatrix(ResultMatrix);
}

void CAnimator::Compute_RootAnimation(_float fRootMotionRate, _bool isRootMotionRotation, _bool isRootMotionTranslate, _bool IsRootMotionEnable)
{
	m_fCurRootMotionRate = fRootMotionRate;

	_vector vScale{}, vRotation{}, vTranslation{};
	XMMatrixDecompose(&vScale, &vRotation, &vTranslation, XMLoadFloat4x4(m_pModel->m_Bones[m_pModel->m_iRootBoneIndex]->Get_TransformationMatrix()));

	_matrix RootBoneLocalMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_pModel->m_Bones[m_pModel->m_iRootBoneIndex]->Set_TransformationMatrix(RootBoneLocalMatrix);

	_matrix matConversion = XMLoadFloat4x4(&m_pModel->m_ConversionMatrix);
	_vector qConversion = XMQuaternionRotationMatrix(matConversion);

	_vector vConvertedTranslation = XMVector3Transform(vTranslation, matConversion);
	_vector vConvertedRotation = XMQuaternionMultiply(qConversion, vRotation);

	_vector vLocalTranslate = vConvertedTranslation - XMLoadFloat4(&m_vPreRootPosition);
	_vector vRotationDelta = XMQuaternionMultiply(vConvertedRotation, XMQuaternionInverse(XMLoadFloat4(&m_vPreRootRotation)));

	if (!isRootMotionRotation)
		vRotationDelta = XMQuaternionIdentity();

	if (!isRootMotionTranslate)
		vLocalTranslate = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	if (true == m_isChangeAnimation)
	{
		m_isChangeAnimation = false;
		vLocalTranslate = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		vRotationDelta = XMQuaternionIdentity();
	}

	if (!IsRootMotionEnable)
	{
		XMStoreFloat3(&m_RootMotionDelta.vTranslate, vLocalTranslate * m_pModel->m_fPreScale);
		XMStoreFloat4(&m_RootMotionDelta.qRotation, vRotationDelta);
	}

	if (IsRootMotionEnable)
	{
		m_RootMatrix = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 1.f),
			XMVectorSet(0.f, 0.f, 0.f, 1.f),
			vRotationDelta,
			vLocalTranslate * m_pModel->m_fPreScale * fRootMotionRate
		);
	}
	else
	{
		m_RootMatrix = XMMatrixIdentity();
	}

	XMStoreFloat4(&m_vPreRootPosition, vConvertedTranslation);
	XMStoreFloat4(&m_vPreRootRotation, vConvertedRotation);
}

void CAnimator::HandleAnimationChange(const _string& strAnimationName)
{
	if (m_strPreAnimation == strAnimationName)
		return;

	m_isChangeAnimation = true;
	m_strPreAnimation = strAnimationName;
	Clear_Animation(strAnimationName);
}

_bool CAnimator::Update_TrackPosition(CAnimation* pAnimation, _float* pTrackPosition, _float fTimeDelta)
{
	_float fTrackPosition = 0.f;

	_bool isAnimationEnd = pAnimation->Update_TrackPosition(fTimeDelta, &fTrackPosition);
	*pTrackPosition = fTrackPosition;

	return isAnimationEnd;
}

_matrix CAnimator::Compute_MotionWarpMatrix(CTransform* pOwnerTransform, _float fTimeDelta)
{
	static constexpr _float WARP_SCALE_MAX = 3.f;

	_matrix ResultMatrix{};

	_matrix matWorld = pOwnerTransform->Get_WorldMatrix();

	_vector vWorldScale;
	_vector vWorldQuat;
	_vector vWorldTrans;
	XMMatrixDecompose(&vWorldScale, &vWorldQuat, &vWorldTrans, matWorld);

	if (!m_WarpState.isStartCaptured)
	{
		XMStoreFloat3(&m_WarpState.vStartPos, vWorldTrans);
		m_WarpState.fStartTrackPos = m_fCurPlayTrackPos;
		m_WarpState.isStartCaptured = true;
	}

	_float fStartTrack = m_fCurPlayTrackPos;
	_float fEndTrack = m_WarpState.fWindowEndTrackPos;

	const ROOT_MOTION_DELTA& AnimRootDelta = m_pModel->Extract_RootMotion(m_strCurPlayKey, fStartTrack, fEndTrack);
	_vector vAnimLocalDelta = XMVectorSetW(XMLoadFloat3(&AnimRootDelta.vTranslate), 1.f);
	_vector vAnimEndWorldDelta = XMVector3TransformNormal(XMLoadFloat3(&AnimRootDelta.vTranslate), matWorld);
	_vector vAnimEndWorldTrans = vWorldTrans + vAnimEndWorldDelta;

	_vector vAnimEndLocalRot = XMLoadFloat4(&AnimRootDelta.qRotation);
	_vector vAnimEndWorldRot = XMQuaternionMultiply(vAnimEndLocalRot, vWorldQuat);

	_vector vTarget = XMVectorSetW(XMLoadFloat3(&m_WarpState.vTargetPos), 1.f);
	_vector vStart = vWorldTrans;

	_float fAnimDist = XMVectorGetX(XMVector3Length(vAnimEndWorldTrans - vStart));
	_float fScale = 1.f;

	_vector vPredictedEnd = vAnimEndWorldTrans;
	_vector vError = vTarget - vPredictedEnd;

	if (fAnimDist > 0.0001f)
	{
		fScale = XMVectorGetX(XMVector3Length(vTarget - vStart)) / fAnimDist;
	}

	fScale = std::clamp(fScale, 0.f, WARP_SCALE_MAX);

	_vector vRootScale;
	_vector vRootQuat;
	_vector vRootTrans;
	XMMatrixDecompose(&vRootScale, &vRootQuat, &vRootTrans, m_RootMatrix);

	_vector vRootWorldDelta = XMVector3TransformNormal(vRootTrans, matWorld);
	_vector vRootWorldTrans = vWorldTrans + vRootWorldDelta;
	_vector vRootWorldQuat = XMQuaternionMultiply(vRootQuat, vWorldQuat);

	_float fDeltaTrack = m_fCurPlayTrackPos - m_WarpState.fPrevTrackPos;
	_float fRemainingTrack = fEndTrack - m_WarpState.fPrevTrackPos;
	m_WarpState.fPrevTrackPos = m_fCurPlayTrackPos;

	_vector vCorrection = XMVectorZero();
	if (fRemainingTrack > MW_EPS && fDeltaTrack > 0.f)
		vCorrection = vError * (fDeltaTrack / fRemainingTrack);

	_vector vWarpedPos = vStart + (vRootWorldDelta)+vCorrection;

	_vector vResultQuat = vRootWorldQuat;

	if (m_WarpState.hasTargetRot)
	{

	}

	if (m_fCurPlayTrackPos >= m_WarpState.fWindowEndTrackPos)
	{
		End_MotionWarp();
	}

	ResultMatrix = XMMatrixAffineTransformation(
		XMVectorSet(1.f, 1.f, 1.f, 0.f),
		XMVectorSet(0.f, 0.f, 0.f, 1.f),
		vResultQuat,
		vWarpedPos
	);

	return ResultMatrix;
}

_bool CAnimator::Play_Animation_GPU(CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	CAnimation* pAnimation = m_pModel->Get_AnimationOrNull(playDesc.strAnimationName);
	if (nullptr == pComputeShaderCom || nullptr == playDesc.pTrackPosition || nullptr == pAnimation)
		return false;

	HandleAnimationChange(playDesc.strAnimationName);

	_bool isAnimationEnd = Update_TrackPosition(pAnimation, playDesc.pTrackPosition, playDesc.fSpeed * fTimeDelta);
	FetchLocalMatrices_FromCompute(pComputeShaderCom, *playDesc.pTrackPosition, playDesc.strAnimationName);

	if (true == rootMotionDesc.isEnable)
		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();

	for (_uint i = 0; i < m_pModel->m_Bones.size(); i++)
		m_pModel->m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);

	if (isAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		return true;
	}

	return false;
}

_bool CAnimator::Play_Animation_GPU(CComputeShader* pComputeShaderCom, CComputeShader* pMorphComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	CAnimation* pAnimation = m_pModel->Get_AnimationOrNull(playDesc.strAnimationName);
	if (nullptr == pComputeShaderCom || nullptr == pMorphComputeShaderCom ||
		nullptr == playDesc.pTrackPosition || nullptr == pAnimation)
		return false;

	HandleAnimationChange(playDesc.strAnimationName);
	_bool isAnimationEnd = Update_TrackPosition(pAnimation, playDesc.pTrackPosition, playDesc.fSpeed * fTimeDelta);

	FetchLocalMatrices_FromCompute(pComputeShaderCom, *playDesc.pTrackPosition, playDesc.strAnimationName);

	Update_MorphAnimation(pAnimation, pMorphComputeShaderCom, playDesc.fSpeed * fTimeDelta, playDesc.isFacial);

	if (true == rootMotionDesc.isEnable)
		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();

	for (_uint i = 0; i < m_pModel->m_Bones.size(); i++)
		m_pModel->m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);

	if (isAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		return true;
	}

	return false;
}

_bool CAnimator::Play_NonRibAnimation_GPU(CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	CAnimation* pAnimation = m_pModel->Get_AnimationOrNull(playDesc.strAnimationName);
	if (nullptr == pComputeShaderCom || nullptr == playDesc.pTrackPosition || nullptr == pAnimation)
		return false;

	HandleAnimationChange(playDesc.strAnimationName);
	_bool isAnimationEnd = Update_TrackPosition(pAnimation, playDesc.pTrackPosition, playDesc.fSpeed * fTimeDelta);

	FetchLocalMatrices_FromComputeNonRib(pComputeShaderCom, *playDesc.pTrackPosition, playDesc.strAnimationName);

	if (true == rootMotionDesc.isEnable)
		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();

	for (_uint i = 0; i < m_pModel->m_Bones.size(); i++)
		m_pModel->m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);

	if (isAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		return true;
	}

	return false;
}

_bool CAnimator::Play_FlyAnimation_GPU(CComputeShader* pComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, const GPU_BLEND_INFO& gpuBlendInfo, _float fTimeDelta)
{
	CAnimation* pAnimation = m_pModel->Get_AnimationOrNull(playDesc.strAnimationName);
	if (nullptr == pComputeShaderCom || nullptr == playDesc.pTrackPosition || nullptr == pAnimation)
		return false;

	HandleAnimationChange(playDesc.strAnimationName);
	_bool isAnimationEnd = Update_TrackPosition(pAnimation, playDesc.pTrackPosition, playDesc.fSpeed * fTimeDelta);

	FetchLocalMatrices_FromComputeFly(pComputeShaderCom, *playDesc.pTrackPosition, playDesc.strAnimationName, gpuBlendInfo);

	if (true == rootMotionDesc.isEnable)
		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();

	for (_uint i = 0; i < m_pModel->m_Bones.size(); i++)
		m_pModel->m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);

	if (isAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		return true;
	}

	return false;
}

_bool CAnimator::Play_FlyAnimation_GPU(CComputeShader* pComputeShaderCom, CComputeShader* pMorphComputeShaderCom, const ANIMATION_PLAY_DESC& playDesc, const ROOTMOTION_DESC& rootMotionDesc, const GPU_BLEND_INFO& gpuBlendInfo, _float fTimeDelta)
{
	CAnimation* pAnimation = m_pModel->Get_AnimationOrNull(playDesc.strAnimationName);
	if (nullptr == pComputeShaderCom || nullptr == pMorphComputeShaderCom ||
		nullptr == playDesc.pTrackPosition || nullptr == pAnimation)
		return false;

	HandleAnimationChange(playDesc.strAnimationName);
	_bool isAnimationEnd = Update_TrackPosition(pAnimation, playDesc.pTrackPosition, playDesc.fSpeed * fTimeDelta);

	FetchLocalMatrices_FromComputeFly(pComputeShaderCom, *playDesc.pTrackPosition, playDesc.strAnimationName, gpuBlendInfo);

	Update_MorphAnimation(pAnimation, pMorphComputeShaderCom, playDesc.fSpeed * fTimeDelta, playDesc.isFacial);

	if (true == rootMotionDesc.isEnable)
		Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate);
	else
		m_RootMatrix = XMMatrixIdentity();

	for (_uint i = 0; i < m_pModel->m_Bones.size(); i++)
		m_pModel->m_Bones[i]->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);

	if (isAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		return true;
	}

	return false;
}

void CAnimator::FetchLocalMatrices_FromCompute(CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName)
{
	if (nullptr == pComputeShaderCom)
		return;

	Update_AnimConstantBuffer(strAnimationName, fTrackPosition);
	Bind_AnimationResource(pComputeShaderCom);

	_uint iNumBones = static_cast<_uint>(m_pModel->m_Bones.size());
	_uint iGroupCount = (iNumBones + (pComputeShaderCom->Get_ThreadInfo().iThreadGroupX - 1)) /
		pComputeShaderCom->Get_ThreadInfo().iThreadGroupX;
	pComputeShaderCom->Dispatch(iGroupCount, 1, 1);

	Readback_BoneMatrices();
}

void CAnimator::FetchLocalMatrices_FromComputeFly(CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName, const GPU_BLEND_INFO& gpuBlendInfo)
{
	ASSERT_CRASH(pComputeShaderCom);
	Update_FlyAnimConstantBuffer(gpuBlendInfo, strAnimationName, fTrackPosition);
	Bind_FlyAnimationResource(pComputeShaderCom);
	_uint iNumBones = static_cast<_uint>(m_pModel->m_Bones.size());
	_uint iGroupCount = (iNumBones + (pComputeShaderCom->Get_ThreadInfo().iThreadGroupX - 1)) / pComputeShaderCom->Get_ThreadInfo().iThreadGroupX;
	pComputeShaderCom->Dispatch(iGroupCount, 1, 1);
	Readback_BoneMatrices();
}

void CAnimator::FetchLocalMatrices_FromComputeNonRib(CComputeShader* pComputeShaderCom, _float fTrackPosition, const _string& strAnimationName)
{
	ASSERT_CRASH(pComputeShaderCom);

	Update_NonRibAnimConstantBuffer(strAnimationName, fTrackPosition);
	Bind_AnimationResource(pComputeShaderCom);

	_uint iNumBones = static_cast<_uint>(m_pModel->m_Bones.size());
	_uint iGroupCount = (iNumBones + (pComputeShaderCom->Get_ThreadInfo().iThreadGroupX - 1)) / pComputeShaderCom->Get_ThreadInfo().iThreadGroupX;
	pComputeShaderCom->Dispatch(iGroupCount, 1, 1);

	Readback_BoneMatrices();
}

void CAnimator::Update_NonRibAnimConstantBuffer(const _string& strAnimationName, _float fTrackPosition)
{
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	m_pModel->m_pContext->Map(m_pModel->m_Buffers[CModel::BUFFER_ANIM_INFOCB], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);

	ANIMATION_CBINFO* pAnimCBInfo = static_cast<ANIMATION_CBINFO*>(MappedSubResource.pData);
	pAnimCBInfo->fTrackPosition = fTrackPosition;
	pAnimCBInfo->iAnimindex = m_pModel->m_AnimationNameToIndex[strAnimationName];
	pAnimCBInfo->iRibAnimUsed = 0;
	pAnimCBInfo->iRibbonAnimIndex = 0;

	m_pModel->m_pContext->Unmap(m_pModel->m_Buffers[CModel::BUFFER_ANIM_INFOCB], 0);
}

void CAnimator::Update_AnimConstantBuffer(const _string& strAnimationName, _float fTrackPosition)
{
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	m_pModel->m_pContext->Map(m_pModel->m_Buffers[CModel::BUFFER_ANIM_INFOCB], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);

	_bool IsRibAnimUsed = false;
	ANIMATION_CBINFO* pAnimCBInfo = static_cast<ANIMATION_CBINFO*>(MappedSubResource.pData);
	pAnimCBInfo->fTrackPosition = fTrackPosition;
	pAnimCBInfo->iAnimindex = m_pModel->m_AnimationNameToIndex[strAnimationName];
	pAnimCBInfo->iRibAnimUsed = 0;
	pAnimCBInfo->iRibbonAnimIndex = 0;

	_string strRibAnimationName = CModel::kRibPrefix + strAnimationName;
	auto iter = m_pModel->m_Animations.find(strRibAnimationName);
	if (iter == m_pModel->m_Animations.end())
	{
		pAnimCBInfo->iRibAnimUsed = 0;
	}
	else
	{
		pAnimCBInfo->iRibAnimUsed = 1;
		pAnimCBInfo->iRibbonAnimIndex = m_pModel->m_AnimationNameToIndex[strRibAnimationName];
	}

	m_pModel->m_pContext->Unmap(m_pModel->m_Buffers[CModel::BUFFER_ANIM_INFOCB], 0);
}

void CAnimator::Update_FlyAnimConstantBuffer(const GPU_BLEND_INFO& gpuBlendInfo, const _string& strAnimationName, _float fTrackPosition)
{
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	m_pModel->m_pContext->Map(m_pModel->m_Buffers[CModel::BUFFER_ANIM_INFOFLYCB], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);

	_bool IsRibAnimUsed = false;
	ANIMATIONFLY_CBINFO* pAnimCBInfo = static_cast<ANIMATIONFLY_CBINFO*>(MappedSubResource.pData);
	pAnimCBInfo->fTrackPosition = fTrackPosition;
	pAnimCBInfo->iAnimindex = m_pModel->m_AnimationNameToIndex[strAnimationName];
	pAnimCBInfo->iRibAnimUsed = 0;
	pAnimCBInfo->iRibbonAnimIndex = 0;

	_string strRibAnimationName = CModel::kRibPrefix + strAnimationName;
	auto iter = m_pModel->m_Animations.find(strRibAnimationName);
	if (iter == m_pModel->m_Animations.end())
	{
		pAnimCBInfo->iRibAnimUsed = 0;
	}
	else
	{
		pAnimCBInfo->iRibAnimUsed = 1;
		pAnimCBInfo->iRibbonAnimIndex = m_pModel->m_AnimationNameToIndex[strRibAnimationName];
	}

	if (gpuBlendInfo.IsBlendEnabled)
	{
		pAnimCBInfo->IsBlendEnabled = gpuBlendInfo.IsBlendEnabled;
		pAnimCBInfo->fBlendParamLR = gpuBlendInfo.fBlendParamLR;
		pAnimCBInfo->fBlendParamDU = gpuBlendInfo.fBlendParamDU;
		pAnimCBInfo->fPadding = 0.f;

		pAnimCBInfo->iClipIndexL = m_pModel->GetSafeIndex(gpuBlendInfo.strClipxL);
		pAnimCBInfo->iClipIndexMidLR = m_pModel->GetSafeIndex(gpuBlendInfo.strClipMidLR);
		pAnimCBInfo->iClipIndexR = m_pModel->GetSafeIndex(gpuBlendInfo.strClipxR);
		pAnimCBInfo->iWeightClipLR = m_pModel->GetSafeIndex(gpuBlendInfo.strWeightClipLR);

		pAnimCBInfo->iClipIndexD = m_pModel->GetSafeIndex(gpuBlendInfo.strClipxD);
		pAnimCBInfo->iClipIndexMidDU = m_pModel->GetSafeIndex(gpuBlendInfo.strClipMidDU);
		pAnimCBInfo->iClipIndexU = m_pModel->GetSafeIndex(gpuBlendInfo.strClipxU);
		pAnimCBInfo->iWeightClipDU = m_pModel->GetSafeIndex(gpuBlendInfo.strWeightClipDU);
	}
	else
	{
		pAnimCBInfo->fBlendParamLR = 0.f;
		pAnimCBInfo->fBlendParamDU = 0.f;

		pAnimCBInfo->iClipIndexL = 0;
		pAnimCBInfo->iClipIndexMidLR = 0;
		pAnimCBInfo->iClipIndexR = 0;
		pAnimCBInfo->iWeightClipLR = 0;

		pAnimCBInfo->iClipIndexD = 0;
		pAnimCBInfo->iClipIndexMidDU = 0;
		pAnimCBInfo->iClipIndexU = 0;
		pAnimCBInfo->iWeightClipDU = 0;
	}

	m_pModel->m_pContext->Unmap(m_pModel->m_Buffers[CModel::BUFFER_ANIM_INFOFLYCB], 0);
}

void CAnimator::Bind_AnimationResource(CComputeShader* pComputeShaderCom)
{
	pComputeShaderCom->Set_SRV("g_AllKeyframes", m_pModel->m_SRVs[CModel::SRV_KEY_FRAME]);
	pComputeShaderCom->Set_SRV("g_AllAnimInfos", m_pModel->m_SRVs[CModel::SRV_ANIM_INFO]);
	pComputeShaderCom->Set_SRV("g_ChannelInfos", m_pModel->m_SRVs[CModel::SRV_BONE_CHANNEL]);
	pComputeShaderCom->Set_UAV("g_OutLocalMatrices", m_pModel->m_UAVs[CModel::UAV_FINAL_BONEMATRIX]);
	pComputeShaderCom->Set_ConstantBuffer("AnimationInfoCB", m_pModel->m_Buffers[CModel::BUFFER_ANIM_INFOCB]);
}

void CAnimator::Bind_FlyAnimationResource(CComputeShader* pComputeShaderCom)
{
	pComputeShaderCom->Set_SRV("g_AllKeyframes", m_pModel->m_SRVs[CModel::SRV_KEY_FRAME]);
	pComputeShaderCom->Set_SRV("g_AllAnimInfos", m_pModel->m_SRVs[CModel::SRV_ANIM_INFO]);
	pComputeShaderCom->Set_SRV("g_ChannelInfos", m_pModel->m_SRVs[CModel::SRV_BONE_CHANNEL]);
	pComputeShaderCom->Set_UAV("g_OutLocalMatrices", m_pModel->m_UAVs[CModel::UAV_FINAL_BONEMATRIX]);
	pComputeShaderCom->Set_ConstantBuffer("AnimationInfoCB", m_pModel->m_Buffers[CModel::BUFFER_ANIM_INFOFLYCB]);
}

void CAnimator::Readback_BoneMatrices()
{
	_uint iNumBones = static_cast<_uint>(m_pModel->m_Bones.size());
	_uint iReadIdx = CModel::BUFFER_STAGING_0;

	m_pModel->m_pContext->CopyResource(m_pModel->m_Buffers[iReadIdx], m_pModel->m_Buffers[CModel::BUFFER_FINAL_BONEMATRIX]);

	D3D11_MAPPED_SUBRESOURCE Mapped;
	LARGE_INTEGER t0{}, t1{};
	const HRESULT hr = m_pModel->m_pContext->Map(m_pModel->m_Buffers[iReadIdx], 0, D3D11_MAP_READ, 0, &Mapped);

	if (SUCCEEDED(hr))
	{
		memcpy(m_pModel->m_vLocalMatrices.data(), Mapped.pData, sizeof(_float4x4) * iNumBones);
		m_pModel->m_pContext->Unmap(m_pModel->m_Buffers[iReadIdx], 0);

		for (size_t i = 0; i < iNumBones; ++i)
			m_pModel->m_Bones[i]->Set_TransformationMatrix(XMLoadFloat4x4(&m_pModel->m_vLocalMatrices[i]));
	}

	m_pModel->m_iCurStagingFlip = (m_pModel->m_iCurStagingFlip + 1) % 2;
}

void CAnimator::Update_MorphAnimation(CAnimation* pAnimation, CComputeShader* pMorphComputeShaderCom, _float fTimeDelta, _bool isFacial)
{
	if (m_pModel->m_eType != MODELTYPE::CHARACTER || !isFacial)
		return;
	if (nullptr == m_pModel->m_Buffers[CModel::BUFFER_MORPH_WEIGHT] || nullptr == m_pModel->m_SRVs[CModel::SRV_MORPH_WEIGHT])
		return;

	pAnimation->Update_MorphWeights(fTimeDelta, m_pModel->m_ShapeKeyWeights);

	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	if (SUCCEEDED(m_pModel->m_pContext->Map(m_pModel->m_Buffers[CModel::BUFFER_MORPH_WEIGHT], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource)))
	{
		memcpy(MappedSubResource.pData, m_pModel->m_ShapeKeyWeights.data(), sizeof(_float) * m_pModel->m_ShapeKeyWeights.size());
		m_pModel->m_pContext->Unmap(m_pModel->m_Buffers[CModel::BUFFER_MORPH_WEIGHT], 0);
	}

	for (auto& pMesh : m_pModel->m_Meshes)
	{
		if (false == pMesh->HasMorphTargets())
			continue;

		pMesh->Compute_Morph(pMorphComputeShaderCom, m_pModel->m_SRVs[CModel::SRV_MORPH_WEIGHT]);
	}
}

CAnimator* CAnimator::Create(CModel* pModel)
{
	CAnimator* pInstance = new CAnimator();
	if (FAILED(pInstance->Initialize(pModel)))
	{
		MSG_BOX("Failed to Create : CAnimator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CAnimator::Free()
{
	__super::Free();

	Safe_Release(m_pModel);
}

NS_END
