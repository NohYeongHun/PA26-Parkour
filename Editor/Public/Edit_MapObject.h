#pragma once
#include"StaticObject.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CRigidbody;
class CTexture;
class CModel_Streaming;
NS_END


NS_BEGIN(Editor)
class CEdit_MapObject : public CStaticObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
		_uint iLevel = ENUM_CLASS(LEVEL::MAP);
		OBJECTTYPE eObjectType;
		PARKOUR_FLAG eParkourFlag = { PARKOUR_FLAG::ALL };
		_float3 vBoundingPos;
		_float3 vBoundingExtends;
		CEdit_MapObject* pCopyObject = { nullptr };
		_bool IsChild = { false };
		CEdit_MapObject* pParent = { nullptr };
	}MAP_LOAD;

	typedef struct tagMapSave
	{
		_uint m_iNameLength;
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex;
		_float4x4 WorldMatrix;
	}OBJECT_SAVE;

protected:
	CEdit_MapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_MapObject(const CEdit_MapObject& Prototype);
	virtual ~CEdit_MapObject() = default;

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
	virtual		void			Render_Shadow();

	virtual void Set_ImGuiOption();

	HRESULT Ready_Component(void* pArg = nullptr);

	virtual void Bind_Resources();

	_char* Get_ModelName() { return m_ModelName; }

	void Add_Child(CEdit_MapObject* pObject);
	void Quit_Child(CEdit_MapObject* pObject);
	void Make_ChildLocalMatrix(_fmatrix ParentMatrix);
	void Set_ShaderPass(_uint i) { m_iShaderPassIndex = i; }


	void Load_SoundTag(_string FilePath);
protected:
	void Export_MaterialData();
	void Child_UpdateMatrix(_fmatrix Matrix, _fvector vParentsPos, _fvector vDeltaTranslation);
	void About_Parent();
	void About_Transform();
	void About_Parkour();
	void About_Texture();
	void Copy_MapObject(_bool IsChild = false, CEdit_MapObject* pParent = nullptr);

	void SetCopyData(CEdit_MapObject* pParent);
protected:
	class CShader* m_pShaderCom = { nullptr };
	CModel_Streaming* m_pModelCom = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };
	CEdit_MapObject* m_pParent = { nullptr };
	list<CEdit_MapObject*> m_ChildObjects;

	CEdit_MapObject* m_pPickedChild = { nullptr };
	_bool m_IsSetParent = { false };
	_bool m_IsParent = { false };
	vector<CTexture*> m_pDiffuseTextureCom;
	vector<CTexture*> m_pNormalTextureCom;
	vector<CTexture*> m_pMaskTextureCom;
	vector<CTexture*> m_pMaskDiffuseTextureCom;
	vector<CTexture*> m_pMaskNormalTextureCom;


	class CMap_Interface* m_pMapInterface = {nullptr};

	_bool m_ExportAllLOD = { true };
	_float4x4 m_ChildLocalMat = {};

	_float4x4 m_DefaultMat = {};
protected:
	_float m_fTestRot = {};
	_char m_ModelName[MAX_PATH];

	_uint m_iShaderPassIndex = {};
	_float3 m_vScale = {};
	_float3 m_vRotation = {};
	_float3 m_vTranslation = {};

	_float3 m_vNewScale = {};
	_float3 m_vNewRotation = {};
	_float3 m_vNewTranslation = {};

	_bool m_IsCustomTexture = { false };
	_bool m_IsLoaded = { false };
	_bool m_MakeJson = { false };
	_bool m_TexMode = { false };
	
	//
	vector<_string> m_EntireDiffuseTextureName;
	vector<_string> m_EntireNormalTextureName;
	vector<_string> m_EntireMaskTextureName;

	
	vector<_string> m_SelectedDiffuseTexturePath;
	vector<_string> m_SelectedNormalTexturePath;
	vector<_string> m_SelectedMaskTexturePath;
	vector<_string> m_SelectedMaskDiffusePath;
	vector<_string> m_SelectedMaskNormalPath;

	_string m_SelectedDiffuse;
	_string m_SelectedNormal;
	_string m_SelectedMask;
	_string m_SelectedMaskDiffuse;
	_string m_SelectedMaskNormal;

	
	vector<_string >m_SelectedDiffuseName;
	vector<_string >m_SelectedNormalName;
	vector<_string> m_SelectedMaskTextureName;
	vector<_string> m_SelectedMaskDiffuseName;
	vector<_string> m_SelectedMaskNormalName;

	
	_string m_iSelectedMeshName;
	_uint m_iSelectedMesh={};

	_uint* m_iSelectedDiffuseIndex;
	_uint* m_iSelectedNormalIndex;
	_uint* m_iSelectedMaskIndex;
	_uint* m_iSelectedMaskDiffuseIndex;
	_uint* m_iSelectedMaskNormalIndex;
	
	_uint m_iNumObject = {};


	_uint m_iLevel = {};
	OBJECTTYPE m_eObjectType = { OBJECTTYPE::END };
	PARKOUR_FLAG m_eParkourFlag = { PARKOUR_FLAG::ALL };

	_bool m_IsSonoro = { false };
	_bool m_IsRender = { true };
	_float m_fFlyingTime = {};
	_bool m_IsFlying = { false };
	_float m_fDlayTime = {};
	_bool m_fMode = { false };
	_float m_fTotalTime = {};
	_float m_fDistortionTime = {};
	_float m_fDynamicVolume = {};

	vector<_string> m_SoundTags;
	_uint m_iPickedSoundTag = {};
protected:
	static _uint g_iNumObjects;

public:
	static CEdit_MapObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END