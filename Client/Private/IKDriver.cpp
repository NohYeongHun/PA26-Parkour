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
	ASSERT_CRASH(m_pIKCom != nullptr);

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
void CIKDriver::Execute(const vector<IK_REQUEST>& Requests, _float fTimeDelta)
{
	_matrix matModelToWorld = m_pMeshAlignCom->Get_LocalMatrix() * m_pTransformCom->Get_WorldMatrix();
	m_pIKCom->Set_SpaceMatrix(matModelToWorld);

	Apply_Requests(Requests);
	Reap_Missing();

	for (auto& [strGoal, Entry] : m_ActiveMap)
	{
		if (Entry.Req.fDrivenAlpha >= 0.f)
			m_pIKCom->Set_TargetAlpha(strGoal, min(Entry.Req.fDrivenAlpha, 1.f));

		Resolve_Target(strGoal, Entry);
	}

	m_pIKCom->Execute(fTimeDelta);
}

void CIKDriver::Apply_Requests(const vector<IK_REQUEST>& Requests)
{
	// 매 프레임 isSeen 해제.
	for (auto& [strGoal, Entry] : m_ActiveMap)
		Entry.isSeen = false;

	// 상태 요청이 먼저, 클립 요청이 나중에 들어오므로
	// 같은 골이면 클립 요청이 덮어쓴다.
	for (const IK_REQUEST& Req : Requests)
	{
		auto it = m_ActiveMap.find(Req.strGoal);
		if (it == m_ActiveMap.end())
		{
			m_pIKCom->Begin_Target(Req.strGoal, Req.eMode, Req.fPosWeight, Req.fRotWeight, Req.fBlendInSec);

			ACTIVE_IK_ENTRY Entry{};
			Entry.Req = Req;
			Entry.isSeen = true;
			m_ActiveMap.emplace(Req.strGoal, Entry);
			continue;
		}

		ACTIVE_IK_ENTRY& Entry = it->second;

#ifdef _DEBUG
		if (Entry.isSeen)
			OutputDebugStringA(("[IKDriver] duplicate request in same layer: " + Req.strGoal + "\n").c_str());
#endif

		// 소스가 바뀌면 래치 무효화
		if (Entry.Req.eSource != Req.eSource || Entry.Req.strToken != Req.strToken)
			Entry.isLatched = false;

		// weight/mode가 바뀌면 솔버의 목표 weight 갱신 블렌드 재시작.
		if (Entry.Req.fPosWeight != Req.fPosWeight || Entry.Req.fRotWeight != Req.fRotWeight
			|| Entry.Req.eMode != Req.eMode)
			m_pIKCom->Begin_Target(Req.strGoal, Req.eMode, Req.fPosWeight, Req.fRotWeight, Req.fBlendInSec);

		Entry.Req = Req;
		Entry.isSeen = true;
	}
}

// 매프레임 체크.
void CIKDriver::Reap_Missing()
{
	for (auto it = m_ActiveMap.begin(); it != m_ActiveMap.end(); )
	{
		if (!it->second.isSeen)
		{
			m_pIKCom->End_Target(it->first, it->second.Req.fBlendOutSec);
			it = m_ActiveMap.erase(it);
		}
		else
			++it;
	}
}

void CIKDriver::Resolve_Target(const _string& strGoal, ACTIVE_IK_ENTRY& Entry)
{
	const IK_REQUEST& Req = Entry.Req;

	switch (Req.eSource)
	{
	case EIKSOURCE_MODE::FIXED:
	{
		m_pIKCom->Set_Target(strGoal, XMLoadFloat3(&Req.vWorldPos), XMLoadFloat3(&Req.vWorldNormal));
#ifdef _DEBUG
		m_pGameInstance->Add_DebugSphere(XMLoadFloat3(&Req.vWorldPos), 0.05f, JPH::Color(255.f, 0.f, 0.f, 1.f));
#endif
		break;
	}
	case EIKSOURCE_MODE::ANCHOR:
	{
		_vector vPos{}, vNormal{};
		if (Req.isFix && Entry.isLatched)
		{
			vPos = XMLoadFloat3(&Entry.vLatchedPos);
			vNormal = XMLoadFloat3(&Entry.vLatchedNormal);
		}
		else
		{
			if (!m_pEnvQueryCom->Resolve_Anchor(Req.strToken, vPos, vNormal))
				break;	// 미해석 — 이 프레임 스킵 (isFix면 다음 프레임 재시도)

			if (Req.isFix)
			{
				XMStoreFloat3(&Entry.vLatchedPos, vPos);
				XMStoreFloat3(&Entry.vLatchedNormal, vNormal);
				Entry.isLatched = true;
			}
		}

		m_pIKCom->Set_Target(strGoal, vPos, vNormal);

		// 거리 자석: FK 손끝이 앵커 반경 안으로 들어온 만큼만 알파 개입.
		if (Req.fReachRadius > 0.f)
		{
			_vector vEndFK{};
			if (m_pIKCom->Get_TargetEndWorldPos(strGoal, vEndFK))
			{
				const _float fDist = XMVectorGetX(XMVector3Length(vPos - vEndFK));
				_float w = 1.f - min(fDist / Req.fReachRadius, 1.f);
				w = w * w * (3.f - 2.f * w);	// smoothstep
				const _float fRamp = (Req.fDrivenAlpha >= 0.f) ? min(Req.fDrivenAlpha, 1.f) : 0.f;
				m_pIKCom->Set_TargetAlpha(strGoal, max(fRamp, w));
			}
		}
#ifdef _DEBUG
		m_pGameInstance->Add_DebugSphere(vPos, 0.05f, JPH::Color(255.f, 0.f, 0.f, 1.f));
#endif
		break;
	}
	case EIKSOURCE_MODE::WALL_PROBE:
	{
		_vector vFootWorld{};
		if (!m_pIKCom->Get_TargetEndWorldPos(strGoal, vFootWorld))
			break;

		// 토큰이 있으면(클립 저작 경로) 앵커의 노멀을 벽 방향으로 사용
		_vector vWallN = XMLoadFloat3(&Req.vWallNormal);
		if (!Req.strToken.empty())
		{
			_vector vAnchorPos{}, vAnchorN{};
			if (!m_pEnvQueryCom->Resolve_Anchor(Req.strToken, vAnchorPos, vAnchorN))
				break;	// 앵커 미해석 — 이 프레임 스킵
			vWallN = vAnchorN;
		}

		_vector vPos{}, vNormal{};
		if (!m_pEnvQueryCom->Raycast_Wall(vFootWorld, vWallN,
			Req.fProbeOut, Req.fProbeDepth, Req.fSkin, vPos, vNormal))
			break;	// 미스 — 이 프레임 발 IK 스킵 (애님 유지)

		m_pIKCom->Set_Target(strGoal, vPos, vNormal);
#ifdef _DEBUG
		m_pGameInstance->Add_DebugSphere(vPos, 0.05f, JPH::Color(255.f, 0.f, 0.f, 1.f));
#endif
		break;
	}
	}
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
	m_ActiveMap.clear();
}
