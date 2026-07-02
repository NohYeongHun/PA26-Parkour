#pragma once
#include "GameObject.h"
NS_BEGIN(Client)
class CMapObject_DynamicSound final: public CGameObject
{
public:
	typedef struct tagSoundDesc {
		_float4x4 SoundMat;
		_uint iSoundIndex;
	}SOUND_DESC;
private:
	explicit CMapObject_DynamicSound(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMapObject_DynamicSound(const CMapObject_DynamicSound& Prototype);
	virtual ~CMapObject_DynamicSound() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override {};
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override {};
	virtual		void			Render() override {};

private:
	class CGameSystem* m_pGameSystem = { nullptr };
	_int	m_iSoundChannel = { -1 };
	_uint m_iSoundIndex = {};
	_float m_fSoundCoolDown = {};
	_float m_fTotalTime = {};
	_float m_fSoundVolume = {};
public:
	static vector<_wstring> m_SoundTags;
public:
	static		CMapObject_DynamicSound* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};


NS_END