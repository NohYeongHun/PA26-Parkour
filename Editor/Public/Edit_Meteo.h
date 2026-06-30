#pragma once
#include "StaticObject.h"
NS_BEGIN(Engine)

NS_END

NS_BEGIN(Editor)
class CEdit_Meteo final: public CStaticObject
{
public:
	union MyFloat4 {
		_vector Vec;
		_float4 float4;
		_float arr[4];
	};	

	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4 WorldMatrix = {};
		_uint iLevel = ENUM_CLASS(LEVEL::MAP);
		_float4 vSourPos = _float4(0.f, 0.f, 0.f, 1.f);
		_float4 vDestPos = _float4(0.f, 0.f, 0.f, 1.f);
		_float fDuration = {};
		_float fArchY = {};
		_uint TriggerIndex = {};
		_int TriggerActiveIndex = { -1 };
		OBJECTTYPE eObjectType;
		_float3 vBoundingPos;
		_float3 vBoundingExtends;
	}MAP_LOAD;

private:
	CEdit_Meteo(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_Meteo(const CEdit_Meteo& Prototype);
	virtual ~CEdit_Meteo() = default;

public:
	virtual		HRESULT		Initialize_Prototype()override;
	virtual		HRESULT		Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	virtual		void			Render();

	void LerpPos(_float fTimeDelta);
	virtual void Set_ImGuiOption();
	void Set_ShaderPass(_uint i) { m_iShaderPassIndex = i; }
private:
	void Ready_Components(void* pArg);

private:
	class CMap_Interface* m_pMapInterface = { nullptr };
	MyFloat4 m_vSourPos = {};
	MyFloat4 m_vDestPos = {};
	_float m_fFall = {};
	_float m_fDuration = {};

	_float m_farchY = {};
	_bool m_Test = { false };
private:
	_uint m_iTriggerIndex = {};
	_int m_iTriggerActiveIndex = {};

private:
	CModel_Streaming* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };

	vector<CModel*> m_pModelComArray;
	CRigidbody* m_pRigidbodyCom = { nullptr };

protected:
	_char m_ModelName[MAX_PATH];

	_uint m_iShaderPassIndex = {};
	_uint m_iNumObject = {};


	_uint m_iLevel = {};
	OBJECTTYPE m_eObjectType = { OBJECTTYPE::END };

	_bool m_IsRender = { true };
public:
	static CEdit_Meteo* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};
NS_END