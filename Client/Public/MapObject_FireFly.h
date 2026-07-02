#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CModel_Instance_FireFly;
NS_END

NS_BEGIN(Client)
class CMapObject_FireFly final: public CGameObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex;
		_float4x4 WorldMatrix;
		_uint iLevel;
	}MAP_LOAD;

private:
	explicit CMapObject_FireFly(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMapObject_FireFly(const CMapObject_FireFly& Prototype);
	virtual ~CMapObject_FireFly() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;

private:
	CShader* m_pShaderCom = { nullptr };
	CModel_Instance_FireFly* m_pModelCom = { nullptr };
	_uint						m_iShaderPassIndex = {};
	_float						m_fTotalTime = {};
private:
	void						Ready_Component(void* pArg);

public:
	static		CMapObject_FireFly* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END