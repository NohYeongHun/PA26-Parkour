#pragma once
#include "Editor_Define.h"
#include "Base.h"

NS_BEGIN(Editor)

class CModelLoader final : public CBase
{
private:
	explicit CModelLoader();
	virtual ~CModelLoader() = default;

public:
	const aiScene* Get_Scene() { return m_pAIScene; }
public:
	HRESULT		Initialize();
	void			Update();

	HRESULT		Save_Dat_VatMesh(const _char* pFileName);
	HRESULT		Save_Dat_NonAnim(const _char* pFileName);
	HRESULT		Save_Dat_Anim(const _char* pFileName);
	HRESULT		Save_Dat_Character(const _char* pFileName);	
	HRESULT		Save_Animation(const _char* pFileName);
	HRESULT		Save_Animation_Character(const _char* pFileName);
	HRESULT		Save_Material(const _char* pFileName);


private:
	const aiScene*		m_pAIScene = { nullptr };
	Assimp::Importer	m_Importer = {};

	_uint					m_iAnim = {};
	MODELTYPE			m_eType = { MODELTYPE::NONANIM };
	_bool					m_isLoadAll = { false };
	_char					m_szReadPath[MAX_PATH] = {};
	_char					m_szWritePath[MAX_PATH] = {};
	_char					m_szFileName[MAX_PATH] = {};

	_bool					m_isShowLoadFile = { false };
	_bool					m_isShowSaveFile = { false };
	_bool					m_isMapSaveLoadFile = { false };

	// Info
	_string				m_strModelName;

private:
	void					Map_SaveLoad();
	void					Load_File();
	void					Save_File();
	void					Show_Info();

private:
	aiMesh* FindMeshByMorphChannelName(const aiString& strMorphChannelName); // 메쉬 찾기 함수
	aiNode* Find_Node(aiNode* pNode, const _string& strNodeName);

	void SimplifyChannel(const aiNodeAnim* pChannel, std::vector<KEYFRAME>& outKeys, float epsilon = 0.001f);

private:
	HRESULT				Save_Texture(json& MaterialData, const aiMaterial* pMaterial, aiTextureType eType);
	HRESULT				Save_Bone(ofstream& OutPut, const aiNode* pNode);

public:
	static		CModelLoader*		Create();
	virtual		void					Free() override;
};

NS_END