#pragma once
#include "Editor_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Mesh;
class CComputeShader;
NS_END

NS_BEGIN(Editor)

class CTrail_Mesh final : public CGameObject
{
public:
	typedef struct tagTrailMeshDesc : Engine::EFFECT_DESC
	{
		_wstring strTextureTag;
		_wstring strColorTextureTag;
		_wstring strDissolveTextureTag = {};
		_wstring strDistortionTextureTag = {};
		_wstring strVIBufferTag;

		_int	iShaderPass = 0;
		
		_float	fSweep = 0.f;
		_float	fSweepWitdh = 0.f;
		_float	fSoft = 0.3f;

		_int	iDirFlag = 0;
		_int	iMaskFlag = 0;

		_bool	IsDissolve = false;

		_bool	IsDistortion = false;
		_float	fDistortionWeight = 0.1f;

		_bool	IsLoop = false;
		
		_float	fColorSpeed = 1.f;
		_float	fMaskSpeed = 1.f;
		_float  fAlpha = 1.f;

		_float fColorGain = 0.f;
		_float fColorGamma = 1.f;

		_float3	vSize = { 1.f, 1.f, 1.f };
		_float3 vPos = { 0.f, 0.f, 0.f };
		_float3 vColor = { 0.f, 0.f, 0.f };
		_float2	vLifeTime = { 0.f, 10.f};
	}TRAILMESH_DESC;

private:
	CTrail_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrail_Mesh(const CTrail_Mesh& Prototype);
	virtual ~CTrail_Mesh() = default;

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
	void Update_Transform();

private:
	CShader*					m_pShaderCom = { nullptr };
	CTexture*					m_pTextureCom = { nullptr };
	CTexture*					m_pColorTextureCom = { nullptr };
	CTexture*					m_pDissolveTextureCom = { nullptr };
	CTexture*					m_pDistortionTextureCom = { nullptr };
	CVIBuffer_Mesh*				m_pVIBufferCom = { nullptr };

	_int						m_iShaderPass = 0;
		
	//셰이더에 전달할 값 연산용
	_float						m_fSweepSpeed = 0.f;

	//셰이더에 전달할 값
	_float						m_fSweep = 0.f;
	_float						m_fSweepWitdh = 0.f;
	_float						m_fSoft = 0.f;
	_float						m_fCurrentTime = 0.f;

	_float						m_fColorSpeed = 0.f;
	_float						m_fColorSweep = 0.f;
	_float						m_fColorGain = 0.f;
	_float						m_fColorGamma = 1.f;

	_float						m_fMaskSweep = 0.f;
	_float						m_fMaskSpeed = 3.f;
	_float						m_fAlpha = {};

	_int						m_iDirFalg = {};
	_int						m_iMaskFlag = {};

	_bool						m_IsDissolve = false;

	_bool						m_IsDistortion = false;
	_float						m_fDistortionWeight = {};

	_float3						m_vPos = {};
	_float3						m_vColor = {};
	_float2						m_vLifeTime = {};

	_float						m_fTime = 0.f;

	_bool						m_IsLoop = false;
	_bool						m_IsEnd = false;
	_bool						m_IsRoot = false;
	_float4x4					m_ComBindMatrix = {  };

	const _bool*				m_pActiveFlag = nullptr;
	const _float4x4*			m_pBoneMatrixPtr = nullptr;
	const _float4x4*			m_pObjectMatrixPtr = nullptr;
	_matrix						m_OffsetMatrix = {};

private:
	HRESULT Ready_Components(TRAILMESH_DESC& Desc);
	HRESULT Bind_ShaderResources();

public:
	static CTrail_Mesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END