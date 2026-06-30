#pragma once
#include "Base.h"
#include "Trail_Mesh.h"

NS_BEGIN(Editor)

class CTrailMesh_Controller final : public CBase
{
public:
	typedef struct MeshTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;
	}MESH_TEXTURE;

	typedef struct MeshDissolveTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;
	}DISSOLVE_TEXTURE;

	typedef struct MeshDistortionTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;
	}DISTORTION_TEXTURE;

	typedef struct ColorTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;
	}COLOR_TEXTURE;

	typedef struct MeshDatTag {
		_char szName[MAX_PATH] = {};
		_tchar strMeshTag[MAX_PATH] = {};
		_char szDatPath[MAX_PATH] = {};
		//매쉬는 미리보기 어떻게 못하나 ?
	}MESH_TAG;

private:
	explicit CTrailMesh_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CTrailMesh_Controller() = default;

#pragma region 기본
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	//void Texture_Loading(const char* TextureName, const _tchar* pFilePath);
	void Load_AllTextureFromFolder(const _string& strFolderPath);
	void Load_AllMeshDatFromFolder(const _string& strFolderPath);
	void Load_AllColorTextureFormFolder(const _string& strFolderPath);
	void Load_AllDissolveTextureFromFolder(const _string& strFolderPath);
	void Load_AllDistortionTextureFromFolder(const _string& strFolderPath);

public:
	void TrailMesh_Tab();

	void TrailMesh_Base_Tab(CTrail_Mesh::TRAILMESH_DESC& tEffectMeshDesc, _bool& IsCreate);

public:
	void Set_TrailMeshTag(const _char* szEffectMeshTag);
	void UpdateSelected_TrailMeshFormTag(_wstring FMMeshTag);

	CTrail_Mesh::TRAILMESH_DESC* Get_TrailMeshDesc(_wstring& EffectMeshTag);
	void Set_TrailMeshDesc(_wstring& TrailMeshTag, CTrail_Mesh::TRAILMESH_DESC& TrailDesc);

	void Remove_Desc(const _wstring& DescTag);

private:
	ID3D11Device*												m_pDevice = { nullptr };
	ID3D11DeviceContext*										m_pContext = { nullptr };
	class CGameInstance*										m_pGameInstance = { nullptr };

	vector<MESH_TEXTURE>										m_Textures = {};
	_int														m_iSelectedTexture = -1;
	_bool														m_TexturPopOpend = false;

	vector<DISSOLVE_TEXTURE>									m_DissolveTextures = {};
	_int														m_iSelectedDissovle = -1;
	_bool														m_DissolveTexturPopOpend = false;

	vector<DISTORTION_TEXTURE>									m_DistortionTextures = {};
	_int														m_iSelectedDistortion = -1;
	_bool														m_DistortionTexturPopOpend = false;

	vector<COLOR_TEXTURE>										m_ColorTextures = {};
	_int														m_iSelectedColor = -1;
	_bool														m_ColorTexturePopOpend = false;

	vector<MESH_TAG>											m_MeshVBTag = {};
	_int														m_iSelectedMeshVBTag = -1;
	_bool														m_bMeshVBTag = false;

	_char														m_TrailMeshTag[MAX_PATH] = {};
	_bool														m_bTagFlag = false;

	_bool														m_IsRoot = false;


	map<const _wstring, CTrail_Mesh::TRAILMESH_DESC>			m_tTrailMeshDesc = {};

	_bool														m_bSelectedMesh = false;
	CTrail_Mesh::TRAILMESH_DESC*								m_pSelectedTrailMeshDesc = { nullptr };

public:
	static CTrailMesh_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END