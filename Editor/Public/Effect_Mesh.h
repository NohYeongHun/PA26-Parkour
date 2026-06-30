#pragma once
#include "Editor_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_FXMesh_Instance;
class CComputeShader;
NS_END

NS_BEGIN(Editor)

class CEffect_Mesh final : public CGameObject
{
public:
	typedef struct tagEffectMeshDesc : Engine::EFFECT_DESC
	{
		_wstring strTextureTag;
		//_wstring strShaderTag;
		_wstring strVIBufferTag;

		_int	fShaderPass = 0.f;
		_float3	vSize = { 1.f, 1.f, 1.f };
		_float3 vPos = { 0.f, 0.f, 0.f };
		_float3 vColor = { 0.f, 0.f, 0.f };
		_float2	vLifeTime = { 0.f, 10.f};
	}EFFECTMESH_DESC;

private:
	CEffect_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect_Mesh(const CEffect_Mesh& Prototype);
	virtual ~CEffect_Mesh() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize_Clone(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual void Render();

public:
	virtual		void	Reset(const _fmatrix& WorldMatrix, void* pArg) override;

private:
	void Root_Transform(_fmatrix WorldMatrix);

private:
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CVIBuffer_FXMesh_Instance*				m_pVIBufferCom = { nullptr };
	CComputeShader*				m_pComputeShaderCom = { nullptr };

	_float						m_fShaderPass = 0;
	_float3						m_vPos = {};
	_float3						m_vColor = {};
	_float2						m_vLifeTime = {};

	_bool						m_IsRoot = false;
	_float4x4					m_ComBindMatrix = {  };

private:
	HRESULT Ready_Components(EFFECTMESH_DESC& Desc);
	HRESULT Bind_ShaderResources();

public:
	static CEffect_Mesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END