#pragma once
#include "Component.h"
#include "Client_Struct.h"

NS_BEGIN(Client)
class CClimbEvaluator final : public Engine::CComponent
{
public:
	typedef struct tagClimbEvaluatorDesc : Engine::CComponent::COMPONENT_DESC
	{
	}CLIMB_EVALUATOR_DESC;

protected:
	explicit CClimbEvaluator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CClimbEvaluator(const CClimbEvaluator& Prototype);
	virtual ~CClimbEvaluator() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize_Clone(void* pArg) override;

public:
	void Begin_Climb(_fvector vWallNormal);
	void Evaluate(const ENV_PERCEPTION& P, const PARKOUR_DECISION& D, _float fTimeDelta);
	const CLIMB_EVAL& Get_Eval() const { return m_Eval; }

private:
	CLIMB_EVAL m_Eval;

	_bool   m_hasLeftGround    = false;
	_float3 m_vTopStandCache   = {};
	_bool   m_hasTopStandCache = false;

	class CCollider*          m_pColliderCom  = { nullptr };
	Engine::CTransform*       m_pTransformCom = { nullptr };

public:
	static CClimbEvaluator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual Engine::CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END
