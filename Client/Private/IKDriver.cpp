#include "ClientPch.h"
#include "IKDriver.h"
#include "IKComponent.h"
#include "EnvironmentQueryComponent.h"
#include "MeshAlignComponent.h"

CIKDriver::CIKDriver(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CIKDriver::CIKDriver(const CIKDriver& Prototype)
	: CComponent(Prototype)
{
}

void CIKDriver::Activate(const _string& strTarget, const _string& strToken, EIKTARGET_MODE eMode, _float fPosWeight, _float fRotWeight, _float fBlendSec, IK_TRIGGER eTrigger, _bool isFix)
{
	m_pIKCom->Begin_Target(strTarget, eMode, fPosWeight, fRotWeight, fBlendSec);

	ACTIVE_IK Active{};
	Active.strToken = strToken;
	Active.eTrigger = eTrigger;
	Active.isFixed = isFix;
	Active.isResolved = false; 
	m_ActiveIKSource[strTarget] = Active;
}

void CIKDriver::Activate_Fixed(const _string& strTarget, _fvector vWorldPos, _fvector vWorldNormal
	, EIKTARGET_MODE eMode, _float fPosWeight, _float fRotWeight, _float fBlendSec)
{
	m_pIKCom->Begin_Target(strTarget, eMode, fPosWeight, fRotWeight, fBlendSec);

	ACTIVE_IK Active{};
	Active.eTrigger   = IK_TRIGGER::STATE;
	Active.isFixed    = true;
	Active.isResolved = true;                       // 즉시 latch — perception 재쿼리 없음
	XMStoreFloat3(&Active.vFixedPos,    vWorldPos);
	XMStoreFloat3(&Active.vFixedNormal, vWorldNormal);
	m_ActiveIKSource[strTarget] = Active;
}

void CIKDriver::Activate_WallFoot(const _string& strTarget, _fvector vWallNormal, _float fPosWeight, _float fRotWeight, _float fBlendSec, _float fProbeOut, _float fProbeDepth, _float fSkin)
{
	
	m_pIKCom->Begin_Target(strTarget, EIKTARGET_MODE::POSITION_CLEARANCE, fPosWeight, fRotWeight, fBlendSec);

	ACTIVE_IK Active{};
	Active.eTrigger = IK_TRIGGER::STATE;
	Active.isWallProject = true;
	XMStoreFloat3(&Active.vWallNormal, vWallNormal);
	Active.fProbeOut = fProbeOut;
	Active.fProbeDepth = fProbeDepth;
	Active.fSkin = fSkin;
	m_ActiveIKSource[strTarget] = Active;
}

void CIKDriver::Deactivate(const _string& strTarget, _float fBlendSec)
{
	// 현재 Driver 제거.
	m_pIKCom->End_Target(strTarget, fBlendSec);
	m_ActiveIKSource.erase(strTarget);
}

HRESULT CIKDriver::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIKDriver::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	ASSERT_CRASH(m_pOwner != nullptr);

	m_pIKCom = dynamic_cast<CIKComponent*>(m_pOwner->Get_Component(TEXT("Com_IK")));
	ASSERT_CRASH(m_pIKCom != nullptr); // expression이 잘못되면 Crash

	m_pEnvQueryCom = dynamic_cast<CEnvironmentQueryComponent*>(m_pOwner->Get_Component(TEXT("Com_EnvQuery")));
	ASSERT_CRASH(m_pEnvQueryCom != nullptr);

	m_pTransformCom = dynamic_cast<CTransform*>(m_pOwner->Get_Component(TEXT("Com_Transform")));
	ASSERT_CRASH(m_pTransformCom != nullptr);

	m_pMeshAlignCom = dynamic_cast<CMeshAlignComponent*>(m_pOwner->Get_Component(TEXT("Com_MeshAlign")));
	ASSERT_CRASH(m_pMeshAlignCom != nullptr);

	return S_OK;
}

// Physics 시뮬레이션 이후 Late_Update 구간에서 실행되어야함 
// => 물리 처리가 끝난 애니메이션 상태에서 계산해야 정확함.
void CIKDriver::Execute(_float fTimeDelta)
{
	_matrix matModelToWorld = m_pMeshAlignCom->Get_LocalMatrix() * m_pTransformCom->Get_WorldMatrix();
	m_pIKCom->Set_SpaceMatrix(matModelToWorld);

	for (auto& [target, active] : m_ActiveIKSource)
	{
		
		_vector vWorld{};
		_vector vNormal{};
		_bool isValid{};

		if (active.isWallProject)
		{
			_vector vFootWorld{};
			
			if (!m_pIKCom->Get_TargetEndWorldPos(target, vFootWorld))
				continue;

			_vector vPos{}, vNor{};
			if (!m_pEnvQueryCom->Raycast_Wall(vFootWorld, XMLoadFloat3(&active.vWallNormal),
				active.fProbeOut, active.fProbeDepth, active.fSkin, vPos, vNor))
				continue;   // 미스 → 이 프레임 발 IK 스킵(애님 유지)

			m_pIKCom->Set_Target(target, vPos, vNor);
#ifdef _DEBUG
			m_pGameInstance->Add_DebugSphere(vPos, 0.05f, JPH::Color(0.f, 255.f, 0.f, 1.f));
#endif
			continue;
		}


		if (active.isFixed && active.isResolved)
		{
			vWorld = XMLoadFloat3(&active.vFixedPos);
			vNormal = XMLoadFloat3(&active.vFixedNormal);
			isValid = true;
		}
		else
		{
			isValid = m_pEnvQueryCom->Resolve_Anchor(active.strToken, vWorld, vNormal);
			
		}

		if (!isValid)
			continue;

		if (isValid && active.isFixed)
		{
			XMStoreFloat3(&active.vFixedPos, vWorld);
			active.isResolved = true;
		}

		m_pIKCom->Set_Target(target, vWorld, vNormal);
#ifdef _DEBUG
		m_pGameInstance->Add_DebugSphere(vWorld, 0.05f, JPH::Color(255.f, 0.f, 0.f, 1.f));
#endif
	}
	
	// Solver의 실행은 IK Component가 수행.
	m_pIKCom->Execute(fTimeDelta);

	
}

CIKDriver* CIKDriver::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIKDriver* pInstance = new CIKDriver(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CIKDriver");
		Safe_Release(pInstance);
	}
	return pInstance;
}

Engine::CComponent* CIKDriver::Clone(void* pArg)
{
	CIKDriver* pClone = new CIKDriver(*this);
	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : CIKDriver (Clone)");
		Safe_Release(pClone);
	}
	return pClone;
}

void CIKDriver::Free()
{
	__super::Free();
	m_pIKCom = nullptr;
	m_pEnvQueryCom = nullptr;
	m_pTransformCom = nullptr;
	m_pMeshAlignCom = nullptr;
	m_ActiveIKSource.clear();
}
