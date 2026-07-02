#pragma once
#include"StaticObject.h"

NS_BEGIN(Engine)
class CDeferredShader;
class CShader;
class CModel;
class CRigidbody;
class CModel_Instance;
NS_END

NS_BEGIN(Client)
class CMapObject_Instance final: public CStaticObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex;
		_float4x4* InstanceWorldMatrix = { nullptr };
		_uint iNumInstance;
		_float4x4 WorldMatrix;
		_uint iSaveIndex;
		_uint iLevel;
		_float4 vDiffuseColor;
		_float3 vBoundingPos;
		_float3 vBoundingExtends;
		INSTANCETYPE eInstanceType = { INSTANCETYPE::DEFAULT };
	}MAP_LOAD;

private:
	explicit CMapObject_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMapObject_Instance(const CMapObject_Instance& Prototype);
	virtual ~CMapObject_Instance() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void			Render_Shadow() override;

	virtual		void			OnCollide_Enter(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {};
	virtual		void			OnCollide_OnGoing(_uint iLayer, CGameObject* pOther, const ContactManifold& Manifold) {};

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg) {}

private:
	CShader* m_pShaderCom = { nullptr };
	CShader* m_pShadowShaderCom = { nullptr };
	vector<CModel_Instance*>		m_pModelComArray;
	CModel_Instance* m_pModelCom = { nullptr };
	_uint						m_iShaderPassIndex = {};
	_float4						m_vDiffuseColor = {};
	_float						m_fTotalTime = {};
	class CGameSystem*			m_pGameSystem = { nullptr };
	INSTANCETYPE m_eInstanceType = { INSTANCETYPE::DEFAULT };
	_bool*						m_bSonoroMode = { nullptr };
	_bool						m_TypeMode = { false };
private:
	void						Ready_Component(void* pArg);

public:
	static		CMapObject_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END