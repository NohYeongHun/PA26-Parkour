#include "ClientPch.h"
#include "ClimbEvaluator.h"
#include "Collider.h"

CClimbEvaluator::CClimbEvaluator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CClimbEvaluator::CClimbEvaluator(const CClimbEvaluator& Prototype)
	: CComponent(Prototype)
{
}

HRESULT CClimbEvaluator::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CClimbEvaluator::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	m_pColliderCom = dynamic_cast<CCollider*>(m_pOwner->Get_Component(TEXT("Com_Collider")));
	if (nullptr == m_pColliderCom) return E_FAIL;

	m_pTransformCom = dynamic_cast<Engine::CTransform*>(m_pOwner->Get_Component(TEXT("Com_Transform")));
	if (nullptr == m_pTransformCom) return E_FAIL;

	return S_OK;
}

void CClimbEvaluator::Begin_Climb(_fvector vWallNormal)
{
	m_hasLeftGround    = false;
	m_hasTopStandCache = false;
	XMStoreFloat3(&m_Eval.vClimbNormal, vWallNormal);
}

void CClimbEvaluator::Evaluate(const ENV_PERCEPTION& P, const PARKOUR_DECISION& D, _float fTimeDelta)
{
	_float3 vGroundN{};
	_bool isSupported = m_pColliderCom->IsLand(&vGroundN);
	_bool isLand = isSupported && vGroundN.y >= cosf(XMConvertToRadians(50.f));

	if (!isLand)
		m_hasLeftGround = true;

	if (P.Geometry.isTopReachable)
	{
		m_vTopStandCache   = P.Geometry.vTopEdgePos;
		m_hasTopStandCache = true;
	}

	_vector vPos     = m_pTransformCom->Get_State(Engine::STATE::POSITION);
	_float  fPosY    = XMVectorGetY(vPos);
	_vector vToCacheXZ = XMVectorSetY(XMLoadFloat3(&m_vTopStandCache) - vPos, 0.f);
	_float  fXZDistSq  = XMVectorGetX(XMVector3LengthSq(vToCacheXZ));
	const _float fRadius = m_pColliderCom->Get_Radius();
	_bool inTopBand = m_hasTopStandCache
		&& fPosY >= m_vTopStandCache.y - fRadius
		&& fXZDistSq <= (fRadius * 1.5f) * (fRadius * 1.5f);
	_bool isToppingOut = inTopBand && (!isSupported || fPosY >= m_vTopStandCache.y);

	m_Eval.isSupported = isSupported;
	m_Eval.isArrived   = m_hasLeftGround && isToppingOut;
	m_Eval.isLanded    = !m_Eval.isArrived && (m_hasLeftGround && isLand);
	m_Eval.shouldFall  = !m_Eval.isArrived && ((D.wantsDown || !isSupported) && !inTopBand);
	m_Eval.canMantle   = !m_Eval.isArrived && (P.Geometry.isTopReachable && !P.Scan.HeadHit.isHit);
	m_Eval.kneeHit     = P.Scan.KneeHit.isHit;
}

CClimbEvaluator* CClimbEvaluator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CClimbEvaluator* pInstance = new CClimbEvaluator(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CClimbEvaluator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

Engine::CComponent* CClimbEvaluator::Clone(void* pArg)
{
	CClimbEvaluator* pInstance = new CClimbEvaluator(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Clone Failed : CClimbEvaluator");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CClimbEvaluator::Free()
{
	__super::Free();
}
