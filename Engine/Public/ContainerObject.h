#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CContainerObject abstract : public CGameObject
{
protected:
	explicit CContainerObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CContainerObject(const CContainerObject& Prototype);
	virtual ~CContainerObject() = default;

public:
	virtual		HRESULT				Initialize_Prototype();
	virtual		HRESULT				Initialize_Clone(void* pArg);
	virtual		void					Priority_Update(_float fTimeDelta);
	virtual		void					Update(_float fTimeDelta);
	virtual		void					Late_Update(_float fTimeDelta);
	virtual		void					Render();

protected:
	typedef map<const _wstring, class CPartObject*> PARTOBJECTS;
	PARTOBJECTS		m_PartObjects;

protected:
	HRESULT							Add_PartObject(const _wstring& strPartObjectName, _uint iPrototypeLevelID, const _wstring& strPrototypeTag, void* pArg = nullptr);
	class CPartObject*				Find_PartObject(const _wstring& strPartObjectName);

public:
	virtual		CGameObject*		Clone(void* pArg) = 0;
	virtual		void					Free() override;
};

NS_END