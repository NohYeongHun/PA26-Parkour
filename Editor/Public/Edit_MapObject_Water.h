#pragma once
#include "StaticObject.h"
#include"Editor_Enum.h"
NS_BEGIN(Engine)
class CModel;
class CModel_Instance;
class CShader;
NS_END
NS_BEGIN(Editor)
class CEdit_MapObject_Water final: public CStaticObject
{
private:

public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex;
		_float4x4 WorldMatrix;
		_float4 vDiffuseColor = _float4(1.f, 1.f, 1.f, 1.f);
		_uint iLevel = ENUM_CLASS(LEVEL::MAP);
	}MAP_LOAD;

private:
	CEdit_MapObject_Water(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_MapObject_Water(const CEdit_MapObject_Water& Prototype);
	virtual ~CEdit_MapObject_Water() = default;

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
	void Set_ShaderPass(_uint iShaderPass);

private:
	void Ready_Events();

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	class CMap_Interface* m_pMapInterface = { nullptr };

private:
	_char m_ModelName[MAX_PATH];
	OBJECTTYPE m_eObjectType = { OBJECTTYPE::END };
	_uint m_iShaderPassIndex = {};
	_uint m_iNumInstance = {};
	_uint m_iPickedInstance = {};
	_float4x4* m_pInstanceMatrix = { nullptr };
	_float4* m_pRotation = { nullptr };
	_float4* m_pScale = { nullptr };
	_float4* m_pTranslation = { nullptr };

	_float m_fXOffset ={};
	_float m_fYOffset ={};

	_float m_XBindOffSet = {};
	_float m_YBindOffSet = {};
	_bool m_IsOffset = { false };
	MAP_LOAD InstanceDesc = {};
	_uint m_iSaveIndex = {};
	union MyFloat4 {
		_vector Vec = XMVectorSet(1.f, 1.f, 1.f, 1.f);
		_float4 float_4;
		_float arr[4];
	};
	MyFloat4 m_vDiffuseColor = {};
	_float m_fTotalTime = {};
	_uint m_iLevel = { ENUM_CLASS(LEVEL::MAP) };
public:
	static CEdit_MapObject_Water* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END