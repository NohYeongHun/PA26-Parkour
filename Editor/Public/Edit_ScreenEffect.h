#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CVIBuffer_Rect;
class CShader;
NS_END

NS_BEGIN(Editor)

class CEdit_ScreenEffect abstract : public CGameObject
{
protected:
	CEdit_ScreenEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_ScreenEffect(const CEdit_ScreenEffect& Prototype);
	virtual ~CEdit_ScreenEffect() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;

	virtual		void		Play() {};
	virtual		void		Stop() {};
	virtual		void		Reset() {}

protected:
	_bool					m_IsPlay = {};
	_float4x4				m_ViewMatrix = {};
	_float4x4				m_ProjMatrix = {};

	_float2					m_vWinSize = {};

protected:
	void					Setting_Scale(_float fSizeX, _float fSizeY);
	void					Setting_Pos(_float fPosX, _float fPosY);

private:
	HRESULT					Ready_Components();
public:
	virtual CGameObject*	Clone(void* pArg) PURE;
	virtual void			Free() override;
};

NS_END