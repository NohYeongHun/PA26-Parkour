#include "ClientPch.h"
#include "MotionWarpingComponent.h"
#include "Model.h"
#include "Transform.h"
#include "Collider.h"

CMotionWarpingComponent::CMotionWarpingComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CMotionWarpingComponent::CMotionWarpingComponent(const CMotionWarpingComponent& Prototype)
	: CComponent(Prototype)
{
}

HRESULT CMotionWarpingComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMotionWarpingComponent::Initialize_Clone(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	if (nullptr == m_pOwner)
		return E_FAIL;

	m_pOwnerModelCom = dynamic_cast<CModel*>(m_pOwner->Get_Component(TEXT("Com_Model")));
	if (nullptr == m_pOwnerModelCom)
		return E_FAIL;

	m_pOwnerTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(TEXT("Com_Transform")));
	if (nullptr == m_pOwnerTransformCom)
		return E_FAIL;

	m_pOwnerColliderCom = dynamic_cast<CCollider*>(m_pOwner->Get_Component(TEXT("Com_Collider")));
	if (nullptr == m_pOwnerColliderCom)
		return E_FAIL;

	return S_OK;
}

void CMotionWarpingComponent::Set_WarpTarget(const _string& strName, const _float3& vPos)
{
	WARP_TARGET target{};
	target.vPosition = vPos;
	target.hasRotation = false;
	target.isValid = true;
	m_WarpTargets[strName] = target;
}

void CMotionWarpingComponent::Set_WarpTarget(const _string& strName, const _float3& vPos, const _float4& qRot)
{
	WARP_TARGET target{};
	target.vPosition = vPos;
	target.hasRotation = true;
	target.qRotation = qRot;
	target.isValid = true;
	m_WarpTargets[strName] = target;
}

void CMotionWarpingComponent::Clear_WarpTargets()
{
	m_WarpTargets.clear();
}

void CMotionWarpingComponent::Abort_Warp()
{
	if (m_pOwnerModelCom)
		m_pOwnerModelCom->End_MotionWarp();
	Clear_WarpTargets();
}

void CMotionWarpingComponent::On_WarpNotify(const _string& strName, _bool isStart,
                                            _float fWindowEndTrackPos, _bool bTrans, _bool bRot)
{
	if (nullptr == m_pOwnerModelCom)
		return;

	if (!isStart)
	{
		m_pOwnerModelCom->End_MotionWarp();
		return;
	}

	auto it = m_WarpTargets.find(strName);
	if (it == m_WarpTargets.end() || !it->second.isValid)
		return;

	const WARP_TARGET& T = it->second;
	m_pOwnerModelCom->Begin_MotionWarp(
		T.vPosition, T.hasRotation ? &T.qRotation : nullptr,
		fWindowEndTrackPos, bTrans, bRot);

	
}

#ifdef _DEBUG
void CMotionWarpingComponent::Update_DebugTrail()
{
	if (nullptr == m_pOwnerModelCom || nullptr == m_pOwnerTransformCom)
		return;

	if (m_pOwnerModelCom->Is_MotionWarping())
	{
		_float3 vPos{};
		XMStoreFloat3(&vPos, m_pOwnerTransformCom->Get_State(STATE::POSITION));

		constexpr _float kMinStepSq = 0.01f * 0.01f; // 1cm^2
		_bool bAppend = m_DebugTrail.empty();
		if (!bAppend)
		{
			const _float3& p = m_DebugTrail.back();
			_float dx = vPos.x - p.x, dy = vPos.y - p.y, dz = vPos.z - p.z;
			bAppend = (dx * dx + dy * dy + dz * dz) > kMinStepSq;
		}
		if (bAppend)
			m_DebugTrail.push_back(vPos);
	}

	Draw_DebugTrail();
}

void CMotionWarpingComponent::Draw_DebugTrail()
{
	if (nullptr == m_pGameInstance || !m_pGameInstance->IsParkourDebug())
		return;

	// 궤적 선분 (초록)
	for (size_t i = 1; i < m_DebugTrail.size(); ++i)
		m_pGameInstance->Add_DebugLine(XMLoadFloat3(&m_DebugTrail[i - 1]),
		                               XMLoadFloat3(&m_DebugTrail[i]),
		                               JPH::Color(0.f, 255.f, 0.f, 1.f));

	// 시작점 구 (자홍)
	if (!m_DebugTrail.empty())
		m_pGameInstance->Add_DebugSphere(XMLoadFloat3(&m_DebugTrail.front()),
		                                 0.05f, JPH::Color(255.f, 0.f, 255.f, 1.f));
}

void CMotionWarpingComponent::Reset_DebugTrail()
{
	m_DebugTrail.clear();
}
#endif

void CMotionWarpingComponent::Begin_CurveWarp(_fvector vStart, _fvector vEnd, _float fApexOffsetY,
	_fvector vLookStart, _fvector vLookTarget)
{
	_vector vP1 = (vStart + vEnd) * 0.5f;
	vP1 = XMVectorSetY(vP1, XMVectorGetY(vStart) + fApexOffsetY);

	XMStoreFloat3(&m_vCurveP0, vStart);
	XMStoreFloat3(&m_vCurveP1, vP1);
	XMStoreFloat3(&m_vCurveP2, vEnd);
	XMStoreFloat3(&m_vLookStart, vLookStart);
	XMStoreFloat3(&m_vLookTarget, vLookTarget);
	m_isCurveWarping = true;
}

void CMotionWarpingComponent::Update_CurveWarp(_float fCurveT)
{
	if (!m_isCurveWarping)
		return;

	fCurveT = min(fCurveT, 1.f);
	_float  fSmoothT = fCurveT * fCurveT * (3.0f - 2.0f * fCurveT);
	_vector vPos = XMVectorSetW(QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, fSmoothT), 1.f);
	m_pOwnerTransformCom->Set_State(Engine::STATE::POSITION, vPos);

	_vector vLookLerp = XMVectorLerp(
		XMVector3Normalize(XMLoadFloat3(&m_vLookStart)),
		XMVector3Normalize(XMLoadFloat3(&m_vLookTarget)), fSmoothT);
	m_pOwnerTransformCom->LookDir(vLookLerp);

	m_pOwnerColliderCom->Set_Position(vPos);

#ifdef _DEBUG
	Draw_DebugCurveWarp();
#endif
}

#ifdef _DEBUG
void CMotionWarpingComponent::Draw_DebugCurveWarp()
{
	CGameInstance* pGI = CGameInstance::GetInstance();
	_vector vPrev = QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2, 0.f);
	for (_uint i = 1; i <= 20; ++i)
	{
		_vector vCur = QuadraticCurve(m_vCurveP0, m_vCurveP1, m_vCurveP2,
			static_cast<_float>(i) / 20.f);
		pGI->Add_DebugLine(vPrev, vCur, JPH::Color(0.f, 255.f, 0.f, 1.f));
		vPrev = vCur;
	}
	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP0), 0.1f, JPH::Color(0.f, 255.f, 255.f, 1.f));
	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP1), 0.1f, JPH::Color(255.f, 0.f, 255.f, 1.f));
	pGI->Add_DebugSphere(XMLoadFloat3(&m_vCurveP2), 0.1f, JPH::Color(255.f, 255.f, 255.f, 1.f));
}
#endif

CMotionWarpingComponent* CMotionWarpingComponent::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMotionWarpingComponent* pInstance = new CMotionWarpingComponent(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CMotionWarpingComponent");
		Safe_Release(pInstance);
	}
	return pInstance;
}

Engine::CComponent* CMotionWarpingComponent::Clone(void* pArg)
{
	CMotionWarpingComponent* pClone = new CMotionWarpingComponent(*this);
	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CMotionWarpingComponent (Clone)");
		Safe_Release(pClone);
	}
	return pClone;
}

void CMotionWarpingComponent::Free()
{
	__super::Free();
	m_pOwnerModelCom = nullptr;
	m_WarpTargets.clear();

#ifdef _DEBUG
	m_DebugTrail.clear();
#endif
}
