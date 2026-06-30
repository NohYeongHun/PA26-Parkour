#pragma once
#include "Base.h"
#include "Transform.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC {

	}GAMEOBJECT_DESC;
protected:
	explicit CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CGameObject(const CGameObject& Prototype);
	virtual ~CGameObject() = default;

public:
	class CComponent*	Get_Component(const _wstring& strComponentTag);
	_bool						IsActivate() { return m_isActivate; }
	void						SetActivate(_bool isActivate) { m_isActivate = isActivate; }
	_uint						Get_ID() { return m_iObjectID; }

public:
	virtual		HRESULT			Initialize_Prototype();
	virtual		HRESULT			Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
	virtual		void			Render_Shadow();
	virtual		void			Render_OutLine();
	virtual		void			Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix);

	// Pooling Spawn CallBack
	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

protected:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	class CGameInstance*		m_pGameInstance = { nullptr };
	class CTransform*			m_pTransformCom = { nullptr };

	map<const _wstring, class CComponent*>	m_Components;

	// Activate
	_bool							m_isActivate = { true };
	// Object ID
	_uint							m_iObjectID = {};

protected:
	HRESULT						Add_Component(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, const _wstring& strComponentTag, CComponent** ppOut, void* pArg);

public:
	virtual CGameObject*		Clone(void* pArg) = 0;
	virtual void					Free() override;
};

NS_END