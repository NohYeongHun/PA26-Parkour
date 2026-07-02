#pragma once
#include "StaticObject.h"
#include"Client_Enum.h"

NS_BEGIN(Engine)
class CDeferredShader;
class CShader;
class CModel;
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CMapObject_Sonoro final : public CStaticObject
{
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
	explicit CMapObject_Sonoro(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMapObject_Sonoro(const CMapObject_Sonoro& Prototype);
	virtual ~CMapObject_Sonoro() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex) override;
	virtual		void			Render_Shadow() override;

	virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {};
	virtual		void			OnCollide_OnGoing(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {};

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}
	virtual		BoundingBox* Get_BoundingBox()override;
	void Change_Collision_Layer(_bool SonoroMode);

private:
	CDeferredShader* m_pShaderCom = { nullptr };
	CShader* m_pShadowShaderCom = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };
	class CGameSystem* m_pGameSystem = { nullptr };
	vector<CModel*>		m_pModelComArray;
	class CModel_Streaming* m_pModelCom;

	_uint						m_iShaderPassIndex = {};
	_bool*						m_IsRender = { nullptr };
	_bool*					 m_SonoroMode = { nullptr };
private:
	void						Ready_Component(void* pArg);

public:
	static		CMapObject_Sonoro* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END