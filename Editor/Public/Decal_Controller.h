#pragma once
#include "Base.h"
#include "Effect_Decal.h"

NS_BEGIN(Editor)

class CDecal_Controller final : public CBase
{
public:
	//typedef struct DecalTextureTag {
	//	_char szName[MAX_PATH] = {};
	//	_tchar strTextureTag[MAX_PATH] = {};
	//	class CTexture* pTexture;

	//}DECAL_TEXTURE;

	//DIFFUSE, NORMAL, MASK,

	typedef struct DecalDiffuseTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		_string strTexturePath = {};
		class CTexture* pTexture;
	}DECAL_DIFFUSE;

	typedef struct DecalNormalTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		_string strTexturePath = {};
		class CTexture* pTexture;
	}DECAL_NORMAL;

	typedef struct DecalMaskTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		_string strTexturePath = {};
		class CTexture* pTexture;
	}DECAL_MASK;

	typedef struct DecalTextureDesc {
		_wstring TexturePath = {};
		TEXTURETYPE eType = TEXTURETYPE::END;
	}TEXTURE_DESC;

	typedef struct DecalData {
		_wstring wstrDecalDataTag = {};
		_int	iTextureCount = {};
		_bool	Emissive = false;
		_float3	vEmissiveLuminance;
		vector<TEXTURE_DESC> TextureDesc;
	}DECAL_DATADESC;


private:
	explicit CDecal_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDecal_Controller() = default;

#pragma region
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	//void Texture_Loading(const char* TextureName, const _tchar* pFilePath);

	void Load_AllJsonDecalDataFromFolder(const _string& strFolderPath);
	void Load_DecalData_FromJson(const _string& strFilePath);
	
	void Load_AllDiffuseTextureFromFolder(const _string& strFolderPath);
	void Load_AllNormalTextureFromFolder(const _string& strFolderPath);
	void Load_AllMaskTextureFromFolder(const _string& strFolderPath);

public:
	void Decal_Tab();

	void Decal_Base_Tab(CEffect_Decal::DECAL_DESC& tEffectDecalDesc, _bool& IsCreate);
	void DecalData_Base_Tab();

public:
	void UpdateSelected_DecalFormTag(_wstring DecalTag);
	
	CEffect_Decal::DECAL_DESC* Get_DecalDesc(_wstring& DecalTag);
	void	Set_DecalDesc(_wstring& DecalTag, CEffect_Decal::DECAL_DESC& DecalDesc);

	void Set_DecalTag(const _char* szDecalTag);

	void Remove_Desc(const _wstring& DescTag);

public:
	void DecalData_To_Json(DECAL_DATADESC* pDesc);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	

	//매니저에 추가할 데칼 정보
	vector<DECAL_DIFFUSE>										m_DiffuseTextures = {};
	_int														m_iSelectedDiffuseTexture = -1;
	_bool														m_TexturDiffusePopOpend = false;

	vector<DECAL_NORMAL>										m_NormalTextures = {};
	_int														m_iSelectedNormalTexture = -1;
	_bool														m_TexturNormalPopOpend = false;

	vector<DECAL_MASK>											m_MaskTextures = {};
	_int														m_iSelectedMaskTexture = -1;
	_bool														m_TexturMaskPopOpend = false;

	_char														m_DecalDataTag[MAX_PATH] = {};
	_bool														m_bDataTagFlag = false;

	vector<_string>												m_DecalData;
	_int														m_iSelectedDacalData = -1;

	//클래스로 만들 데칼이 가질 정보.
	//vector<DECAL_TEXTURE>										m_Textures = {};
	//_int														m_iSelectedTexture = -1;
	//_bool														m_TexturPopOpend = false;

	_bool														m_IsDecalDataFlag = false;

	_char														m_DecalTag[MAX_PATH] = {};
	_bool														m_bTagFlag = false;
	_bool														m_bDataFlag = false;

	map<const _wstring, CEffect_Decal::DECAL_DESC>				m_tDecalDesc = {};

	_int														m_iSelectedDecal = 0;
	_bool														m_bSelectedDecal = false;
	CEffect_Decal::DECAL_DESC*									m_pSelectedDecalDesc = { nullptr };

	_float														m_fColor[4] = {1.f, 1.f, 1.f, 1.f};		

	//Decal 베이스 정보
	_bool														m_bMaskEmissive = false;
	_bool														m_bDiffuseEmissive = false;
	_float3														m_EmissiveLuminance = {};

public:
	static CDecal_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END