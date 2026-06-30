#pragma once
#include "Editor_Define.h"
#include "GameObject.h"


NS_BEGIN(Editor)

class CEffect_Radial final : public CGameObject
{
public:
	typedef struct tagRadialDesc : Engine::EFFECT_DESC
	{
		_float2 Center = { 0.5f, 0.5f };
		_float2 DistanceRange = { 0.f, 0.f };
		_float IntensityRange = 0.f;

		_float fLifeTime = {};
		_float fRadialTime = 0.5f;

		_bool  PositionFlag = false;
	}RADIAL_DESC;

private:
	CEffect_Radial(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect_Radial(const CEffect_Radial& Prototype);
	virtual ~CEffect_Radial() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize_Clone(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	
public:
	virtual		void	Reset(const _fmatrix& WorldMatrix, void* pArg) override;
	

private:
	RADIAL_DESC					m_tDesc = {};
	_float						m_fCurrentTime = 0.f;
	_vector						m_vObjectPos = {};
	

public:
	static CEffect_Radial* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END