#pragma once
#include "Character.h"

NS_BEGIN(Client)
class CTraceur final : public CCharacter
{
#pragma region 기본 함수들.
protected:
	explicit CTraceur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CTraceur(const CTraceur& Prototype);
	virtual ~CTraceur() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void	Priority_Update(_float fTimeDelta) override;
	virtual	void	Update(_float fTimeDelta) override;
	virtual	void	Late_Update(_float fTimeDelta) override;
	virtual	void	Render() override;


public:
	void OnCollider_During(_uint iLayer, void* pDesc, const ContactManifold& Manifold);
	void OnCollider_Enter(_uint iLayer, void* pDesc, const ContactManifold& Manifold);

private:
	// SingleTon
	class CGameSystem* m_pGameSystem = { nullptr };

private:
	// GameObjects
	class CSpringCamera* m_pSpringCamera = { nullptr };

private:
	// Components
	class CRigidbody* m_pRigidbodyCom = { nullptr };
	class CCollider* m_pColliderCom = { nullptr };
	
	class CMovementComponent* m_pMoveCom = { nullptr };
	class CEnvironmentQueryComponent* m_pEnvQueryCom = { nullptr };
	

private:
	// 임시 Variables
	_float m_fTrackPosition;
	ANIMATION_PLAY_DESC m_AnimPlayDesc{}; 
	ROOTMOTION_DESC m_RootModtionDesc{};


private:
	// Priority Update
	void PreUpdate_Input(_float fTimeDelta);
	void Save_PreviousPosition();
	void Handle_Input(_float fTimeDelta);

private:
	// Update
	void Update_Physics(_float fTimeDelta);
	void Update_EnvQuery(_float fTimeDelta);
	void Update_Camera(_float fTimeDelta);

	void Sync_Transform();

private:
	// Late Update
	void Ready_Render();

private:
	HRESULT Ready_Components(const CHARACTER_DESC* pDesc);
	HRESULT Ready_EnvQueryComponents(const CHARACTER_DESC* pDesc);

	HRESULT Ready_Variables(const CHARACTER_DESC* pDesc);
	HRESULT Bind_Matrices();

private:
	// Helper
	void Update_Collider(_float fTimeDelta);

public:
	static		CTraceur* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;
};
NS_END

