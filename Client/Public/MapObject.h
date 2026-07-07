#pragma once
#include "StaticObject.h"
#include "Client_Enum.h"

NS_BEGIN(Engine)
class CDeferredShader;
class CShader;
class CRigidbody;
NS_END

NS_BEGIN(Client)
class CMapObject final: public CStaticObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
		OBJECTTYPE eObjectType;
		PARKOUR_FLAG eParkourFlag;
		_uint iLevel = {};
		_float3 vBoundingPos;
		_float3 vBoundingExtends;
	}MAP_LOAD;

private:
	explicit CMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMapObject(const CMapObject& Prototype);
	virtual ~CMapObject() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex) override;
	virtual		void			Render_Shadow() override;
	virtual		void			Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix) override;

	virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold){};
	virtual		void			OnCollide_OnGoing(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {};

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}
	virtual		BoundingBox*	Get_BoundingBox()override;
	virtual		void			Set_RenderTime(_uint iLODIndex, _float m_fTotalPlayTime)override;

private:
	CDeferredShader*			m_pShaderCom = { nullptr };
	CShader*					m_pShadowShaderCom = { nullptr };
	CRigidbody*					m_pRigidbodyCom = { nullptr };
	class CModel_Streaming*		m_pModelCom = { nullptr } ;
	class CGameSystem*			m_pGameSystem = { nullptr };

	_uint						m_iShaderPassIndex = {};
	_bool						m_IsRender = { true };
	_bool						m_IsParkour = { false };

private:
	/* Jolt로 탐지될 정보 */
	CALLBACK_CLIENT m_CallBack = {}; 


#ifdef _DEBUG
private:
	_uint	m_iDebugShaderParkour = {};
#endif // _DEBUG

private:
	void						Ready_Components(const MAP_LOAD* pDesc);

private:
	void Bind_Resource();

public:
	static		CMapObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END