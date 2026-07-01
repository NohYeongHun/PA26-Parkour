#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)
class CTexture;
class CVIBuffer_Rect;
class CShader;
NS_END

NS_BEGIN(Client)

class CMouse final : public CGameObject
{
private:
	explicit CMouse(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMouse(const CMouse& Prototype);
	virtual ~CMouse() = default;

public:
	void						Set_MouseFix(_bool isFix) { m_isMouseFix = isFix; }
	_bool						IsFix() { return m_isMouseFix; }

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void			Priority_Update(_float fTimeDelta) override;
	virtual		void			Update(_float fTimeDelta) override;
	virtual		void			Late_Update(_float fTimeDelta) override;
	virtual		void			Render() override;

private:
	class CGameSystem*		m_pGameSystem = { nullptr };
	_bool							m_isMouseFix = { true };
	_int							m_iCursorPosX{}, m_iCursorPosY{};

	_float4x4						m_WorldMatrix = {};
	_float4x4						m_ViewMatrix = {};
	_float4x4						m_ProjMatrix = {};

	CTexture*					m_pTextureCom = { nullptr };
	CVIBuffer_Rect*			m_pVIBufferCom = { nullptr };
	CShader*						m_pShaderCom = { nullptr };

private:
	void						Ready_Component();

	void						Compute_XY();

public:
	static		CMouse*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject*		Clone(void* pArg) override;
	virtual		void					Free() override;
};

NS_END