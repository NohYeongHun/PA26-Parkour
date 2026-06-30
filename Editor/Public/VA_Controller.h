#pragma once
#include "Base.h"
#include "TestVA.h"

NS_BEGIN(Editor)

class CVA_Controller final : public CBase
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

	typedef struct MeshDatTag {
		_char szName[MAX_PATH] = {};
		_tchar strMeshTag[MAX_PATH] = {};
		_char szDatPath[MAX_PATH] = {};
	}MESH_TAG;

private:
	explicit CVA_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CVA_Controller() = default;

#pragma region 기본
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	void Load_AllMaskTextureFromFolder(const _string& strFolderPath);
	void Load_AllMeshDatFromFolder(const _string& strFolderPath);
	void Load_AllColorTextureFormFolder(const _string& strFolderPath);

public:
	void VA_Tab();

	void VA_Base_Tab(CTestVA::VA_DESC& tEffectMeshDesc, _bool& IsCreate);

public:
	void Set_VATag(const _char* szEffectMeshTag);
	void UpdateSelected_VAFormTag(_wstring FMMeshTag);

	CTestVA::VA_DESC* Get_VADesc(_wstring& EffectMeshTag);
	void Set_VADesc(_wstring& VATag, CTestVA::VA_DESC& TrailDesc);

	void Remove_Desc(const _wstring& DescTag);

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

	vector<MESH_TAG>											m_MeshVBTag = {};
	_int														m_iSelectedMeshVBTag = -1;
	_bool														m_bMeshVBTag = false;

	_char														m_VATag[MAX_PATH] = {};
	_bool														m_bTagFlag = false;

	_bool														m_IsRoot = false;


	map<const _wstring, CTestVA::VA_DESC>						m_tVADesc = {};

	_bool														m_bSelectedMesh = false;
	CTestVA::VA_DESC*											m_pSelectedVADesc = { nullptr };

public:
	static CVA_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END