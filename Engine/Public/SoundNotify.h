#pragma once
#include "AnimNotify.h"

NS_BEGIN(Engine)
class ENGINE_DLL CSoundNotify final : public CAnimNotify
{
public:
	explicit CSoundNotify(_float fTrackPosition, const _string& strTag, const _string& strSoundType, _uint iChannel, _float fVolume);
	virtual void Execute() override;
	virtual json To_Json() const override;
	virtual const _string& Get_NotifyTypeName() const override;


#ifdef _DEBUG

	virtual void ImGui_Print() override;
#endif // _DEBUG


public:
	static CSoundNotify* From_Json(const json& soundJson);
	virtual void Free() override;

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	_string m_strSoundType;  // SoundType
	_string m_strSoundTag;   // SoundTag
	_wstring m_wStrSoundTag; 
	_uint m_iChannel;
	_float m_fVolume;

	


};
NS_END
