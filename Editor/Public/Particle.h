#pragma once
#include "Editor_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Point_Instance;
class CComputeShader;
NS_END

NS_BEGIN(Editor)

class CParticle final : public CGameObject
{
public:
	typedef struct tagParticleDesc : Engine::EFFECT_DESC
	{
		_wstring strTextureTag;
		_wstring strVIBufferTag;

		_int	fShaderPass = 0;
		_float3	vSize = { 1.f, 1.f, 1.f };
		_float3 vPos = { 0.f, 0.f, 0.f };
		_float4 vColor = { 1.f, 1.f, 1.f, 1.f };
		_float2	vLifeTime = { 0.f, 10.f};

		_int	iMaskFlag = 0;

		_bool	IsPivot = false;
		_bool	IsLoop = false;

		_bool	IsSprite = false;
		_int    iRows = 0;
		_int	iCols = 0;
	}PARTICLE_DESC;

private:
	CParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticle(const CParticle& Prototype);
	virtual ~CParticle() = default;

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
	void Default_Transform(_fmatrix WorldMatrix);
	void Bind_CS_SpriteInfo();
	void Update_Root_Transform();

private:
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CVIBuffer_Point_Instance*	m_pVIBufferCom = { nullptr };
	CComputeShader*				m_pComputeShader = { nullptr };

	PARTICLE_DESC				m_tDesc = {};

	_int						m_iShaderPass = 0;
	_float3						m_vPos = {};
	_float4						m_vColor = {};
	_float2						m_vLifeTime = {};

	_bool						m_IsSprite = false;
	_int						m_iRow = {};
	_int						m_iCol = {};

	_int						m_iMaskFlag = 0;

	_bool						m_IsLoop = false;
	_bool						m_IsPivot = false;
	_bool						m_IsRoot = false;
	_float4x4					m_ComBindMatrix = {  };

	const _float4x4*			m_pBoneMatrixPtr = nullptr;
	const _float4x4*			m_pObjectMatrixPtr = nullptr;
	_matrix						m_OffsetMatrix = {};

private:
	HRESULT Ready_Components(PARTICLE_DESC& Desc);
	HRESULT Bind_ShaderResources();

public:
	static CParticle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END