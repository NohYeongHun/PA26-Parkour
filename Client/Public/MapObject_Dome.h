#pragma once
#include "StaticObject.h"
#include"Client_Enum.h"

NS_BEGIN(Engine)
class CRigidbody;
class CShader;
class CModel_Streaming;
class CDeferredShader;
NS_END

NS_BEGIN(Client)
class CMapObject_Dome final: public CStaticObject
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
	explicit CMapObject_Dome(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMapObject_Dome(const CMapObject_Dome& Prototype);
	virtual ~CMapObject_Dome() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render(ID3D11DeviceContext* pDeferredContext, _uint iIndex) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;
	virtual		void			Render_EnvMap(_float4 vCenter, _float4x4 ViewMatrix, _float4x4 ProjMatrix) override;

	virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {};
	virtual		void			OnCollide_OnGoing(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {};

	virtual		BoundingBox* Get_BoundingBox()override;


	void Change_MaxAlpha(_uint iPhaze) { m_iPhaze = iPhaze; }
	void Start_Dissolve(_bool B) { m_IsDissolveStart = B; }

private:
	CShader* m_pShaderCom = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };
	class CModel_Streaming* m_pModelCom = { nullptr };
	class CGameSystem* m_pGameSystem = { nullptr };
	_uint					m_iShaderPassIndex = {};
	_bool					m_IsRender = { true };
	_float					m_fTotalTime = {};
	_uint					m_iPhaze = {};
	_float					m_fAlpha = {};
	_float					m_fMaxAlpha = {};
	_bool					m_IsDissolveStart = { false };
	_float					m_fDissolveTime = {};
private:
	virtual		void						Ready_Component(void* pArg);

public:
	static		CMapObject_Dome* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};


NS_END