#pragma once
#include "GameObject.h"

NS_BEGIN(Editor)
class CEdit_LightObject final : public CGameObject
{
public:
	typedef struct tagMapload {
		_float4 vWorldPos = {};
		const LIGHT_DESC* CopyDesc = {};
		_bool IsCopy = { false };
		_float4 vLightDiffuse = {};
		_float4 vLightAmbient = {};
		_float4 vLightSpec = {};
	}MAP_LOAD;

	union MyFloat4 {
		_vector Vec = XMVectorSet(1.f, 1.f, 1.f, 1.f);
		_float4 float_4;
		_float arr[4];
	};

private:
	explicit CEdit_LightObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEdit_LightObject(const CEdit_LightObject& Prototype);
	virtual ~CEdit_LightObject() = default;

public:
	virtual		HRESULT		Initialize_Prototype()override;
	virtual		HRESULT		Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	virtual		void			Render()override;
	virtual		void			Render_Shadow()override;
	void Ready_Component(void* pArg);
	void Set_ImGuiOption();
	LIGHT_DESC*					Save_Light() { return m_LightDesc; }
	void Copy();
private:
	LIGHT_DESC* m_LightDesc = {};
	class CMap_Interface* m_pMapInterface = { nullptr };
	MyFloat4 m_vLightDiffuse = {};
	MyFloat4 m_vLightAmbient= {};
	MyFloat4 m_vLightSpec= {};
	_uint m_iLightIndex = {};
public:
	static _uint g_iLightIndex;
public:
	static CEdit_LightObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject* Clone(void* pArg)override;
	virtual void Free()override;
};

NS_END