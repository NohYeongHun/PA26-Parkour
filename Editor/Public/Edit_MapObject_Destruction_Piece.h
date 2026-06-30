#pragma once
#include "StaticObject.h"
#include"Editor_Enum.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CRigidbody;
NS_END

NS_BEGIN(Editor)
class CEdit_MapObject_Destruction_Piece : public CStaticObject
{
public:
	typedef struct tagMapLoad
	{
		_char ModelName[MAX_PATH] = {};
		_uint iShaderPassIndex = {};
		const _float4x4* WorldMatrix = { nullptr };
		_uint iLevel = ENUM_CLASS(LEVEL::MAP);
		OBJECTTYPE eObjectType;
		_float3 vImpulse = {};
	}MAP_LOAD;

	typedef struct tagResetDesc {
		_float3 vImpulse;
	}RESET_DESC;

private:
	CEdit_MapObject_Destruction_Piece(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_MapObject_Destruction_Piece(const CEdit_MapObject_Destruction_Piece& Prototype);
	virtual ~CEdit_MapObject_Destruction_Piece() = default;

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

	virtual		void			Reset(const _fmatrix& WorldMatrix, void* pArg)override;

private:
	_char m_ModelName[MAX_PATH] = {};
	class CMap_Interface* m_pMapInterface = { nullptr };
	_uint m_iShaderPassIndex = {};
	_uint m_iLevel = {};
	OBJECTTYPE m_eObjectType = { OBJECTTYPE::END };
	CModel_Streaming* m_pModelCom = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };


	_float3 m_vScale = {};
	_float3 m_vRotation = {};
	_float3 m_vTranslation = {};

	_float3 m_vNewScale = {};
	_float3 m_vNewRotation = {};
	_float3 m_vNewTranslation = {};
	_float m_fTimeDelta = {};
	_float3 m_vImpulse = {};
	_bool m_IsTriggered = { false };
public:
	static CEdit_MapObject_Destruction_Piece* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg)override;
	virtual void Free()override;


};

NS_END