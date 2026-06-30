#pragma once
#include "Base.h"
#include "Editor_Define.h"
#include "AnimNotifyDefine.h"

NS_BEGIN(Engine)
class CAnimNotify;
class CSoundNotify;
class CColliderNotify;
class CEffectNotify;
class CObjectFuncNotify;
NS_END

NS_BEGIN(Editor)
class CAnimNotifyTool final : public CBase
{
public:
	enum class NOTIFYTYPE : _uint
	{
		SOUND = 0,	  // SOUND
		EFFECT = 1,   // EFFECT 
		COLLIDER = 2, // 
		LIGHT = 3,    // LIGHT?
		OBJECT = 4,
		END
	};

	


private:
	explicit CAnimNotifyTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CAnimNotifyTool() = default;



#pragma region
public:
	HRESULT	Initialize(LEVEL eLevel);
	void Update();
	void Render();
#pragma endregion


#pragma region ANIMATION Tool
public:
	void Process_Notify(class CAnimationActor* pActor, const _string& strAnimName, const _string& strModelDirPath, _float fTrackPosition);
	
#pragma endregion

public:
	void Clear();

private:
	// Depth1
	void RenderUI_EditNotify();


private:
	// Depth2
	void RenderUI_EditSound();
	void RenderUI_EditEffect();
	void RenderUI_EditCollider();
	void RenderUI_EditObjectFunc();
	void RenderUI_SaveNotify();
	void RenderUI_LoadNotify();
	

private:
	// Depth3
	void Load_SoundFiles();
	void Select_SoundNotify();

#ifdef _DEBUG
	void Render_CurrentNotify();
#endif // _DEBUG

	void Save_Notify();
	void Load_NotifyFromFile();


private:
	// Depth4
	void Load_SoundsFromFile(const _string& strFilePath, const _string& strSoundPath);
	void Load_AllSoundsFromFolder(const _string& strFolderPath);
	
	void Edit_SoundNotify();

	void Save_NotifyToJson(const _string& strFilePath);
	void Load_NotifyFromJson(const _string& strFilePath);


	

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	class CAnimationActor* m_pCurrentActor = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	LEVEL m_eCurLevel = { LEVEL::END };
	NOTIFYTYPE m_eType = { NOTIFYTYPE::END };

	_string m_strCurrentAnimName = {};
	_string m_strCurrentFolderPath = {};
	_float m_fCurrentDuration = {};

	// Sound Tag
	map<const _string, const _wstring> m_SoundTags = {};
	_string m_CurrentSoundTag = {};
	_string m_CurrentSoundType = {};

	_bool m_IsLoadNotify = { false };

private:
	list<CAnimNotify*>     m_AnimNotifies;
	list<CSoundNotify*>    m_SoundNotifies;
	list<CColliderNotify*> m_ColliderNotifies;
	list<CEffectNotify*> m_EffectNotifies;
	list<CObjectFuncNotify*> m_ObjectNotifies;

private:
	HRESULT Ready_Sound();

public:
	static CAnimNotifyTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);
	virtual	void Free() override;

};
NS_END

