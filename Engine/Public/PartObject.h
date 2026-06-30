#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CPartObject abstract : public CGameObject
{
public:
	typedef struct tagPartObjectDesc : public CGameObject::GAMEOBJECT_DESC {
		class CTransform* pParentTransform = {nullptr};
	} PART_DESC;

protected:
	explicit CPartObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CPartObject(const CPartObject& Prototype);
	virtual ~CPartObject() = default;

public:
	const _float4x4*					Get_CombinedMatrix() { return &m_CombinedMatrix; }

public:
	virtual		HRESULT				Initialize_Prototype();
	virtual		HRESULT				Initialize_Clone(void* pArg);
	virtual		void					Priority_Update(_float fTimeDelta);
	virtual		void					Update(_float fTimeDelta);
	virtual		void					Late_Update(_float fTimeDelta);
	virtual		void					Render();

protected:
	class CTransform*				m_pParentTransform = { nullptr };

	_float4x4							m_CombinedMatrix = {};

public:
	virtual		CGameObject*		Clone(void* pArg) = 0;
	virtual		void					Free() override;
};

NS_END