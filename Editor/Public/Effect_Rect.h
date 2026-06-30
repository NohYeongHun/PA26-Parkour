#pragma once
#include "Editor_Define.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Point;
NS_END

NS_BEGIN(Editor)

class CEffect_Rect final : public CGameObject
{
public:
	typedef struct tagEffectRectDesc : Engine::EFFECT_DESC
	{
		_wstring strTextureTag;
		//_wstring strVIBufferTag;

		_float	fSweepSpeed = 1.f;
		_float	fSoft = 0.3f;

		_float	fXSize = 1.f;
		_float	fYSize = 1.f;

		_int	iMaskFlag = 0;
		_int	iColorFlag = 0;
		_int	iShaderPass = 0;

		_bool	IsLoop = false;
		_bool	IsSprite = false;
		_int    iRows = 0;
		_int	iCols = 0;

		_float3 vPos = { 0.f, 0.f, 0.f };
		_float4 vColor = { 1.f, 1.f, 1.f, 1.f };
		_float2	vLifeTime = { 0.f, 10.f };

	}FXRECT_DESC;

private:
	CEffect_Rect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect_Rect(const CEffect_Rect& Prototype);
	virtual ~CEffect_Rect() = default;

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
	void Sprite_Update(_float fTimeDelta);
	void Update_Root_Transform();


private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Point* m_pVIBufferCom = { nullptr };

	//FXRECT_DESC					m_tDesc = {};

	_int						m_iShaderPass = 0;
	_int						m_iMaskFlag = 0;
	_int						m_iColorFlag = 0;

	_float3						m_vPos = {};
	_float4						m_vColor = {};
	_float2						m_vLifeTime = {};

	_float						m_fXSize = {};
	_float						m_fYSize = {};

	_float						m_fSweep = 0.f;
	_float						m_fSweepSpeed = {};
	_float						m_fSoft = {};

	_bool						m_IsSprite = false;
	_int						m_iRow = {};
	_int						m_iCol = {};
	_float						m_fPhase = 0.f;

	_bool						m_IsLoop = false;
	_bool						m_IsRoot = false;
	_float4x4					m_ComBindMatrix = {  };

	const _float4x4*			m_pBoneMatrixPtr = nullptr;
	const _float4x4*			m_pObjectMatrixPtr = nullptr;
	_matrix						m_OffsetMatrix = {};


private:
	HRESULT Ready_Components(FXRECT_DESC& Desc);
	HRESULT Bind_ShaderResources();

public:
	static CEffect_Rect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END