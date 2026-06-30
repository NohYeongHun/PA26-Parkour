#include "EnginePch.h"
#include "Sound_Manager.h"
#include "GameInstance.h"

CSound_Manager::CSound_Manager()
{
}

void CSound_Manager::Update_Listener(class CTransform* pTransform, _float fTimeDelta)
{
	_vector vListenerPosition = pTransform->Get_State(STATE::POSITION);
	_vector vListenerVelocity = pTransform->Get_Velocity();
	_vector vListenerForward = XMVector4Normalize(pTransform->Get_State(STATE::LOOK));
	_vector vListenerUp = XMVector4Normalize(pTransform->Get_State(STATE::UP));

	if (XMVectorGetX(XMVector3Length(vListenerVelocity)) > 3.f)
		vListenerVelocity = XMVector4Normalize(vListenerVelocity) * 3.f;

	FMOD_VECTOR vPosition = {};
	FMOD_VECTOR vVelocity = {};
	FMOD_VECTOR vForward = {};
	FMOD_VECTOR vUp = { 0.f, 1.f, 0.f };

	memcpy(&vPosition, &vListenerPosition, sizeof(_float) * 3);
	memcpy(&vVelocity, &vListenerVelocity, sizeof(_float) * 3);
	memcpy(&vForward, &vListenerForward, sizeof(_float) * 3);
	vForward.y = 0.f;

	FMOD_System_Set3DListenerAttributes(m_pSystem, 0, &vPosition, &vVelocity, &vForward, &vUp);
}

_uint CSound_Manager::Register_Channel()
{
	if (0 == m_iPoolingChannelIndex.size())
		return MAX_CHANNEL - 1;

	_uint iChannelIndex = m_iPoolingChannelIndex.front();
	m_iPoolingChannelIndex.pop();

	return iChannelIndex;
}

void CSound_Manager::Return_Channel(_uint iChannelIndex)
{
	m_iPoolingChannelIndex.push(iChannelIndex);
}

HRESULT CSound_Manager::Load_Sound(const _wstring& strSoundTag, const _char* pSoundFilePath, _bool is3D)
{
    FMOD_SOUND* pSound = Find_Sound(strSoundTag);

    if (nullptr != pSound)
        return S_OK;

	FMOD_MODE mode = {};
	FMOD_RESULT eResult = {};

	mode = FMOD_DEFAULT;
	eResult = FMOD_System_CreateSound(m_pSystem, pSoundFilePath, mode, 0, &pSound);

	//if (false == is3D)
	//{
	//
	//}
	//else
	//{
	//	mode = FMOD_DEFAULT | FMOD_CREATESAMPLE | FMOD_3D;
	//	eResult = FMOD_System_CreateSound(m_pSystem, pSoundFilePath, mode, 0, &pSound);
	//}

    if (FMOD_OK == eResult)
        m_Sounds.emplace(strSoundTag, pSound);
    else
    {
        MSG_BOX("Failed to Load : Sound File");
        return E_FAIL;
    }

    //FMOD_System_Update(m_pSystem);

    return S_OK;
}

HRESULT CSound_Manager::Load_Sound_FromFolder(const _char* pFolderPath, _bool is3D)
{
	for (const auto& entry : filesystem::directory_iterator(pFolderPath))
	{
		if (entry.is_regular_file())
		{
			_string filePath = entry.path().string();
			_string fileName = entry.path().filename().string();
			_string extension = entry.path().extension().string();

			// .wav
			if (extension == ".wav" || extension == ".WAV")
			{
				_string soundTag = entry.path().stem().string();
				_wstring wSoundTag = StringToWString(soundTag);

				if(FAILED(Load_Sound(wSoundTag, filePath.c_str(), is3D)))
					CRASH("Sound");
			}
		}
	}
	return S_OK;
}


HRESULT CSound_Manager::Load_Sound_FromFolderRecursive(const _char* pFolderPath, _bool is3D)
{
	for (const auto& entry : filesystem::recursive_directory_iterator(pFolderPath))
	{
		if (entry.is_regular_file())
		{
			_string filePath = entry.path().string();
			_string extension = entry.path().extension().string();

			if (extension == ".wav" || extension == ".WAV")
			{
				// 중복 주의
				_string soundTag = entry.path().stem().string();
				_wstring wSoundTag = StringToWString(soundTag);

				if (FAILED(Load_Sound(wSoundTag, filePath.c_str(), is3D)))
					CRASH("Sound Load Failed");
			}
		}
	}
	return S_OK;
}

void CSound_Manager::Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency)
{
    FMOD_SOUND* pSound = Find_Sound(strSoundTag);
    if (nullptr == pSound)
        return;

	FMOD_System_PlaySound(m_pSystem, pSound, nullptr, true, &m_pFixedChannels[iChannelID]);
	FMOD_Channel_SetMode(m_pFixedChannels[iChannelID], FMOD_2D);
    FMOD_Channel_SetVolume(m_pFixedChannels[iChannelID], fVolume);
	FMOD_Channel_SetPaused(m_pFixedChannels[iChannelID], false);
	FMOD_Channel_SetPitch(m_pFixedChannels[iChannelID], fFrequency);
}

void CSound_Manager::Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, CTransform* pTransform, _float fMinDistance, _float fMaxDistance, _float fFrequency)
{
	FMOD_SOUND* pSound = Find_Sound(strSoundTag);
	if (nullptr == pSound)
		return;

	_vector vObjectPosition = pTransform->Get_State(STATE::POSITION);
	_vector vObjectVelocity = pTransform->Get_Velocity();

	FMOD_VECTOR vPosition = {};
	FMOD_VECTOR vVelocity = {};

	memcpy(&vPosition, &vObjectPosition, sizeof(_float) * 3);
	//memcpy(&vVelocity, &vObjectVelocity, sizeof(_float) * 3);

	FMOD_System_PlaySound(m_pSystem, pSound, nullptr, true, &m_pFixedChannels[iChannelID]);

	_float fMinDst = fMinDistance;
	if (fMinDst <= 0.01f)
		fMinDst = 0.01f;

	FMOD_Channel_SetMode(m_pFixedChannels[iChannelID], FMOD_3D | FMOD_3D_LINEARROLLOFF);
	FMOD_Channel_Set3DAttributes(m_pFixedChannels[iChannelID], &vPosition, &vVelocity);
	FMOD_Channel_Set3DMinMaxDistance(m_pFixedChannels[iChannelID], fMinDst * m_fDistanceBias, fMaxDistance * m_fDistanceBias);
	FMOD_Channel_SetVolume(m_pFixedChannels[iChannelID], fVolume);
	FMOD_Channel_SetPaused(m_pFixedChannels[iChannelID], false);
	FMOD_Channel_SetPitch(m_pFixedChannels[iChannelID], fFrequency);
}

void CSound_Manager::Play_Sound_Dynamic(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency)
{
	FMOD_SOUND* pSound = Find_Sound(strSoundTag);
	if (nullptr == pSound)
		return;

	FMOD_System_PlaySound(m_pSystem, pSound, nullptr, true, &m_pPoolingChannels[iChannelID]);
	FMOD_Channel_SetMode(m_pPoolingChannels[iChannelID], FMOD_2D);
	FMOD_Channel_SetVolume(m_pPoolingChannels[iChannelID], fVolume);
	FMOD_Channel_SetPaused(m_pPoolingChannels[iChannelID], false);
	FMOD_Channel_SetPitch(m_pPoolingChannels[iChannelID], fFrequency);
}

void CSound_Manager::Play_Sound_Dynamic(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, CTransform* pTransform, _float fMinDistance, _float fMaxDistance, _float fFrequency)
{
	FMOD_SOUND* pSound = Find_Sound(strSoundTag);
	if (nullptr == pSound)
		return;

	_vector vObjectPosition = pTransform->Get_State(STATE::POSITION);
	_vector vObjectVelocity = pTransform->Get_Velocity();

	FMOD_VECTOR vPosition = {};
	FMOD_VECTOR vVelocity = {};

	memcpy(&vPosition, &vObjectPosition, sizeof(_float) * 3);
	//memcpy(&vVelocity, &vObjectVelocity, sizeof(_float) * 3);

	FMOD_System_PlaySound(m_pSystem, pSound, nullptr, true, &m_pPoolingChannels[iChannelID]);

	_float fMinDst = fMinDistance;
	if (fMinDistance <= 0.01f)
		fMinDst = 0.01f;

	FMOD_Channel_SetMode(m_pPoolingChannels[iChannelID], FMOD_3D | FMOD_3D_LINEARROLLOFF);
	FMOD_Channel_Set3DAttributes(m_pPoolingChannels[iChannelID], &vPosition, &vVelocity);
	FMOD_Channel_Set3DMinMaxDistance(m_pPoolingChannels[iChannelID], fMinDst * m_fDistanceBias, fMaxDistance * m_fDistanceBias);
	FMOD_Channel_SetVolume(m_pPoolingChannels[iChannelID], fVolume);
	FMOD_Channel_SetPaused(m_pPoolingChannels[iChannelID], false);
	FMOD_Channel_SetPitch(m_pPoolingChannels[iChannelID], fFrequency);
}

void CSound_Manager::Play_BGM(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency)
{
	// BGM OFF 용 임시 코드
	return;

    FMOD_SOUND* pSound = Find_Sound(strSoundTag);
    if (nullptr == pSound)
        return;

    FMOD_System_PlaySound(m_pSystem, pSound, nullptr, true, &m_pFixedChannels[iChannelID]);
    FMOD_Channel_SetMode(m_pFixedChannels[iChannelID], FMOD_LOOP_NORMAL);
    FMOD_Channel_SetVolume(m_pFixedChannels[iChannelID], fVolume);
	FMOD_Channel_SetPaused(m_pFixedChannels[iChannelID], false);
	FMOD_Channel_SetPitch(m_pFixedChannels[iChannelID], fFrequency);
}

void CSound_Manager::Stop_Sound(_uint iChannelID)
{
	FMOD_Channel_SetPaused(m_pFixedChannels[iChannelID], true);
    FMOD_Channel_Stop(m_pFixedChannels[iChannelID]);
	m_pFixedChannels[iChannelID] = nullptr;
}

void CSound_Manager::Stop_Sound_Dynamic(_uint iChannelID)
{
	FMOD_Channel_SetPaused(m_pPoolingChannels[iChannelID], true);
	FMOD_Channel_Stop(m_pPoolingChannels[iChannelID]);
	m_pPoolingChannels[iChannelID] = nullptr;
}

void CSound_Manager::Stop_All()
{
    for (size_t i = 0; i < m_iNumChannels; ++i)
    {
		if (nullptr != m_pFixedChannels[i])
		{
			FMOD_Channel_SetPaused(m_pFixedChannels[i], true);
            FMOD_Channel_Stop(m_pFixedChannels[i]);
			m_pFixedChannels[i] = nullptr;
		}
    }

	for (auto& pChannel : m_pPoolingChannels)
	{
		if (nullptr != pChannel)
		{
			FMOD_Channel_SetPaused(pChannel, true);
			FMOD_Channel_Stop(pChannel);
			pChannel = nullptr;
		}
	}
}

void CSound_Manager::Set_ChannelVolume(_uint iChannelID, _float fVolume)
{
    FMOD_Channel_SetVolume(m_pFixedChannels[iChannelID], fVolume);
}

void CSound_Manager::Set_ChannelVolume_Dynamic(_uint iChannelID, _float fVolume)
{
	FMOD_Channel_SetVolume(m_pPoolingChannels[iChannelID], fVolume);
}

HRESULT CSound_Manager::Initialize(_uint iNumChannel)
{
	m_iNumChannels = iNumChannel;
	m_pFixedChannels.resize(m_iNumChannels);
	m_pPoolingChannels.resize(MAX_CHANNEL - m_iNumChannels);

	for (_uint i = 0; i < iNumChannel; ++i)
		m_pFixedChannels[i] = nullptr;

	for (_uint i = 0; i < MAX_CHANNEL - m_iNumChannels; ++i)
	{
		m_iPoolingChannelIndex.push(i);
		m_pPoolingChannels[i] = nullptr;
	}

    FMOD_System_Create(&m_pSystem, FMOD_VERSION);

    FMOD_System_Init(m_pSystem, m_iNumChannels, FMOD_INIT_NORMAL, nullptr);

	FMOD_System_Set3DSettings(m_pSystem, 1.f, 0.01f, 1.f);

	m_fDistanceBias = 1.f;

    return S_OK;
}

void CSound_Manager::Update(_float fTimeDelta)
{
	FMOD_System_Update(m_pSystem);
}

HRESULT CSound_Manager::Clear_Resource()
{
	Stop_All();
	FMOD_System_Update(m_pSystem);
	for (auto& Pair : m_Sounds)
		FMOD_Sound_Release(Pair.second);
	m_Sounds.clear();

	return S_OK;
}

FMOD_SOUND* CSound_Manager::Find_Sound(const _wstring& strSoundTag)
{
    auto iter = m_Sounds.find(strSoundTag);

    if (iter == m_Sounds.end())
        return nullptr;

    return iter->second;
}

CSound_Manager* CSound_Manager::Create(_uint iNumChannel)
{
    CSound_Manager* pInstance = new CSound_Manager();

    if (FAILED(pInstance->Initialize(iNumChannel)))
    {
        MSG_BOX("Failed To Create : Sound_Manager");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSound_Manager::Free()
{
    __super::Free();
	
	Stop_All();
    for (auto& Pair : m_Sounds)
        FMOD_Sound_Release(Pair.second);
    m_Sounds.clear();

    FMOD_System_Close(m_pSystem);
    FMOD_System_Release(m_pSystem);
}
