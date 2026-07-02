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
class CMapObject_NonSonoro final : public CStaticObject
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
	explicit CMapObject_NonSonoro(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMapObject_NonSonoro(const CMapObject_NonSonoro& Prototype);
	virtual ~CMapObject_NonSonoro() = default;

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
	void Compute_DelayTime(_float4 vCamPos);

	void Turn_Sonoro(_fvector vUpSpeed, _float fTriggerdTime);
	void ReturnPos();
	void Change_Collision_Layer(_bool SonoroMode);
private:
	CDeferredShader* m_pShaderCom = { nullptr };
	CShader* m_pShadowShaderCom = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };
	vector<CModel*>		m_pModelComArray;
	class CModel_Streaming* m_pModelCom;

	class CGameSystem* m_pGameSystem = { nullptr };

	_uint						m_iShaderPassIndex = {};
	_float						m_fDlayTime = {};
	_float4x4					m_DefaultMatrix = {};
	_bool*						m_IsRender = { nullptr };
	_bool*						m_SonoroMode = { nullptr };

	OBJECTTYPE					m_eObjectType = { OBJECTTYPE::END };
private:
	void						Ready_Component(void* pArg);

public:
	static		CMapObject_NonSonoro* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END