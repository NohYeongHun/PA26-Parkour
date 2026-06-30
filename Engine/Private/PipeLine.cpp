#include "EnginePch.h"
#include "PipeLine.h"

#include "GameObject.h"
#include "Transform.h"

CPipeLine::CPipeLine()
{
}

const _float4x4* CPipeLine::Get_TransformState_Float4x4(D3DTS eState) const
{
	return &m_TransformMatrixes[ENUM_CLASS(eState)];
}

_matrix CPipeLine::Get_TransformState_Matrix(D3DTS eState) const
{
	return XMLoadFloat4x4(&m_TransformMatrixes[ENUM_CLASS(eState)]);
}

const _float4x4* CPipeLine::Get_TransformState_Float4x4_Inv(D3DTS eState) const
{
	return &m_TransformMatrixes_Inv[ENUM_CLASS(eState)];
}

_matrix CPipeLine::Get_TransformState_Matrix_Inv(D3DTS eState) const
{
	return XMLoadFloat4x4(&m_TransformMatrixes_Inv[ENUM_CLASS(eState)]);
}

const _float4x4* CPipeLine::Get_PrevTransformState_Float4x4(D3DTS eState) const
{
	return &m_PrevTransformMatrixes[ENUM_CLASS(eState)];
}

_matrix CPipeLine::Get_PrevTransformState_Matrix(D3DTS eState) const
{
	return XMLoadFloat4x4(&m_PrevTransformMatrixes[ENUM_CLASS(eState)]);
}

void CPipeLine::Set_TransformState(D3DTS eState, _fmatrix Matrix)
{
	XMStoreFloat4x4(&m_TransformMatrixes[ENUM_CLASS(eState)], Matrix);
}

void CPipeLine::Set_TransformState(D3DTS eState, const _float4x4& Matrix)
{
	m_TransformMatrixes[ENUM_CLASS(eState)] = Matrix;
}

void CPipeLine::Set_PrevTransformState(D3DTS eState, _fmatrix Matrix)
{
	XMStoreFloat4x4(&m_PrevTransformMatrixes[ENUM_CLASS(eState)], Matrix);
}

void CPipeLine::Set_PrevTransformState(D3DTS eState, const _float4x4& Matrix)
{
	m_PrevTransformMatrixes[ENUM_CLASS(eState)] = Matrix;
}

_float CPipeLine::Compute_Distance(CGameObject* pObject)
{
	_vector vPos = static_cast<CTransform*>(pObject->Get_Component(TEXT("Com_Transform")))->Get_State(STATE::POSITION);

	return XMVectorGetX(XMVector3Length(XMLoadFloat4(&m_vCamPos) - vPos));
}

HRESULT CPipeLine::Initialize()
{
	for (size_t i = 0; i < ENUM_CLASS(D3DTS::END); ++i)
	{
		XMStoreFloat4x4(&m_TransformMatrixes[i], XMMatrixIdentity());
		XMStoreFloat4x4(&m_TransformMatrixes_Inv[i], XMMatrixIdentity());
		XMStoreFloat4x4(&m_PrevTransformMatrixes[i], XMMatrixIdentity());
	}

	return S_OK;
}

void CPipeLine::Update()
{
	for (size_t i = 0; i < ENUM_CLASS(D3DTS::END); ++i)
		XMStoreFloat4x4(&m_TransformMatrixes_Inv[i], XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_TransformMatrixes[i])));

	memcpy(&m_vCamPos, &m_TransformMatrixes_Inv[ENUM_CLASS(D3DTS::VIEW)].m[ENUM_CLASS(STATE::POSITION)], sizeof(_float4));
}

CPipeLine* CPipeLine::Create()
{
	CPipeLine* pInstance = new CPipeLine();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : PipeLine");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPipeLine::Free()
{
	__super::Free();
}
