#pragma once
#include "Base.h"
#include "Spectrum.h"

NS_BEGIN(Editor)

class CSpectrum_Controller final : public CBase
{
public:
	typedef struct MaskTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;
	}MASK_TEXTURE;

	typedef struct ColorTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;
	}COLOR_TEXTURE;

private:
	explicit CSpectrum_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CSpectrum_Controller() = default;

#pragma region 기본
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	//void Texture_Loading(const char* TextureName, const _tchar* pFilePath);
	void Load_AllTextureFromFolder(const _string& strFolderPath);
	void Load_AllColorTextureFormFolder(const _string& strFolderPath);

public:
	void Spectrum_Tab();
	void Spectrum_Info_Tab();
	void Spectrum_Base_Tab();

public:
	void Set_SpectrumTag(const _char* szEffectSpectrumTag);

	void UpdateSelected_SpectrumFormTag(_wstring SpectrumTag);

	void UpdateSelected_SpectrumFromIndex();

	CSpectrum::SPECTRUM_DESC* Get_SpectrumDesc(_wstring& EffectSpectrumTag);
	void Set_SpectrumDesc(_wstring& SpectrumTag, CSpectrum::SPECTRUM_DESC& SpectrumDesc);
	void Set_VBSpectrumDesc(_wstring& SpectrumTag, CVIBuffer_Spectrum::VB_SPECTRUM_DESC& SpectrumVBDesc);

	void Remove_Desc(const _wstring& DescTag);
	
	void Save_SelectedSpectrum_To_Json();

	void Spectrum_VB_To_Json(json& SpectrumVBJson);
	void Spectrum_OB_To_Json(json& SpectrumJson);

private:
	ID3D11Device*												m_pDevice = { nullptr };
	ID3D11DeviceContext*										m_pContext = { nullptr };
	class CGameInstance*										m_pGameInstance = { nullptr };

	vector<MASK_TEXTURE>										m_Textures = {};
	_int														m_iSelectedTexture = -1;
	_bool														m_TexturPopOpend = false;

	vector<COLOR_TEXTURE>										m_ColorTextures = {};
	_int														m_iSelectedColor = -1;
	_bool														m_ColorTexturePopOpend = false;

	_char														m_SpectrumTag[MAX_PATH] = {};
	_bool														m_bTagFlag = false;
	_bool														m_IsBaseFlag = true;

	_bool														m_IsRoot = false;

	map<const _wstring, class CSpectrum*>						m_Sspectrums;
	_int														m_iSelectedSpectrum = -1;
	class CSpectrum*											m_pSelectedSpecturm = nullptr;

	map<const _wstring, CSpectrum::SPECTRUM_DESC>				m_tSpectrumDesc = {};
	map<const _wstring, CVIBuffer_Spectrum::VB_SPECTRUM_DESC>	m_tVBDesc = {};

	_bool														m_bSelectedSpectrum = false;
	CSpectrum::SPECTRUM_DESC*									m_pSelectedSpectrumDesc = { nullptr };
	CVIBuffer_Spectrum::VB_SPECTRUM_DESC*						m_pSelectedSpectrumVBDesc = { nullptr };


public:
	static CSpectrum_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END