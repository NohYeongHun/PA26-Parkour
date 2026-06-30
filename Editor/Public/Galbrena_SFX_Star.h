#pragma once
#include "Edit_ScreenEffect.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect_Instance;
class CShader;
class CTexture;
NS_END

NS_BEGIN(Editor)

class CGalbrena_SFX_Star final : public CEdit_ScreenEffect
{
private:
	CGalbrena_SFX_Star(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGalbrena_SFX_Star(const CGalbrena_SFX_Star& Prototype);
	virtual ~CGalbrena_SFX_Star() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;
	virtual		void		Play() override;
	virtual		void		Stop() override;
	virtual		void		Reset() override;
private:
	CVIBuffer_Rect_Instance* m_pVIBuffer = { nullptr };
	CShader* m_pShader = { nullptr };
	CTexture* m_pTexture = { nullptr };

	_float2						m_vCenter = {};

	_float2						m_vEffectTime = {};
	_float						m_fCurrentTime = {};

	_float2						m_vSize = {};

	_float2						m_vScale = {};
	_float						m_fCurrentScale = {};

	_float						m_fRotate[2] = {};

	_float3						m_vColor = {};

private:
	HRESULT						Ready_Components();
	void						Update_Instance();
	VTXINSTANCE_RECT			Make_Instance(_float2 vCenter, _float fSizeX, _float fSizeY, _float fRotateZ);


public:
	static CGalbrena_SFX_Star* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void					Free() override;
};

NS_END