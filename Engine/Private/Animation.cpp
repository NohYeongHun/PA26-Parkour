#include "EnginePch.h"
#include "Animation.h"

#include "Channel.h"
#include "MorphChannel.h"

#include "SoundNotify.h"
#include "ColliderNotify.h"
#include "EffectNotify.h"
#include "ObjectFuncNotify.h"
#include "StateFlagNotifyState.h"
#include "MotionWarpNotifyState.h"
#include "IKNotifyState.h"

CAnimation::CAnimation()
{
}

CAnimation::CAnimation(const CAnimation& Prototype)
	: m_fDuration { Prototype.m_fDuration },
	m_fTickPerSecond { Prototype.m_fTickPerSecond },
	m_fCurrentTrackPosition { Prototype.m_fCurrentTrackPosition },
	m_iNumChannels { Prototype.m_iNumChannels },
	m_Channels { Prototype.m_Channels },
	m_CurrentFrameIndices{ Prototype.m_CurrentFrameIndices },
	m_CurrentMorphCurveIndicies{ Prototype.m_CurrentMorphCurveIndicies },
	m_iNumMorphCurves{ Prototype.m_iNumMorphCurves },
	m_MorphKeyIndicies { Prototype.m_MorphKeyIndicies },
	m_MorphMeshChannels { Prototype.m_MorphMeshChannels } 
	
{
	strcpy_s(m_szName, Prototype.m_szName);

	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);

	
	for (auto& pMorphChannel : m_MorphMeshChannels)
		Safe_AddRef(pMorphChannel);
}

void CAnimation::Set_CurrentTrackPosition(_float fTrackPos)
{
	m_fCurrentTrackPosition = fTrackPos;
	m_iNotifyIndex = 0;
	while(m_iNotifyIndex < m_AnimNotifies.size() && m_fCurrentTrackPosition > m_AnimNotifies[m_iNotifyIndex]->Get_TrackPosition())
		m_iNotifyIndex++;

	Force_EndNotifyStates();
}

void CAnimation::Release_Channels()
{
	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);
	m_Channels.clear();
}



#ifdef _DEBUG
void CAnimation::Print_MorphKeyIndices()
{

	_wstring strIndices = { };

	for (size_t i = 0; i < m_MorphKeyIndicies.size(); ++i)
	{
		strIndices += to_wstring(m_MorphKeyIndicies[i]);
		strIndices += _wstring(L"\n");
	}

	OutputDebugString(strIndices.c_str());
}
void CAnimation::Clear_AnimNotifies()
{
	for (auto& pNotify : m_AnimNotifies)
		Safe_Release(pNotify);
	m_AnimNotifies.clear();

	for (auto& pNotifyState : m_AnimNotifyStates)
		Safe_Release(pNotifyState);
	m_AnimNotifyStates.clear();

	m_iNotifyIndex = 0;
}
#endif // _DEBUG



void CAnimation::Register_Notify(const NOTIFY& AnimNotify)
{
	m_Notifies.push_back(AnimNotify);
}

void CAnimation::Load_Notify(const json& notifyJson
	, function<void(const _wstring&, _bool)> ColliderCallback
	, function<void(const _wstring&)> EffectCallback
	, function<void(const _wstring&)> ObjectCallback
	, function<void(const _string&, _bool)> StateFlagCallback
	, function<void(const _string&, _bool, _float, _bool, _bool)> WarpCallback
	, function<void(const vector<IK_BINDING>& Bindings, _float fBlendSec, _bool isBegin)> IKCallback)
{
	for (const auto& notifyObject : notifyJson)
	{
		string type = notifyObject["NotifyType"].get<string>();

		if (type == "StateFlag")
		{
			CStateFlagNotifyState* pState = CStateFlagNotifyState::From_Json(notifyObject);
			pState->Set_StateFlagCallback(StateFlagCallback);
			m_AnimNotifyStates.emplace_back(pState);
			continue;
		}
		if (type == "MotionWarp")
		{
			json warpJson = notifyObject;
			if (warpJson.contains("EndTrackPosition") && warpJson["EndTrackPosition"].get<_float>() < 0.f)
				warpJson["EndTrackPosition"] = m_fDuration;

			CMotionWarpNotifyState* pState = CMotionWarpNotifyState::From_Json(warpJson);
			pState->Set_WarpCallback(WarpCallback);
			m_AnimNotifyStates.emplace_back(pState);
			continue;
		}
		if (type == "IK")
		{
			CIKNotifyState* pState = CIKNotifyState::From_Json(notifyObject);
			pState->Set_IKCallback(IKCallback);
			m_AnimNotifyStates.emplace_back(pState);
			continue;
		}

		CAnimNotify* pAnimNotify = { nullptr };

		if (type == "Sound")
			pAnimNotify = CSoundNotify::From_Json(notifyObject);
		else if (type == "Collider")
		{
			pAnimNotify = CColliderNotify::From_Json(notifyObject);
			pAnimNotify->Set_ColliderCallBack(ColliderCallback);
		}
		else if (type == "Effect")
		{
			pAnimNotify = CEffectNotify::From_Json(notifyObject);
			pAnimNotify->Set_EffectCallback(EffectCallback);
		}
		else if (type == "Object")
		{
			CObjectFuncNotify* pObjectNotify = CObjectFuncNotify::From_Json(notifyObject);
			pObjectNotify->Set_ObjectCallback(ObjectCallback);
			pAnimNotify = pObjectNotify;
		}
		ASSERT_CRASH(pAnimNotify);

		m_AnimNotifies.emplace_back(pAnimNotify);
	}
}


void CAnimation::Sort_Notify()
{
	if (0 == m_Notifies.size())
		return;

	sort(m_Notifies.begin(), m_Notifies.end(), [](const NOTIFY& Src, const NOTIFY& Dst)->_bool {
			return Src.fTrackPosition < Dst.fTrackPosition ? true : false;
		});
}

void CAnimation::Sort_AnimNotify()
{
	if (0 < m_AnimNotifies.size())
	{
		sort(m_AnimNotifies.begin(), m_AnimNotifies.end(), [](CAnimNotify* pSrc, CAnimNotify* pDst)->_bool{
			return pSrc->Get_TrackPosition() < pDst->Get_TrackPosition();
		});
	}

	if (0 < m_AnimNotifyStates.size())
	{
		sort(m_AnimNotifyStates.begin(), m_AnimNotifyStates.end(), [](CAnimNotifyState* pSrc, CAnimNotifyState* pDst)->_bool {
			return pSrc->Get_BeginTrackPos() < pDst->Get_BeginTrackPos();
		});
	}
}

void CAnimation::Update_NotifyStates()
{
	const _float t = m_fCurrentTrackPosition;
	for (auto& pState : m_AnimNotifyStates)
	{
		if (false == pState->Is_Active() && t >= pState->Get_BeginTrackPos() && t < pState->Get_EndTrackPos())
			pState->Notify_Begin();
		else if (pState->Is_Active() && t >= pState->Get_EndTrackPos())
			pState->Notify_End();
	}
}

void CAnimation::Force_EndNotifyStates()
{
	for (auto& pState : m_AnimNotifyStates)
	{
		if (pState->Is_Active())
			pState->Notify_End();
	}
}

void CAnimation::Collect_ActiveIKWindows(vector<ACTIVE_IK_WINDOW>& Out)
{
	for (CAnimNotifyState* pNotifyState : m_AnimNotifyStates)
	{
		if (pNotifyState->Get_NotifyTypeName() != "IK")
			continue;

		if (m_fCurrentTrackPosition < pNotifyState->Get_BeginTrackPos()
			|| m_fCurrentTrackPosition >= pNotifyState->Get_EndTrackPos())
			continue;

		CIKNotifyState* pIKState = static_cast<CIKNotifyState*>(pNotifyState);

		ACTIVE_IK_WINDOW Window{};
		Window.pBindings    = &pIKState->Get_Bindings();
		Window.fBlendInSec  = pIKState->Get_BlendInSec();
		Window.fBlendOutSec = pIKState->Get_BlendOutSec();

		const _float fBegin = pNotifyState->Get_BeginTrackPos();
		const _float fRamp  = (pIKState->Get_RampLen() > 0.f)
			? pIKState->Get_RampLen()
			: (pNotifyState->Get_EndTrackPos() - fBegin);
		if (fRamp > 0.f)
		{
			_float fProgress = (m_fCurrentTrackPosition - fBegin) / fRamp;
			if (fProgress < 0.f) fProgress = 0.f;
			if (fProgress > 1.f) fProgress = 1.f;
			Window.fProgress = fProgress;
		}

		Out.push_back(Window);
	}
}


void CAnimation::Reset_Status()
{
	m_fCurrentTrackPosition = 0.f;
	m_iNotifyIndex = 0;

	// Bone Channel 캐싱 인덱스 초기화
	if (!m_CurrentFrameIndices.empty())
		fill(m_CurrentFrameIndices.begin(), m_CurrentFrameIndices.end(), 0); // 0으로 채워서 처음부터 다시 검색하게 함

	// Morph Channel 캐싱 인덱스 초기화
	if (!m_CurrentMorphCurveIndicies.empty())
		fill(m_CurrentMorphCurveIndicies.begin(), m_CurrentMorphCurveIndicies.end(), 0); // 0번 키프레임부터 다시 찾도록 리셋
}

HRESULT CAnimation::Initialize(ifstream& InputFile, const vector<class CBone*>& Bones, MODELTYPE eModelType)
{
	// 얘넨 이름도 그대로쓰네?..

	if (MODELTYPE::ANIM == eModelType) // .fbx로 임포트한 경우?
	{
		if (FAILED(Ready_NormalAnimations(InputFile, Bones, eModelType)))
			return E_FAIL;
	}
	else if (MODELTYPE::CHARACTER == eModelType)
	{
		if (FAILED(Ready_CharacterAnimations(InputFile, Bones, eModelType)))
			return E_FAIL;
	}
	return S_OK;
}

_bool CAnimation::Update_TransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition)
{
	if(nullptr != pTrackPosition)
		*pTrackPosition = m_fCurrentTrackPosition;

	if (m_fCurrentTrackPosition > m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		return true;
	}

	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_TransformationMatrix(m_fCurrentTrackPosition, Bones, &m_CurrentFrameIndices[i]);
	}

	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;

	return false;
}

_bool CAnimation::Update_RibTransformationMatrices(_float fTrackPosition, const vector<class CBone*>& Bones, _float* pTrackPosition)
{
	m_fCurrentTrackPosition = fTrackPosition;

	if (m_fCurrentTrackPosition > m_fDuration)
	{
		m_fCurrentTrackPosition = 0.f;
		return true;
	}


	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_RibTransformationMatrix(m_fCurrentTrackPosition, Bones, &m_CurrentFrameIndices[i]);
	}

	return false;
}

_bool CAnimation::Update_TransformationMatrices_All(_float fTimeDelta, const vector<class CBone*>& Bones, _float* pTrackPosition)
{
	if (nullptr != pTrackPosition)
		*pTrackPosition = m_fCurrentTrackPosition;

	if (m_fCurrentTrackPosition > m_fDuration)
	{
		Force_EndNotifyStates();
		m_fCurrentTrackPosition = 0.f;
		m_iNotifyIndex = 0;
		return true;
	}

	while (m_iNotifyIndex < m_AnimNotifies.size() && m_fCurrentTrackPosition >= m_AnimNotifies[m_iNotifyIndex]->Get_TrackPosition())
		m_AnimNotifies[m_iNotifyIndex++]->Execute();

	Update_NotifyStates();



	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Update_TransformationMatrix_All(m_fCurrentTrackPosition, Bones, &m_CurrentFrameIndices[i]);
	}

	

	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;

	return false;
}

/*
	1. 현재 fWeight가 1.f를 넘으면 멈춥니다.
	2. 현재 TrackPosition은 계속해서 증가되어야 합니다.
*/
_bool CAnimation::Blend_TransformationMatrices(_float fTimeDelta, const vector<class CBone*>& Bones, _float fWeight)
{
	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		m_Channels[i]->Blend_TransformationMatrix(m_fCurrentTrackPosition, Bones, fWeight);
	}
	
	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;

	return fWeight >= 1.f;
}


void CAnimation::Sample_AtTrackPosition(_float fTrackPosition, const vector<class CBone*>& Bones)
{
	_float fClamped = min(fTrackPosition, m_fDuration);
	for (size_t i = 0; i < m_iNumChannels; ++i)
		m_Channels[i]->Sample_TransformationMatrix(fClamped, Bones);
}

void CAnimation::Blend_AtTrackPosition(_float fTrackPosition, const vector<class CBone*>& Bones, _float fWeight)
{
	_float fClamped = min(fTrackPosition, m_fDuration);
	for (size_t i = 0; i < m_iNumChannels; ++i)
		m_Channels[i]->Blend_TransformationMatrix_At(fClamped, Bones, fWeight);
}

void CAnimation::Sample_BoneAtTrackPosition(_float fTrackPosition, const vector<class CBone*>& Bones, _uint iBoneIndex)
{
	_float fClamped = min(fTrackPosition, m_fDuration);
	for (size_t i = 0; i < m_iNumChannels; ++i)
	{
		if (m_Channels[i]->Get_BoneIndex() != iBoneIndex)
			continue;
		m_Channels[i]->Sample_TransformationMatrix(fClamped, Bones);
		return;
	}
}

_bool CAnimation::Update_TrackPosition(_float fTimeDelta, _float* pTrackPosition)
{
	if (nullptr != pTrackPosition)
		*pTrackPosition = m_fCurrentTrackPosition;

	if (m_fCurrentTrackPosition > m_fDuration)
	{
		Force_EndNotifyStates();	
		m_fCurrentTrackPosition = 0.f;
		m_iNotifyIndex = 0;
		return true;
	}

	while (m_iNotifyIndex < m_AnimNotifies.size() && m_fCurrentTrackPosition >= m_AnimNotifies[m_iNotifyIndex]->Get_TrackPosition())
		m_AnimNotifies[m_iNotifyIndex++]->Execute();

	Update_NotifyStates();

 	m_fCurrentTrackPosition += m_fTickPerSecond * fTimeDelta;

	return false;
}

// MorphChannels 바인딩.
_bool CAnimation::Bind_MorphChannels(const vector<string>& modelShapeKeys)
{
	m_MorphKeyIndicies.clear();
	m_MorphKeyIndicies.resize(m_MorphMeshChannels.size(), -1); // -1로 초기화.

	for (size_t i = 0; i < m_MorphMeshChannels.size(); ++i)
	{
		const _char* pChannelName = m_MorphMeshChannels[i]->Get_Name();

		for (size_t j = 0; j < modelShapeKeys.size(); ++j)
		{
			if (modelShapeKeys[j] == pChannelName)
			{
				m_MorphKeyIndicies[i] = static_cast<_int>(j); 
				break;
			}
		}
	}
	
	return true;
}

_bool CAnimation::Update_MorphWeights(_float fTimeDelta, vector<float>& modelWeights)
{

	if (modelWeights.empty()) return false;

	fill(modelWeights.begin(), modelWeights.end(), 0.0f);

	for (size_t i = 0; i < m_MorphMeshChannels.size(); ++i)
	{
		_int iTargetIndex = m_MorphKeyIndicies[i];

		if (iTargetIndex == -1) continue;
		if (iTargetIndex >= modelWeights.size()) continue;

		_float fWeight = m_MorphMeshChannels[i]->Get_CurrentWeight(m_fCurrentTrackPosition, &m_CurrentMorphCurveIndicies[i]);

		modelWeights[iTargetIndex] = fWeight;
	}

	return true;
}

HRESULT CAnimation::Ready_NormalAnimations(ifstream& InputFile, const vector<class CBone*>& Bones, MODELTYPE eModelType)
{

#pragma region 기존 애니메이션 내용 저장.
	_uint iLength = {};
	InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
	_char szAnimationName[MAX_PATH] = {};
	InputFile.read(szAnimationName, iLength);

	_char* pAnimationName = { nullptr };
	strtok_s(szAnimationName, "|", &pAnimationName);
	if (0 == strcmp(pAnimationName, ""))
		strcpy_s(m_szName, szAnimationName);
	else
		strcpy_s(m_szName, pAnimationName);

	InputFile.read(reinterpret_cast<_char*>(&m_fDuration), sizeof(_float));
	InputFile.read(reinterpret_cast<_char*>(&m_fTickPerSecond), sizeof(_float));

	InputFile.read(reinterpret_cast<_char*>(&m_iNumChannels), sizeof(_uint));

	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		CChannel* pChannel = CChannel::Create(InputFile, Bones);
		if (nullptr == pChannel)
			return E_FAIL;
		m_Channels.push_back(pChannel);
	}

	m_CurrentFrameIndices.resize(m_iNumChannels);

	return S_OK;
}

HRESULT CAnimation::Ready_CharacterAnimations(ifstream& InputFile, const vector<class CBone*>& Bones, MODELTYPE eModelType)
{
#pragma region 기존 애니메이션 내용 저장.
	_uint iLength = {};
	InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
	_char szAnimationName[MAX_PATH] = {};
	InputFile.read(szAnimationName, iLength);
	
	// .gltf의 경우 애니메이션 이름이 그대로 저장됨.
	strcpy_s(m_szName, szAnimationName);

	InputFile.read(reinterpret_cast<_char*>(&m_fDuration), sizeof(_float));
	InputFile.read(reinterpret_cast<_char*>(&m_fTickPerSecond), sizeof(_float));

	InputFile.read(reinterpret_cast<_char*>(&m_iNumChannels), sizeof(_uint));

	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		CChannel* pChannel = CChannel::Create(InputFile, Bones);
		if (nullptr == pChannel)
			return E_FAIL;
		m_Channels.push_back(pChannel);
	}

	m_CurrentFrameIndices.resize(m_iNumChannels);


	// 추가 작업.
	if (eModelType == MODELTYPE::CHARACTER)
	{
		// 1. Morph Mesh Curves 개수를 받아옵니다. => 	ModelLoader의 iTotalCurves와 동일.
		InputFile.read(reinterpret_cast<_char*>(&m_iNumMorphCurves), sizeof(_uint));

		// 2. Morph Mesh Curves 정보를 가져옵니다.
		for (_uint i = 0; i < m_iNumMorphCurves; ++i)
		{
			CMorphChannel* pMorphChannel = CMorphChannel::Create(InputFile);
			if (nullptr == pMorphChannel)
				return E_FAIL;

			m_MorphMeshChannels.push_back(pMorphChannel);
		}

		m_CurrentMorphCurveIndicies.resize(m_iNumMorphCurves);

	}
#pragma endregion

	return S_OK;
}

CAnimation* CAnimation::Create(ifstream& InputFile, const vector<class CBone*>& Bones, MODELTYPE eModelType)
{
	CAnimation* pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(InputFile, Bones, eModelType)))
	{
		MSG_BOX("Failed to Create : Animation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation* CAnimation::Clone()
{
	return new CAnimation(*this);
}

void CAnimation::Free()
{
	__super::Free();

	for (auto& pAnimNotfiy : m_AnimNotifies)
		Safe_Release(pAnimNotfiy);
	m_AnimNotifies.clear();

	for (auto& pNotifyState : m_AnimNotifyStates)
		Safe_Release(pNotifyState);
	m_AnimNotifyStates.clear();

	m_Notifies.clear();

	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);
	m_Channels.clear();

	for (auto& pMorphCannel : m_MorphMeshChannels)
		Safe_Release(pMorphCannel);

	m_MorphMeshChannels.clear();
}
