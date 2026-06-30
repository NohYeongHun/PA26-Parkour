#pragma once
#include "GameObject.h"
NS_BEGIN(Editor)
class CEdit_Portal final: public CGameObject
{
private:
	CEdit_Portal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual~CEdit_Portal() = default;

public:
	virtual		HRESULT			Initialize_Prototype()override;
	virtual		HRESULT			Initialize_Clone(void* pArg)override;
	virtual		void			Priority_Update(_float fTimeDelta)override;
	virtual		void			Update(_float fTimeDelta)override;
	virtual		void			Late_Update(_float fTimeDelta)override;
	virtual		void			Render()override;


	void Ready_Components(void* pArg);
private:
	CShader* m_pShaderCom = { nullptr };
	class CModel* m_pModelCom = { nullptr };
	class CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	class CTexture* m_pDiffuseTextureCom = { nullptr };
	class CTexture* m_pMaskTextureCom = { nullptr };
	_uint m_iShaderPassIndex = {};
	_float m_fTotalTime = {};
	_float	m_vColor[4];
public:
	static 	CEdit_Portal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END