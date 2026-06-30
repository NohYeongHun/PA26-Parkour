#pragma once
#include "Editor_Define.h"
#include "GameObject.h"


NS_BEGIN(Editor)

class CEffect_Light final : public CGameObject
{
public:
	typedef struct tagLightDesc : Engine::EFFECT_DESC
	{
		_wstring wstrLightTag = {};
		_float4 vColor = { 1.f, 1.f, 1.f, 1.f };
		_float2 vLifeTime = {0.f , 5.f};
		_float	fSpeed = 1.f;
		_float2 vRange = { 0.f, 1.f };
		_float fAmbient = 0.1f;
	}LIGHT_DESC;

private:
	CEffect_Light(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect_Light(const CEffect_Light& Prototype);
	virtual ~CEffect_Light() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize_Clone(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	
public:
	virtual		void	Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	void Update_LightDesc(_float fTimeDelta);

private:
	LIGHT_DESC					m_tDesc = {};

	Engine::LIGHT_DESC			m_tLightDesc = {};

	_wstring					m_wstrLightTag = {};
	_float4						m_vColor = {};
	_float2						m_vLifeTime = {};
	_float						m_fSpeed = {};
	_float2						m_vRange = {};
	_float						m_fAmbient = {};


public:
	static CEffect_Light* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END