#pragma once
#include "Base.h"
#include "Effect_Rect.h"

NS_BEGIN(Editor)

class CRect_Controller final : public CBase
{
public:
	typedef struct RectTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;

	}RECT_TEXTURE;

private:
	explicit CRect_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRect_Controller() = default;

#pragma region
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	//void Texture_Loading(const char* TextureName, const _tchar* pFilePath);
	void Load_AllTextureFromFolder(const _string& strFolderPath);

public:
	void Rect_Tab();

	void Rect_Base_Tab(CEffect_Rect::FXRECT_DESC& tRectDesc, _bool& IsCreate);

public:
	void UpdateSelected_RectFormTag(_wstring RectTag);
	
	CEffect_Rect::FXRECT_DESC* Get_RectDesc(_wstring& RectTag);
	void	Set_RectDesc(_wstring& RectTag, CEffect_Rect::FXRECT_DESC& RectDesc);

	void Set_RectTag(const _char* szRectTag);

	void Remove_Desc(const _wstring& DescTag);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	vector<RECT_TEXTURE>										m_Textures = {};
	_int														m_iSelectedTexture = -1;
	_bool														m_TexturPopOpend = false;

	_char														m_RectTag[MAX_PATH] = {};
	_bool														m_bTagFlag = false;

	map<const _wstring, CEffect_Rect::FXRECT_DESC>				m_tRectDesc = {};

	_int														m_iSelectedRect = 0;
	_bool														m_bSelectedRect = false;
	CEffect_Rect::FXRECT_DESC*									m_pSelectedRectDesc = { nullptr };

	_float														m_fColor[4] = {1.f, 1.f, 1.f, 1.f};		

public:
	static CRect_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END