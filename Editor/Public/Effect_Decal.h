#pragma once
#include "Editor_Define.h"
#include "GameObject.h"


NS_BEGIN(Editor)

class CEffect_Decal final : public CGameObject
{
public:
	typedef struct tagDecalDesc : Engine::EFFECT_DESC
	{
		_wstring wstrDecalTag = {};
		_float4 vColor = { 1.f, 1.f, 1.f, 1.f };
		_float LifeTime = {};
		_float fBlendTime = 0.f;
		_float fEmissiveIntensity = 0.f;
	}DECAL_DESC;

private:
	CEffect_Decal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect_Decal(const CEffect_Decal& Prototype);
	virtual ~CEffect_Decal() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize_Clone(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	
public:
	virtual		void	Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	void Default_Transform(_fmatrix SpawnMatrix, _fmatrix OffsetMatrix);
	

private:
	DECAL_DESC					m_tDesc = {};

	_wstring					m_wstrMyTag = {};
	_float4						m_vColor = {};
	_float						m_LifeTime = {};
	_float						m_fBlendTime = {};
	_float						m_fEmissiveIntensity = {};

	_matrix						m_ComBindMatrix = {};

public:
	static CEffect_Decal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END