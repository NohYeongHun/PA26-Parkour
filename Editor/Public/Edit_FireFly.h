#pragma once
#include "GameObject.h"
NS_BEGIN(Engine)
class CModel_Instance_FireFly;
NS_END

NS_BEGIN(Editor)
class CEdit_FireFly final: public CGameObject
{
public:
	typedef struct tagFireFlyDesc {
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex;
		_float4x4* InstanceWorldMatrix = { nullptr };
		_uint iNumInstance;
		_float4x4 WorldMatrix;
		_uint iSaveIndex;
		_bool IsLoaded;
		INSTANCETYPE eInstanceType = { INSTANCETYPE::DEFAULT };

		_float2 vPerSin = {};
		_float2 vPerCos = {};
		_float2 vPerSin2 = {};
		_float2 vRange = {};
	}MAP_LOAD;
	
private:
	CEdit_FireFly(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_FireFly(const CEdit_FireFly& Prototype);
	virtual ~CEdit_FireFly() = default;

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
	void	Move(_fvector vPos);


	void Bind_Resources();
	_uint Get_Num() { return m_iSaveIndex; }
	void Change_ShaderIndex(_uint iNumShader) { m_iShaderPassIndex = iNumShader; }
	_char* GetName() { return m_ModelName; }
	_uint Get_ShaderPass() { return m_iShaderPassIndex; }
	_uint ShaderPassWindow();
	void Set_Color(_float4 vColor) { m_vDiffuseColor = vColor; }
	_float4* Get_Color() { return &m_vDiffuseColor; }
	void SaveData(ofstream& File);
	//INSTANCETYPE Get_Type() { return m_eInstanceType; }
	//void Set_Type(INSTANCETYPE eType) { m_eInstanceType = eType; }
private:
	void Ready_Events();

private:
	vector<CModel_Instance_FireFly*> m_pModelComArray;
	CModel_Instance_FireFly* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	class CMap_Interface* m_pMapInterface = { nullptr };
	//INSTANCETYPE m_eInstanceType = { INSTANCETYPE::DEFAULT };

private:
	_char m_ModelName[MAX_PATH];

	_uint m_iShaderPassIndex = {};
	_uint m_iNumInstance = {};
	_uint m_iPickedInstance = {};
	_float4x4* m_pInstanceMatrix = { nullptr };
	_float4* m_pRotation = { nullptr };
	_float4* m_pScale = { nullptr };
	_float4* m_pTranslation = { nullptr };

	//MAP_LOAD InstanceDesc = {};
	_uint m_iSaveIndex = {};
	_float4 m_vDiffuseColor = {};
	_float m_fTotalTime = {};

	MAP_LOAD m_Desc = {};
public:
	static CEdit_FireFly* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END