#pragma once
#include "StaticObject.h"

NS_BEGIN(Client)

class CMapObject_Water : public CStaticObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
		OBJECTTYPE eObjectType;
		_uint iLevel = {};
		_float3 vBoundingPos;
		_float3 vBoundingExtends;
	}MAP_LOAD;

private:
	explicit CMapObject_Water(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMapObject_Water(const CMapObject_Water& Prototype);
	virtual ~CMapObject_Water() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;
	virtual		void			Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix) override;

	virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {};
	virtual		void			OnCollide_OnGoing(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {};

	virtual		BoundingBox* Get_BoundingBox()override;
	virtual		void					Set_RenderTime(_uint iLODIndex, _float m_fTotalPlayTime)override;

private:
	CShader*				m_pShaderCom = { nullptr };
	CShader*				m_pShadowShaderCom = { nullptr };
	CRigidbody*				m_pRigidbodyCom = { nullptr };
	vector<CModel*>			m_pModelComArray;
	class CModel_Streaming* m_pModelCom;

	_uint					m_iShaderPassIndex = {};
	_bool					m_IsRender = { true };

	_float					m_fTime = {};
	_float					m_fAnimTime = {};

	_int					m_iTextureIndex = {};

private:
	virtual		void						Ready_Component(void* pArg);

public:
	static		CMapObject_Water* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END