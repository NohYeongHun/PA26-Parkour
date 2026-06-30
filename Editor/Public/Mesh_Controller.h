#pragma once
#include "Base.h"
#include "Effect_Mesh.h"

NS_BEGIN(Editor)

class CMesh_Controller final : public CBase
{
public:
	typedef struct MeshTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;
	}MESH_TEXTURE;

	typedef struct MeshDatTag {
		_char szName[MAX_PATH] = {};
		_tchar strMeshTag[MAX_PATH] = {};
		_char szDatPath[MAX_PATH] = {};
		//�Ž��� �̸����� ��� ���ϳ� ?
	}MESH_TAG;

private:
	explicit CMesh_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMesh_Controller() = default;

#pragma region �⺻
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	//void Texture_Loading(const char* TextureName, const _tchar* pFilePath);
	void Load_AllTextureFromFolder(const _string& strFolderPath);
	void Load_AllMeshDatFromFolder(const _string& strFolderPath);

public:
	void EffectMesh_Tab();

	void EffectMesh_Base_Tab(CEffect_Mesh::EFFECTMESH_DESC& tEffectMeshDesc, _bool& IsCreate);

public:
	void Set_EffectMeshTag(const _char* szEffectMeshTag);
	void UpdateSelected_FXMeshFormTag(_wstring FMMeshTag);

	CEffect_Mesh::EFFECTMESH_DESC* Get_EffectMeshDesc(_wstring& EffectMeshTag);
	CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC* Get_VBMeshDesc(_wstring& VBMesTag);
	void Set_EffectMeshDesc(_wstring& MeshTag, CEffect_Mesh::EFFECTMESH_DESC& MeshDesc);
	void Set_MeshVBDesc(_wstring& MeshTag, CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC& MeshVBDesc);

	void Remove_Desc(const _wstring& DescTag);

	//void UpdateSelected_ParticleFormTag(_wstring ParticleTag);
	//
	//CEffect_Mesh::EFFECTMESH_DESC* Get_ParticleDesc(_wstring& ParticleTag);
	//CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* Get_VBDesc(_wstring& VBTag);

	//void Set_ParticleTag(const _char* szParticleTag);
	//
	//void Remove_Desc(const _wstring& DescTag);

private:
	ID3D11Device*												m_pDevice = { nullptr };
	ID3D11DeviceContext*										m_pContext = { nullptr };
	class CGameInstance*										m_pGameInstance = { nullptr };

	vector<MESH_TEXTURE>										m_Textures = {};
	_int														m_iSelectedTexture = -1;
	_bool														m_TexturPopOpend = false;

	vector<MESH_TAG>											m_MeshVBTag = {};
	_int														m_iSelectedMeshVBTag = -1;
	_bool														m_bMeshVBTag = false;

	_char														m_TrailMeshTag[MAX_PATH] = {};
	_bool														m_bTagFlag = false;

	_bool														m_IsRoot = false;

	map<const _wstring, CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC>	m_tVBMeshDesc = {};
	map<const _wstring, CEffect_Mesh::EFFECTMESH_DESC>			m_tEffectMeshDesc = {};

	_bool														m_bSelectedMesh = false;
	CEffect_Mesh::EFFECTMESH_DESC*								m_pSelectedEffectMeshDesc = { nullptr };
	CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC*						m_pSelectedVBFXDesc = { nullptr };

	//_int														m_iSelectedParticle = 0;
	//_bool														m_bSelectedParticle = false;
	//CEffect_Mesh::PARTICLE_DESC*									m_pSelectedParticleDesc = { nullptr };
	//CVIBuffer_Point_Instance::POINT_INSTANCE_DESC*				m_pSelectedVBDesc = { nullptr };

public:
	static CMesh_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END