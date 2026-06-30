#include "EnginePch.h"
#include "Bone.h"

CBone::CBone()
{
}

CBone::CBone(const CBone& Copy)
	: m_TransformationMatrix { Copy.m_TransformationMatrix },
	m_CombinedTransformationMatrix { Copy.m_CombinedTransformationMatrix },
	m_iParentBoneIndex { Copy.m_iParentBoneIndex }
{
	strcpy_s(m_szName, MAX_PATH, Copy.m_szName);
}

HRESULT CBone::Initialize(const _char* pBoneName, const _fmatrix& TransformationMatrix, _int iParentBoneIndex)
{
	strcpy_s(m_szName, pBoneName);
	m_iParentBoneIndex = iParentBoneIndex;

	XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());

	return S_OK;
}

void CBone::Update_CombinedTransformationMatrix(const _fmatrix& PreTransformationMatrix, const vector<CBone*>& Bones)
{
	if (-1 == m_iParentBoneIndex)
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, PreTransformationMatrix * XMLoadFloat4x4(&m_TransformationMatrix));
		return;
	}
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, 
		XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
}

void CBone::Update_RibCombinedTransformationMatrix(const _fmatrix& PreTransformationMatrix, const vector<CBone*>& Bones)
{
	if (-1 == m_iParentBoneIndex)
	{
		/*_matrix		PreRibTransformMatrix = XMMatrixIdentity();
		_float fSize = 0.1f;
		PreRibTransformMatrix = XMMatrixScaling(fSize, fSize, fSize);
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, PreRibTransformMatrix);*/
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, PreTransformationMatrix * XMLoadFloat4x4(&m_TransformationMatrix));
		return;
	}
	XMStoreFloat4x4(&m_CombinedTransformationMatrix,
		XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
}

void CBone::Update_CombinedTransformationMatrix(const _fmatrix& PreTransformationMatrix, const _float4x4* pBoneMatrix, const vector<CBone*>& Bones)
{
	if (-1 == m_iParentBoneIndex)
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, PreTransformationMatrix * XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(pBoneMatrix));
		return;
	}
	XMStoreFloat4x4(&m_CombinedTransformationMatrix,
		XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
}

CBone* CBone::Create(const _char* pBoneName, const _fmatrix& TransformationMatrix, _int iParentBoneIndex)
{
	CBone* pInstance = new CBone();

	if (FAILED(pInstance->Initialize(pBoneName, TransformationMatrix, iParentBoneIndex)))
	{
		MSG_BOX("Failed to Create : Bone");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CBone* CBone::Clone()
{
	return new CBone(*this);
}

void CBone::Free()
{
	__super::Free();
}
