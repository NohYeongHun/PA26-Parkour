#include "ClientPch.h"
#include "MeshAlignComponent.h"

CMeshAlignComponent::CMeshAlignComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CMeshAlignComponent::CMeshAlignComponent(const CMeshAlignComponent& Prototype)
	: CComponent(Prototype)
{
}

HRESULT CMeshAlignComponent::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeshAlignComponent::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

void CMeshAlignComponent::Request_Pose(_fvector qRot, const _float3& vLocalOffset, _float fBlendTime)
{
	m_qStartRot    = m_qCurrentRot;
	m_vStartOffset = m_vCurrentOffset;
	XMStoreFloat4(&m_qTargetRot, XMQuaternionNormalize(qRot));
	m_vTargetOffset  = vLocalOffset;
	m_fBlendDuration = fBlendTime;
	m_fBlendElapsed  = 0.f;

	if (m_fBlendDuration <= 0.f)
	{
		m_qCurrentRot    = m_qTargetRot;
		m_vCurrentOffset = m_vTargetOffset;
	}
}

void CMeshAlignComponent::Clear_Pose(_float fBlendTime)
{
	Request_Pose(XMQuaternionIdentity(), _float3(0.f, 0.f, 0.f), fBlendTime);
}

void CMeshAlignComponent::Update(_float fTimeDelta)
{
	if (!IsBlending())
		return;

	m_fBlendElapsed = min(m_fBlendElapsed + fTimeDelta, m_fBlendDuration);
	_float fRatio = m_fBlendElapsed / m_fBlendDuration;

	XMStoreFloat4(&m_qCurrentRot, XMQuaternionSlerp(
		XMLoadFloat4(&m_qStartRot), XMLoadFloat4(&m_qTargetRot), fRatio));
	XMStoreFloat3(&m_vCurrentOffset, XMVectorLerp(
		XMLoadFloat3(&m_vStartOffset), XMLoadFloat3(&m_vTargetOffset), fRatio));
}

_matrix CMeshAlignComponent::Get_LocalMatrix() const
{
	return XMMatrixRotationQuaternion(XMLoadFloat4(&m_qCurrentRot))
		* XMMatrixTranslation(m_vCurrentOffset.x, m_vCurrentOffset.y, m_vCurrentOffset.z);
}

CMeshAlignComponent* CMeshAlignComponent::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeshAlignComponent* pInstance = new CMeshAlignComponent(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : MeshAlignComponent");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMeshAlignComponent::Clone(void* pArg)
{
	CMeshAlignComponent* pClone = new CMeshAlignComponent(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : MeshAlignComponent (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMeshAlignComponent::Free()
{
	__super::Free();
}
