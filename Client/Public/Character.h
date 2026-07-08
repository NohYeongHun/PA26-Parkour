#pragma once
#include "GameObject.h"
#include "Client_Define.h"
#include "Client_CharacterEnum.h"

NS_BEGIN(Client)
class CCharacter abstract : public CGameObject
{
public:
	typedef struct tagCharacterDesc : public CGameObject::GAMEOBJECT_DESC
	{
		LEVEL eCurLevel = { LEVEL::END };
		pair<LEVEL, _wstring> modelData = {};
		pair<LEVEL, _wstring> shaderData = {};
		pair<LEVEL, _wstring> stateMachineData = {};
		pair<LEVEL, _wstring> computeShaderData = {};
		pair<LEVEL, _wstring> colliderData = {};
		pair<LEVEL, _wstring> rigidBodyData = {};
		pair<LEVEL, _wstring> inputControllerData = {};
		_float3 vScale = { 1.f, 1.f, 1.f };
		_float3 vRotation = { 0.f, 0.f, 0.f };
		_float3 vPosition = { 0.f, 0.f, 0.f };
		_string strFolderPath = {};
	}CHARACTER_DESC;

protected:
	explicit CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CCharacter(const CCharacter& Prototype);
	virtual ~CCharacter() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;

public:
	void	Move(ACTORDIR eDir, const _fvector& vCamForward, const _fvector& vCamRight, _float fTimeDelta, _float fSpeed);

protected:
	// Component
	class CModel* m_pModelCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	class CMovementComponent* m_pMoveCom = { nullptr };

protected:
	

protected:
	CALLBACK_CLIENT m_CallBack = {};
	_float3 m_vColliderOffSet = {};
	_float m_fColliderHeight = {};
	_float m_fColliderRadius = {};

	mutex m_Mutex;

protected:
	LEVEL m_eCurLevel = { LEVEL::END };


protected:
	HRESULT Ready_Components(const CHARACTER_DESC* pDesc);
	HRESULT Ready_MovementComponents(const CHARACTER_DESC* pDesc);

public:
	virtual		CGameObject* Clone(void* pArg) = 0;
	virtual		void Free() override;
};
NS_END

