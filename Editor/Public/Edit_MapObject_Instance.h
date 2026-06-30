#pragma once
#include"GameObject.h"


NS_BEGIN(Engine)
class CModel;
class CModel_Instance;
class CShader;
NS_END


NS_BEGIN(Editor)
class CEdit_MapObject_Instance : public CGameObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex;
		_float4x4* InstanceWorldMatrix = { nullptr };
		_uint iNumInstance;
		_float4x4 WorldMatrix;
		_uint iSaveIndex;
		_float4 vDiffuseColor = _float4(1.f, 1.f, 1.f, 1.f);
		_bool IsLoaded;
		INSTANCETYPE eInstanceType = { INSTANCETYPE::DEFAULT };
	}MAP_LOAD;

private:
	CEdit_MapObject_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_MapObject_Instance(const CEdit_MapObject_Instance& Prototype);
	virtual ~CEdit_MapObject_Instance() = default;

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
	virtual		void			Render_Shadow();

	void Set_ImGuiOption();

	HRESULT Ready_Component(void* pArg = nullptr);

	void Bind_Resources();
	_uint Get_Num() { return m_iSaveIndex; }
	void Change_ShaderIndex(_uint iNumShader) { m_iShaderPassIndex = iNumShader; }
	_char* GetName() { return m_ModelName; }
	_uint Get_ShaderPass() { return m_iShaderPassIndex; }
	_uint ShaderPassWindow();
	void Set_Color(_float4 vColor) { m_vDiffuseColor = vColor; }
	_float4* Get_Color(){ return &m_vDiffuseColor; }
	INSTANCETYPE Get_Type() { return m_eInstanceType; }
	void Set_Type(INSTANCETYPE eType) { m_eInstanceType = eType; }
private:
	void Ready_Events();
	
private:
	vector<CModel_Instance*> m_pModelComArray;
	CModel_Instance* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	class CMap_Interface* m_pMapInterface = { nullptr };
	INSTANCETYPE m_eInstanceType = { INSTANCETYPE::DEFAULT };

private:
	_char m_ModelName[MAX_PATH];

	_uint m_iShaderPassIndex = {};
	_uint m_iNumInstance = {};
	_uint m_iPickedInstance = {};
	_float4x4* m_pInstanceMatrix = { nullptr };
	_float4* m_pRotation = { nullptr };
	_float4* m_pScale= { nullptr };
	_float4* m_pTranslation = { nullptr };

	MAP_LOAD InstanceDesc = {};
	_uint m_iSaveIndex = {};
	_float4 m_vDiffuseColor = {};
	_float m_fTotalTime = {};
public:
	static CEdit_MapObject_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END