#pragma once
#include "Editor_Define.h"
#include "Base.h"

NS_BEGIN(Editor)

class CGltfLoader final : public CBase
{
private:
	struct ANIM_SET {
		aiAnimation* pMainAnim = nullptr;  // Bone 채널이 있는 메인 애니메이션
		aiAnimation* pMorphAnim = nullptr; // Morph 채널이 있는 서브 애니메이션 (이름에 _Curves)
	};

private:
	explicit CGltfLoader();
	virtual ~CGltfLoader() = default;

public:
	const aiScene* Get_Scene() { return m_pAIScene; }
public:
	HRESULT		Initialize();
	void			Update();

	
	HRESULT		Save_Dat_Anim(const _char* pFileName);
	HRESULT		Save_Dat_Character(const _char* pFileName);	
	HRESULT		Save_Animation(const _char* pFileName);
	HRESULT		Save_Animation_Character(const _char* pFileName);
	HRESULT		Save_Material(const _char* pFileName);


private:
	const aiScene*		m_pAIScene = { nullptr };
	Assimp::Importer	m_Importer = {};
	_string m_strImportDir = "";

	_uint					m_iAnim = {};
	MODELTYPE				m_eType = { MODELTYPE::NONANIM };
	_char					m_szReadPath[MAX_PATH] = {};
	_char					m_szWritePath[MAX_PATH] = {};
	_char					m_szFileName[MAX_PATH] = {};

	_bool					m_isShowLoadFile = { false };
	_bool					m_isShowSaveFile = { false };

	// Info
	_string				m_strModelName;

private:
	void					Load_File();
	void					Save_File();
	void					Show_Info();



private:
	aiMesh* FindMeshByMorphChannelName(const aiString& strMorphChannelName); // 메쉬 찾기 함수
	aiNode* Find_Node(aiNode* pNode, const _string& strNodeName);

private:
	_float4 GetRotationAtTime(aiNodeAnim* pNodeAnim, _float fTime);
	_float3 GetPositionAtTime(aiNodeAnim* pNodeAnim, _float fTime);
	_float3 GetScaleAtTime(aiNodeAnim* pNodeAnim, _float fTime);

private:
	HRESULT				Save_Texture(json& MaterialData, const aiMaterial* pMaterial, aiTextureType eType);
	HRESULT				Save_Bone(ofstream& OutPut, const aiNode* pNode);

public:
	static		CGltfLoader*		Create();
	virtual		void					Free() override;
};

NS_END