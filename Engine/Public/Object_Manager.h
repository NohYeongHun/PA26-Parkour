#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CObject_Manager final : public CBase
{
private:
	explicit CObject_Manager();
	virtual ~CObject_Manager() = default;

public:
	HRESULT		Add_GameObject_ToLayer(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, void* pArg);
	HRESULT		Add_GameObject_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, class CGameObject* pObject);
	class CComponent* Get_Component(_uint iLayerLevelID, const _wstring& strLayerTag, _uint iGameObjectIndex, const _wstring& strComponentTag);
	HRESULT		Change_TimeRatio_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, _float fTimeRatio, _bool isTimeStop);
	HRESULT		Change_TimeRatio_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, _float fTimeRatio, _float fDuration);

public:
	HRESULT		Initialize(_uint iNumLevel);
	void			Priority_Update(_float fTimeDelta);
	void			Update(_float fTimeDelta);
	void			Late_Update(_float fTimeDelta);
	HRESULT		Clear_Resource(_uint iClearLevelID);

private:
	_uint							m_iNumLevel = {};
	class CGameInstance*	m_pGameInstance = { nullptr };

	map<const _wstring, class CLayer*>* m_Layers = { nullptr };
	typedef map<const _wstring, class CLayer*> LAYERS;

	mutex							m_Mutex;

private:
	class CLayer* Find_Layer(_uint iLayerLevelID, const _wstring& strLayerTag);

public:
	static		CObject_Manager*	Create(_uint iNumLevel);
	virtual		void						Free() override;

};

NS_END