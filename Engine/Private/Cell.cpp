#include "EnginePch.h"
#include "Cell.h"
#include "VIBuffer_Cell.h"

#include "Shader.h"

CCell::CCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CCell::Initialize(const CELL& pCell, _int iIndex)
{
	m_iIndex = iIndex;
	memcpy(m_Points, pCell.vPositions, sizeof(_float3) * ENUM_CLASS(POINTS::END));
	m_iType = pCell.iType;

	// Min / Max Y
	m_fMaxY = max(m_Points[0].y, max(m_Points[1].y, m_Points[2].y));
	m_fMinY = min(m_Points[0].y, min(m_Points[1].y, m_Points[2].y));

	// High / Low Line
	Compute_Line();

	// Normal
	_vector vAB = XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::B)]) - XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::A)]);
	_vector vBC = XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::C)]) - XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::B)]);
	_vector vCA = XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::A)]) - XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::C)]);

	_vector vUp = XMVector3Cross(vAB, vBC);
	XMStoreFloat3(&m_vPlaneNormal, XMVector3Normalize(vUp));

	XMStoreFloat3(&m_vNormals[ENUM_CLASS(LINE::AB)], XMVector3Normalize(XMVector3Cross(vAB, vUp)));
	XMStoreFloat3(&m_vNormals[ENUM_CLASS(LINE::BC)], XMVector3Normalize(XMVector3Cross(vBC, vUp)));
	XMStoreFloat3(&m_vNormals[ENUM_CLASS(LINE::CA)], XMVector3Normalize(XMVector3Cross(vCA, vUp)));

#ifdef _DEBUG
	m_pVIBuffer = CVIBuffer_Cell::Create(m_pDevice, m_pContext, m_Points);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;
#endif
	return S_OK;
}

_bool CCell::IsIn(const _fvector& vLocalPosition, const _fvector& vLookDir, _int* pNeighborIndex, _uint* pLineIndex)
{
	_float fHeight = Compute_Height(vLocalPosition);

	for (_uint i = 0; i < ENUM_CLASS(LINE::END); ++i)
	{
		_vector vDir = XMVector4Normalize(XMVectorSetY(vLocalPosition, fHeight) - XMVectorSetW(XMLoadFloat3(&m_Points[i]), 1.f));

		if (0.f < XMVector3Dot(vDir, XMLoadFloat3(&m_vNormals[i])).m128_f32[0]) // Left 
		{
			*pNeighborIndex = m_iNeighbors[ENUM_CLASS(LINE::AB) + i];
			*pLineIndex = i;
			return false;
		}
	}

	return true;
}

_bool CCell::Compare_Points(const _fvector& vSrcPoint, const _fvector& vDstPoint)
{
	if (true == XMVector3Equal(vSrcPoint, XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::A)])))
	{
		if (true == XMVector3Equal(vDstPoint, XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::B)])))
		return true;
		if(true == XMVector3Equal(vDstPoint, XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::C)])))
			return true;
	}
	else if (true == XMVector3Equal(vSrcPoint, XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::B)])))
	{
		if (true == XMVector3Equal(vDstPoint, XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::A)])))
			return true;
		if (true == XMVector3Equal(vDstPoint, XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::C)])))
			return true;
	}
	else if (true == XMVector3Equal(vSrcPoint, XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::C)])))
	{
		if (true == XMVector3Equal(vDstPoint, XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::A)])))
			return true;
		if (true == XMVector3Equal(vDstPoint, XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::B)])))
			return true;
	}

	return false;
}

void CCell::SetUp_Neighbor(LINE eLine, CCell* pCell)
{
	m_iNeighbors[ENUM_CLASS(eLine)] = pCell->m_iIndex;
}

_float CCell::Compute_Distance(const _fvector& vLocalPos, _float3* vOut)
{
	_vector vPlane = XMPlaneFromPoints(
		XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::A)]),
		XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::B)]),
		XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::C)]));

	_float fDistance = vPlane.m128_f32[0] * vLocalPos.m128_f32[0] +
		vPlane.m128_f32[1] * vLocalPos.m128_f32[1] +
		vPlane.m128_f32[2] * vLocalPos.m128_f32[2] +
		vPlane.m128_f32[3];

	if(nullptr != vOut)
		XMStoreFloat3(vOut, vLocalPos - XMLoadFloat3(&m_vPlaneNormal) * fDistance);

	return vPlane.m128_f32[0] * vLocalPos.m128_f32[0] + 
		vPlane.m128_f32[1] * vLocalPos.m128_f32[1] + 
		vPlane.m128_f32[2] * vLocalPos.m128_f32[2] + 
		vPlane.m128_f32[3];
}

void CCell::Compute_GoDir(const _fvector& vLocalDir, _float3* vOut)
{
	_float fDot = XMVectorGetX(XMVector3Dot(vLocalDir, XMLoadFloat3(&m_vPlaneNormal)));

	XMStoreFloat3(vOut, vLocalDir - XMLoadFloat3(&m_vPlaneNormal) * fDot);
}

_float CCell::Compute_Height(const _fvector& vLocalPos)
{
	_vector vPlane = XMPlaneFromPoints(
		XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::A)]),
		XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::B)]),
		XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::C)]));

	return (vPlane.m128_f32[0] * -1.f * vLocalPos.m128_f32[0] + vPlane.m128_f32[2] * -1.f * vLocalPos.m128_f32[2] + vPlane.m128_f32[3] * -1.f) / vPlane.m128_f32[1] ;
}

#ifdef _DEBUG
HRESULT CCell::Save_Cell(ofstream& Output)
{
	CELL Cell = {};

	memcpy(&Cell.vPositions, m_Points, sizeof(_float3) * ENUM_CLASS(POINTS::END));
	Cell.iType = m_iType;
	Output.write(reinterpret_cast<_char*>(&Cell), sizeof(CELL));

	return S_OK;
}
HRESULT CCell::Render(CShader* pShader, _bool isWire)
{
	_float4 vColor = _float4(0.f, 1.f, 0.f, 1.f);

	pShader->Bind_Value("g_Color", &vColor, sizeof(_float4));
	pShader->Begin(isWire);
	if (false == isWire)
	{
		pShader->Begin(0);
		m_pVIBuffer->Bind_Resources();
		m_pVIBuffer->Render();
		pShader->Begin(1);
	}
	else
	{
		m_pVIBuffer->Bind_Resources();
		m_pVIBuffer->Render();
	}

	return S_OK;
}
#endif

void CCell::Compute_Line()
{
	_float fAB_Y{}, fBC_Y{}, fCA_Y{};

	fAB_Y = (m_Points[ENUM_CLASS(POINTS::A)].y + m_Points[ENUM_CLASS(POINTS::B)].y) * 0.5f;
	fBC_Y = (m_Points[ENUM_CLASS(POINTS::B)].y + m_Points[ENUM_CLASS(POINTS::C)].y) * 0.5f;
	fCA_Y = (m_Points[ENUM_CLASS(POINTS::C)].y + m_Points[ENUM_CLASS(POINTS::A)].y) * 0.5f;

	// High / Low Line
	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	_float fDotAB{}, fDotBC{}, fDotCA{};

	_vector vAB = XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::B)]) - XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::A)]);
	_vector vBC = XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::C)]) - XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::B)]);
	_vector vCA = XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::A)]) - XMLoadFloat3(&m_Points[ENUM_CLASS(POINTS::C)]);

	fDotAB = fabsf(XMVector3Dot(vWorldUp, XMVector3Normalize(vAB)).m128_f32[0]);
	fDotBC = fabsf(XMVector3Dot(vWorldUp, XMVector3Normalize(vBC)).m128_f32[0]);
	fDotCA = fabsf(XMVector3Dot(vWorldUp, XMVector3Normalize(vCA)).m128_f32[0]);

	_float fMaxDot = max(fDotAB, max(fDotBC, fDotCA));

	if (fDotAB == fMaxDot)
	{
		if (fBC_Y > fCA_Y)
		{
			m_eHighLine = LINE::BC;
			m_eLowLine = LINE::CA;
		}
		else
		{
			m_eHighLine = LINE::BC;
			m_eLowLine = LINE::CA;
		}
	}
	else if (fDotBC == fMaxDot)
	{
		if (fCA_Y > fAB_Y)
		{
			m_eHighLine = LINE::CA;
			m_eLowLine = LINE::AB;
		}
		else
		{
			m_eHighLine = LINE::AB;
			m_eLowLine = LINE::CA;
		}
	}
	else
	{
		if (fAB_Y > fBC_Y)
		{
			m_eHighLine = LINE::AB;
			m_eLowLine = LINE::BC;
		}
		else
		{
			m_eHighLine = LINE::BC;
			m_eLowLine = LINE::AB;
		}
	}
}

void CCell::Compute_Slide(_uint iPointIndex, _float3* pSlide)
{
	_vector vLine = XMVector3Normalize(XMLoadFloat3(&m_Points[(iPointIndex + 1) % 3]) - XMLoadFloat3(&m_Points[iPointIndex]));


}

CCell* CCell::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CELL& pCell, _int iIndex)
{
	CCell* pInstance = new CCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pCell, iIndex)))
	{
		MSG_BOX("Failed to Create : Cell");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCell::Free()
{
	__super::Free();

#ifdef _DEBUG
	Safe_Release(m_pVIBuffer);
#endif
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
