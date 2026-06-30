#include "EnginePch.h"
#include "Animation_Inst.h"

#include "Channel.h"

#include "SoundNotify.h"
#include "ColliderNotify.h"
#include "EffectNotify.h"
#include "ObjectFuncNotify.h"

CAnimation_Inst::CAnimation_Inst()
{
}

CAnimation_Inst::CAnimation_Inst(const CAnimation_Inst& Prototype)
	: m_fDuration { Prototype.m_fDuration },
	m_fTickPerSecond { Prototype.m_fTickPerSecond },
	m_iNumChannels { Prototype.m_iNumChannels },
	m_Channels { Prototype.m_Channels },
	m_CurrentFrameIndices{ Prototype.m_CurrentFrameIndices }
{
	strcpy_s(m_szName, Prototype.m_szName);

	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);

	
}

void CAnimation_Inst::Register_Notify(const NOTIFY& AnimNotify)
{
	m_Notifies.push_back(AnimNotify);
}

void CAnimation_Inst::Load_Notify(const json& notifyJson, function<void(const _wstring&, _bool)> ColliderCallback, function<void(const _wstring&)> EffectCallback, function<void(const _wstring&)> ObjectCallback)
{
	for (const auto& notifyObject : notifyJson)
	{
		string type = notifyObject["NotifyType"].get<string>();
		CAnimNotify* pAnimNotify = { nullptr };

		// 1. 
		if (type == "Sound")
			pAnimNotify = CSoundNotify::From_Json(notifyObject);
		else if (type == "Collider")
		{
			pAnimNotify = CColliderNotify::From_Json(notifyObject);
			pAnimNotify->Set_ColliderCallBack(ColliderCallback);
		}
		else if (type == "Effect")
		{
			// EffectNotify
			pAnimNotify = CEffectNotify::From_Json(notifyObject);
			pAnimNotify->Set_EffectCallback(EffectCallback);
		}
		else if (type == "Object")
		{
			// ObjectNotify
			pAnimNotify = CObjectFuncNotify::From_Json(notifyObject);
			dynamic_cast<CObjectFuncNotify*>(pAnimNotify)->Set_ObjectCallback(ObjectCallback);
		}
		ASSERT_CRASH(pAnimNotify);

		m_AnimNotifies.emplace_back(pAnimNotify);
	}
}


void CAnimation_Inst::Sort_Notify()
{
	if (0 == m_Notifies.size())
		return;

	sort(m_Notifies.begin(), m_Notifies.end(), [](const NOTIFY& Src, const NOTIFY& Dst)->_bool {
			return Src.fTrackPosition < Dst.fTrackPosition ? true : false;
		});
}

void CAnimation_Inst::Sort_AnimNotify()
{
	if (0 == m_AnimNotifies.size())
		return;

	sort(m_AnimNotifies.begin(), m_AnimNotifies.end(), [](CAnimNotify* pSrc, CAnimNotify* pDst)->_bool{
		return pSrc->Get_TrackPosition() < pDst->Get_TrackPosition();
	});
}

HRESULT CAnimation_Inst::Initialize(ifstream& InputFile, const vector<class CBone*>& Bones)
{
	_uint iLength = {};
	InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
	_char szAnimationName[MAX_PATH] = {};
	InputFile.read(szAnimationName, iLength);

	_char* pAnimationName = { nullptr };
	strtok_s(szAnimationName, "|", &pAnimationName);
	if(0 == strcmp(pAnimationName, ""))
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

	//auto iter = find_if(m_Channels.begin(), m_Channels.end(), [](CChannel* pChannel) {
	//	return pChannel->Get_BoneIndex() == 1;
	//	});
	//if (iter != m_Channels.end())
	//{
	//	// 문제되는 채널 찾기
	//	vector<KEYFRAME>* temp = (*iter)->Get_KeyframesPtr();
	//	for (auto& tKeyFrame : *temp)
	//	{
	//		tKeyFrame.vScale = _float3(1.f, 1.f, 1.f);
	//	}
	//}
	m_CurrentFrameIndices.resize(m_iNumChannels);

	return S_OK;
}

_bool CAnimation_Inst::Update_TrackPosition(_float fTimeDelta, _float* pTrackPosition, _bool isRootMotion, _matrix* Out)
{
	if(nullptr == pTrackPosition)
		CRASH("pTrackPosition is nullptr");

	if (*pTrackPosition > m_fDuration)
	{
		*pTrackPosition = 0.f;
		m_iNotifyIndex = 0;
		return true;
	}

	//while (m_iNotifyIndex < m_AnimNotifies.size() && (*pTrackPosition) >= m_AnimNotifies[m_iNotifyIndex]->Get_TrackPosition())
	//	m_AnimNotifies[m_iNotifyIndex++]->Execute();

	*pTrackPosition += m_fTickPerSecond * fTimeDelta;

	return false;
}

CAnimation_Inst* CAnimation_Inst::Create(ifstream& InputFile, const vector<class CBone*>& Bones)
{
	CAnimation_Inst* pInstance = new CAnimation_Inst();

	if (FAILED(pInstance->Initialize(InputFile, Bones)))
	{
		MSG_BOX("Failed to Create : Animation");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation_Inst* CAnimation_Inst::Clone()
{
	return new CAnimation_Inst(*this);
}

void CAnimation_Inst::Free()
{
	__super::Free();

	for (auto& pAnimNotfiy : m_AnimNotifies)
		Safe_Release(pAnimNotfiy);
	m_AnimNotifies.clear();

	m_Notifies.clear();

	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);
}
