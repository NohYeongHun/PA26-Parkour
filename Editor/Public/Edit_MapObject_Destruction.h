#pragma once
#include"StaticObject.h"
#include"Editor_Enum.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CRigidbody;
NS_END

NS_BEGIN(Editor)
class CEdit_MapObject_Destruction : public CStaticObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		_float4x4* WorldMatrix = { nullptr };
		_uint iLevel = ENUM_CLASS(LEVEL::MAP);
		OBJECTTYPE eObjectType;
		_float3 vBoundingPos;
		_float3 vBoundingExtends;
		_float3 m_vImpulsePos = {};
		_float3 m_vImpulsePower = _float3(1.f, 1.f, 1.f);
		_uint iTriggerIndex = {};
	}MAP_LOAD;

private:
	CEdit_MapObject_Destruction(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_MapObject_Destruction(const CEdit_MapObject_Destruction& Prototype);
	virtual ~CEdit_MapObject_Destruction() = default;

public:
	virtual		HRESULT		Initialize_Prototype();
	virtual		HRESULT		Initialize_Clone(void* pArg);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	virtual		void			Late_Update(_float fTimeDelta);
	virtual		void			Render();
	virtual		void			Render_Shadow();


	virtual void Set_ImGuiOption();

	void Create_Particles();
	HRESULT Ready_Component(void* pArg = nullptr);

	virtual void Bind_Resources();

	_char* Get_ModelName() { return m_ModelName; }
	void Set_ShaderPass(_uint iShaderIndex) { m_iShaderPassIndex = iShaderIndex; }
private:
	_char m_ModelName[MAX_PATH] = {};
	_char m_BoneModelName[MAX_PATH] = {};
	class CMap_Interface* m_pMapInterface = { nullptr };
	_uint m_iShaderPassIndex = {};
	_uint m_iLevel = {};
	OBJECTTYPE m_eObjectType = { OBJECTTYPE::END };
	vector<CModel*> m_pModelComArray;
	CModel* m_pBoneModel = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CModel_Streaming* m_pModelCom = { nullptr };


	_float3 m_vScale = {};
	_float3 m_vRotation = {};
	_float3 m_vTranslation = {};

	_float3 m_vNewScale = {};
	_float3 m_vNewRotation = {};
	_float3 m_vNewTranslation = {};
	_bool m_IsDestroy = { false };

	_float m_vImpulsePos[3] = {};
	_float m_vImpulsePower[3] = { 1.f,1.f,1.f };

	_uint m_iTriggerIndex = {};

public:
	static CEdit_MapObject_Destruction* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;


};

NS_END