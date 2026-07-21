#pragma once
#include "Component.h"

NS_BEGIN(Client)
class CIKDriver final : public CComponent
{
public:
	typedef struct tagIKDriverDesc : Engine::CComponent::COMPONENT_DESC
	{
	}IK_DRIVER_DESC;

protected:
	explicit CIKDriver(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CIKDriver(const CIKDriver& Prototype);
	virtual ~CIKDriver() = default;

public:
	// IKComponent에 명령 및 ACTIVE_IK 등록
	void Activate(const _string& strTarget, const _string& strToken, EIKTARGET_MODE eMode
		, _float fPosWeight, _float fRotWeight, _float fBlendSec, IK_TRIGGER eTrigger, _bool isFix);
	void Deactivate(const _string& strTarget, _float fBlendSec);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize_Clone(void* pArg) override;

public:
	void Execute(_float fTimeDelta);

private:
	class CIKComponent* m_pIKCom = { nullptr }; 
	class CEnvironmentQueryComponent* m_pEnvQueryCom = { nullptr };
	class CTransform* m_pTransformCom = { nullptr };
	class CMeshAlignComponent* m_pMeshAlignCom = { nullptr };

private:
	unordered_map<_string, ACTIVE_IK> m_ActiveIKSource{};

public:
	static CIKDriver* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

