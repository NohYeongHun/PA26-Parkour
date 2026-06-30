#pragma once
#include "UIObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCustomFont final: public CUIObject
{
public:
	typedef struct tCustomFontDesc : public FONT_SINGLEDESC, UI_DESC
	{

	} CUSTOMFONT_DESC;

private:
	explicit		CCustomFont(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit		CCustomFont(const CCustomFont& Prototype);
	virtual			~CCustomFont() = default;

public:
	virtual HRESULT			Initialize_Prototype()					override;
	virtual HRESULT			Initialize_Clone(void* pArg)			override;
	virtual void			Priority_Update(_float fTimeDelta)		override;
	virtual void			Update(_float fTimeDelta)				override;
	virtual void			Late_Update(_float fTimeDelta)			override;
	virtual void			Render()								override;


private:
	FONT_SINGLEDESC			m_tSingleDesc = {};

	//_wstring				m_strFontTag			= {};
	//_wstring				m_strText				= {};
	//_float2					m_vScreenPos			= {};
	//_float					m_fScale				= {};
	//_float2					m_vLifeTime				= {};
	//_float4					m_vColor				= {};
	//_uint					m_iShaderFlag			= {};

				
public:
	static CCustomFont*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject*	Clone(void* pArg);
	virtual	void			Free()									override;
};


#pragma region old
//class CCustomFont final : public CBase
//{
//private:
//	explicit CCustomFont(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
//	virtual ~CCustomFont() = default;
//
//public:
//	HRESULT						Initialize(const _tchar* pFilePath);
//	HRESULT						Render(const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRadian, const _float2& vOrigin, const _float2& vScale);
//
//private:
//	ID3D11Device*				m_pDevice = { nullptr };
//	ID3D11DeviceContext*	m_pContext = { nullptr };
//
//	SpriteBatch*					m_pBatch = { nullptr };	// Font瑜?洹몃━湲??꾪븳 ?ш컖??踰꾪띁
//	SpriteFont*					m_pFont = { nullptr };		// Font
//
//public:
//	static		CCustomFont*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath);
//	virtual		void				Free() override;
//};  
#pragma endregion


NS_END