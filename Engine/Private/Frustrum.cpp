#include "EnginePch.h"
#include "Frustrum.h"

#include "GameInstance.h"

CFrustrum::CFrustrum()
	: m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CFrustrum::Initialize()
{
	//for (_uint i = 0; i < 8; i++)
	//{
	//	_float fX = ( i & 2 ) ? -1.f : 1.f;
	//	_float fY = ( i & 1 ) ? -1.f : 1.f;
	//	_float fZ = ( i & 4 ) ? 1.f : 0.f;
	//	m_vLocalPoints[i] = _float4(fX, fY, fZ, 1.f);
	//}

	m_vLocalPoints[0] = _float4(-1.f, 1.f, 0.f, 1.f);
	m_vLocalPoints[1] = _float4(1.f, 1.f, 0.f, 1.f);
	m_vLocalPoints[2] = _float4(1.f, -1.f, 0.f, 1.f);
	m_vLocalPoints[3] = _float4(-1.f, -1.f, 0.f, 1.f);

	m_vLocalPoints[4] = _float4(-1.f, 1.f, 1.f, 1.f);
	m_vLocalPoints[5] = _float4(1.f, 1.f, 1.f, 1.f);
	m_vLocalPoints[6] = _float4(1.f, -1.f, 1.f, 1.f);
	m_vLocalPoints[7] = _float4(-1.f, -1.f, 1.f, 1.f);

    return S_OK;
}

void CFrustrum::Update()
{
	_matrix ViewMatrixInv = m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::VIEW);
	_matrix ProjMatrixInv = m_pGameInstance->Get_TransformState_Matrix_Inv(D3DTS::PROJ);

	for (_uint i = 0; i < 8; i++)
	{
		XMStoreFloat4(&m_vWorldPoints[i], XMVector3TransformCoord(XMLoadFloat4(&m_vLocalPoints[i]), ProjMatrixInv));
		XMStoreFloat4(&m_vWorldPoints[i], XMVector3TransformCoord(XMLoadFloat4(&m_vWorldPoints[i]), ViewMatrixInv));
	}

	Make_Planes(m_vWorldPoints, m_vWorldPlanes);
}

_bool CFrustrum::IsIn_WorldSpace(_fvector vWorldPosition, _float fRange)
{
	for (_uint i = 0; i < 6; i++)
	{
		if (fRange < XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vWorldPlanes[i]), vWorldPosition)))
			return false;
	}
	return true;
}

_bool CFrustrum::IsIn_LocalSpace(_fmatrix WorldMatrix, _fvector vLocalPosition, _float fRange)
{
	Update_LocalPlanes(WorldMatrix);

	for (_uint i = 0; i < 6; i++)
	{
		if (fRange < XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vWorldPlanes[i]), vLocalPosition)))
			return false;
	}
	return true;
}

_bool CFrustrum::IsIn_WorldSpace( const BoundingBox* pBoundingBox )
{

	for (_uint i = 0; i < 6; i++)
	{
		PlaneIntersectionType Result = pBoundingBox->Intersects(XMLoadFloat4(&m_vWorldPlanes[i]));
		
		if (Result == PlaneIntersectionType::FRONT)
			return false;
	}
	return true;
}

void CFrustrum::Make_Planes(const _float4* pPoints, _float4* pPlanes)
{
	XMStoreFloat4(&pPlanes[0],
		XMPlaneFromPoints(XMLoadFloat4(&pPoints[1]), XMLoadFloat4(&pPoints[5]), XMLoadFloat4(&pPoints[6])));
	XMStoreFloat4(&pPlanes[1],
		XMPlaneFromPoints(XMLoadFloat4(&pPoints[4]), XMLoadFloat4(&pPoints[0]), XMLoadFloat4(&pPoints[3])));
	XMStoreFloat4(&pPlanes[2],
		XMPlaneFromPoints(XMLoadFloat4(&pPoints[4]), XMLoadFloat4(&pPoints[5]), XMLoadFloat4(&pPoints[1])));
	XMStoreFloat4(&pPlanes[3],
		XMPlaneFromPoints(XMLoadFloat4(&pPoints[3]), XMLoadFloat4(&pPoints[2]), XMLoadFloat4(&pPoints[6])));
	XMStoreFloat4(&pPlanes[4],
		XMPlaneFromPoints(XMLoadFloat4(&pPoints[5]), XMLoadFloat4(&pPoints[4]), XMLoadFloat4(&pPoints[7])));
	XMStoreFloat4(&pPlanes[5],
		XMPlaneFromPoints(XMLoadFloat4(&pPoints[0]), XMLoadFloat4(&pPoints[1]), XMLoadFloat4(&pPoints[2])));
}

void CFrustrum::Update_LocalPlanes(_fmatrix WorldMatrix)
{
	_matrix		WorldMatrixInverse = XMMatrixInverse(nullptr, WorldMatrix);

	_float4		vLocalPoints[8] = {};

	for (_uint i = 0; i < 8; i++)
		XMStoreFloat4(&vLocalPoints[i], XMVector3TransformCoord(XMLoadFloat4(&m_vWorldPoints[i]), WorldMatrixInverse));

	Make_Planes(vLocalPoints, m_vLocalPlanes);
}

CFrustrum* CFrustrum::Create()
{
	CFrustrum* pInstance = new CFrustrum();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Frustrum");
		Safe_Release(pInstance);
	}

    return pInstance;
}

void CFrustrum::Free()
{
	__super::Free();

	Safe_Release(m_pGameInstance);
}
