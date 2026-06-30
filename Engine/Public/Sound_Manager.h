#pragma once
#include "Base.h"

#define MAX_CHANNEL 512

NS_BEGIN(Engine)

class CSound_Manager final : public CBase
{
private:
	explicit CSound_Manager();
	virtual ~CSound_Manager() = default;

public:
	// Player에서 호출할 함수 (Listener Attribute 갱신)
	void			Update_Listener(class CTransform* pTransform, _float fTimeDelta);
	// Pooling Channel
	_uint			Register_Channel();
	// Return Channel
	void			Return_Channel(_uint iChannelIndex);

public:
	// Sound Load (Sound File Key, Sound File Path)
	HRESULT			Load_Sound(const _wstring& strSoundTag, const _char* pSoundFilePath, _bool is3D);
	// Sound Load From Folder
	HRESULT			Load_Sound_FromFolder(const _char* pFolderPath, _bool is3D);
	// Sound Load From Folder Recursive(재귀)
	HRESULT			Load_Sound_FromFolderRecursive(const _char* pFolderPath, _bool is3D);

	// 고정 채널 Sound 재생
	void			Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency);	// 2D
	void			Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, class CTransform* pTransform, _float fMinDistance, _float fMaxDistance, _float fFrequency);	// 3D
	// 동적 채널 Sound 재생
	void			Play_Sound_Dynamic(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency);
	void			Play_Sound_Dynamic(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, class CTransform* pTransform, _float fMinDistance, _float fMaxDistance, _float fFrequency);

	// BGM 재생
	void			Play_BGM(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency);
	// 고정 채널 Sound 멈춤
	void			Stop_Sound(_uint iChannelID);
	// 동적 채널 Sound 재생
	void			Stop_Sound_Dynamic(_uint iChannelID);
	// Sound All Stop
	void			Stop_All();
	// 고정 채널 Volume 조절
	void			Set_ChannelVolume(_uint iChannelID, _float fVolume);
	// 동적 채널 Volume 조절
	void			Set_ChannelVolume_Dynamic(_uint iChannelID, _float fVolume);

public:
	HRESULT		Initialize(_uint iNumChannel);
	void			Update(_float fTimeDelta);
	HRESULT		Clear_Resource();

private:
	FMOD_SYSTEM* m_pSystem = { nullptr };

	_uint													m_iNumChannels = {};
	queue<_uint>										m_iPoolingChannelIndex;

	// ?ъ슫??由ъ냼??蹂닿? 而⑦뀒?대꼫
	map<const _wstring, FMOD_SOUND*>		m_Sounds;
	// 고정 채널
	vector<FMOD_CHANNEL*>						m_pFixedChannels;
	// 유동 채널
	vector<FMOD_CHANNEL*>						m_pPoolingChannels;

	_float													m_fDistanceBias = {};

private:
	FMOD_SOUND* Find_Sound(const _wstring& strSoundTag);

public:
	static		CSound_Manager* Create(_uint iNumChannel);
	virtual		void						Free() override;
};

NS_END