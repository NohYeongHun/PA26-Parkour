#include "EnginePch.h"
#include "Animator.h"
#include "Model.h"
#include "Transform.h"
#include "Bone.h"
#include "Animation.h"

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

	Handle_PlaybackChange(ETransitionSourceType::CLIP, playDesc.strAnimationName, playDesc.fBlendIn);
	m_fCurBlendOut = playDesc.fBlendOut;
	m_fCurPlaySpeed = playDesc.fSpeed;
	m_pCurBS1 = nullptr;
	m_pCurBS2 = nullptr;

	_float fTrackPosition = {};
	_bool IsAnimationEnd = false;

	_float fWeight = Update_TransitionSource(fTimeDelta);
	if (fWeight < 1.f)
	{
		IsAnimationEnd = iter->second->Update_TrackPosition(playDesc.fSpeed * fTimeDelta, &fTrackPosition);
		iter->second->Blend_AtTrackPosition(fTrackPosition, m_pModel->m_Bones, fWeight);
		iter->second->Sample_BoneAtTrackPosition(fTrackPosition, m_pModel->m_Bones, m_pModel->m_iRootBoneIndex);
	}
	else
	{
		IsAnimationEnd = iter->second->Update_TransformationMatrices_All(fTimeDelta * playDesc.fSpeed, m_pModel->m_Bones, &fTrackPosition);
	}

	if (nullptr != playDesc.pTrackPosition)
		*playDesc.pTrackPosition = fTrackPosition;
	m_fCurPlayTrackPos = fTrackPosition;

	Compute_RootAnimation(rootMotionDesc.fRate, rootMotionDesc.isRotate, rootMotionDesc.isTranslate, rootMotionDesc.isEnable);

	if (IsAnimationEnd)
	{
		Clear_Animation(playDesc.strAnimationName);
		m_TransitionSource.eType = ETransitionSourceType::NONE;
		return true;
	}

	for (auto& pBone : m_pModel->m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);

	return false;
}

_bool CAnimator::Play_BlendSpace_CPU(const BLENDSPACE_1D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	if (nullptr == desc.pParam || desc.Samples.size() < 2)
		return false;

	Handle_PlaybackChange(ETransitionSourceType::BLENDSPACE_1D, desc.Samples[0].strAnimationName, desc.fBlendIn);
	m_fCurBlendOut = desc.fBlendOut;
	m_fCurPlaySpeed = desc.fPlayRate;
	m_pCurBS1 = &desc;
	m_pCurBS2 = nullptr;

	_float fWeight = Update_TransitionSource(fTimeDelta);
	Layer_BlendSpace1D(desc, *desc.pParam, m_fBlendSpacePhase, fWeight, fTimeDelta);

	Compute_RootAnimation(rootMotionDesc.fRate,
		rootMotionDesc.isEnable && rootMotionDesc.isRotate,
		rootMotionDesc.isEnable && rootMotionDesc.isTranslate);

	for (auto& pBone : m_pModel->m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);

	return false;
}

_bool CAnimator::Play_BlendSpace2D_CPU(const BLENDSPACE_2D_DESC& desc, const ROOTMOTION_DESC& rootMotionDesc, _float fTimeDelta)
{
	if (nullptr == desc.pParam || desc.Samples.size() < 3)
		return false;

	Handle_PlaybackChange(ETransitionSourceType::BLENDSPACE_2D, desc.Samples[0].strAnimationName, desc.fBlendIn);
	m_fCurBlendOut = desc.fBlendOut;
	m_fCurPlaySpeed = desc.fPlayRate;
	m_pCurBS1 = nullptr;
	m_pCurBS2 = &desc;

	_float fWeight = Update_TransitionSource(fTimeDelta);
	Layer_BlendSpace2D(desc, desc.pParam->x, desc.pParam->y, m_fBlendSpacePhase, fWeight, fTimeDelta);

	Compute_RootAnimation(rootMotionDesc.fRate,
		rootMotionDesc.isEnable && rootMotionDesc.isRotate,
		rootMotionDesc.isEnable && rootMotionDesc.isTranslate);

	for (auto& pBone : m_pModel->m_Bones)
		pBone->Update_CombinedTransformationMatrix(XMLoadFloat4x4(&m_pModel->m_PreTransformMatrix), m_pModel->m_Bones);

	return false;
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
		m_fBlendOverride = -1.f; // consumed once.
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

_float CAnimator::Update_TransitionSource(_float fTimeDelta)
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
		iter->second->Sample_AtTrackPosition(m_TransitionSource.fTrackPos, m_pModel->m_Bones);
		m_TransitionSource.fTrackPos += iter->second->Get_TickPerSecond() * m_TransitionSource.fSpeed * fTimeDelta;
		break;
	}
	case ETransitionSourceType::BLENDSPACE_1D:
		Layer_BlendSpace1D(m_TransitionSource.BS1, m_TransitionSource.fFrozenParamX,
			m_TransitionSource.fPhase, 1.f, fTimeDelta);
		break;
	case ETransitionSourceType::BLENDSPACE_2D:
		Layer_BlendSpace2D(m_TransitionSource.BS2, m_TransitionSource.fFrozenParamX,
			m_TransitionSource.fFrozenParamY, m_TransitionSource.fPhase, 1.f, fTimeDelta);
		break;
	case ETransitionSourceType::FROZEN:
		for (size_t i = 0; i < m_pModel->m_Bones.size(); ++i)
			m_pModel->m_Bones[i]->Set_TransformationMatrix(XMLoadFloat4x4(&m_TransitionSource.FrozenPose[i]));
		break;
	}

	return fWeight;
}

void CAnimator::Layer_BlendSpace1D(const BLENDSPACE_1D_DESC& desc, _float fParam, _float& fPhase, _float fLayerWeight, _float fTimeDelta)
{
	if (desc.Samples.size() < 2 || fLayerWeight <= 0.f)
		return;

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
		return;

	_float fRange = desc.Samples[iB].fXParamValue - desc.Samples[iA].fXParamValue;
	_float fU = (fRange > 0.f) ? ((fParam - desc.Samples[iA].fXParamValue) / fRange) : 0.f;
	fU = max(0.f, min(1.f, fU));

	_float fDurA = iterA->second->Get_Duration();
	_float fDurB = iterB->second->Get_Duration();
	_float fTickA = iterA->second->Get_TickPerSecond();
	_float fTickB = iterB->second->Get_TickPerSecond();
	_float fRealDurA = (fTickA > 0.f) ? (fDurA / fTickA) : fDurA;
	_float fRealDurB = (fTickB > 0.f) ? (fDurB / fTickB) : fDurB;
	_float fBlendedDur = fRealDurA + fU * (fRealDurB - fRealDurA);
	if (fBlendedDur > 0.f)
		fPhase += (fTimeDelta * desc.fPlayRate) / fBlendedDur;
	if (fPhase >= 1.f)
		fPhase -= 1.f;

	_float fTrackA = fPhase * fDurA;
	_float fTrackB = fPhase * fDurB;

	_float wA = fLayerWeight * (1.f - fU);
	_float wB = fLayerWeight * fU;
	_float S = 1.f - fLayerWeight;
	if (wA > 0.f) { iterA->second->Blend_AtTrackPosition(fTrackA, m_pModel->m_Bones, wA / (S + wA)); S += wA; }
	if (wB > 0.f) { iterB->second->Blend_AtTrackPosition(fTrackB, m_pModel->m_Bones, wB / (S + wB)); S += wB; }
}

void CAnimator::Layer_BlendSpace2D(const BLENDSPACE_2D_DESC& desc, _float fParamX, _float fParamY, _float& fPhase, _float fLayerWeight, _float fTimeDelta)
{
	if (desc.Samples.size() < 3 || fLayerWeight <= 0.f)
		return;

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
	if (!pIdle || !pX || !pY) return;

	auto iterI = m_pModel->m_Animations.find(pIdle->strAnimationName);
	auto iterX = m_pModel->m_Animations.find(pX->strAnimationName);
	auto iterY = m_pModel->m_Animations.find(pY->strAnimationName);
	if (iterI == m_pModel->m_Animations.end() || iterX == m_pModel->m_Animations.end() || iterY == m_pModel->m_Animations.end())
		return;

	_float fSum = tx + ty;
	_float wX, wY, wIdle;
	if (fSum < 1e-5f) { wIdle = 1.f; wX = 0.f; wY = 0.f; }
	else { wX = (tx * tx) / fSum; wY = (ty * ty) / fSum; wIdle = 1.f - wX - wY; }

	auto RealDur = [](CAnimation* pAnim) {
		_float dur = pAnim->Get_Duration(), tick = pAnim->Get_TickPerSecond();
		return (tick > 0.f) ? (dur / tick) : dur;
	};
	_float fBlendedDur = wIdle * RealDur(iterI->second) + wX * RealDur(iterX->second) + wY * RealDur(iterY->second);
	if (fBlendedDur > 0.f)
		fPhase += (fTimeDelta * desc.fPlayRate) / fBlendedDur;
	if (fPhase >= 1.f)
		fPhase -= 1.f;

	_float trackI = fPhase * iterI->second->Get_Duration();
	_float trackX = fPhase * iterX->second->Get_Duration();
	_float trackY = fPhase * iterY->second->Get_Duration();

	wIdle *= fLayerWeight;
	wX *= fLayerWeight;
	wY *= fLayerWeight;
	_float S = 1.f - fLayerWeight;
	if (wIdle > 0.f) { iterI->second->Blend_AtTrackPosition(trackI, m_pModel->m_Bones, wIdle / (S + wIdle)); S += wIdle; }
	if (wX > 0.f)    { iterX->second->Blend_AtTrackPosition(trackX, m_pModel->m_Bones, wX / (S + wX));       S += wX; }
	if (wY > 0.f)    { iterY->second->Blend_AtTrackPosition(trackY, m_pModel->m_Bones, wY / (S + wY));       S += wY; }
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

	// Store converted T/R for the next frame.
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
