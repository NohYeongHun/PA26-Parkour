#include "EnginePch.h"
#include "VIBuffer_Sphere.h"

CVIBuffer_Sphere::CVIBuffer_Sphere(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Sphere::CVIBuffer_Sphere(const CVIBuffer_Sphere& Prototype)
	: CVIBuffer{ Prototype }
{
}

HRESULT CVIBuffer_Sphere::Initialize_Prototype()
{
#pragma region VERTEX
	// 구의 정밀도 설정
	const _uint iStacks = 20;  // 가로 줄
	const _uint iSlices = 20;  // 세로 줄

	m_iNumVertices = (iStacks + 1) * (iSlices + 1);
	m_iVertexStride = sizeof(VTXPOSTEX);
	m_iNumVertexBuffers = 1;

	D3D11_BUFFER_DESC   VBDesc = {};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	VTXPOSTEX* pVertices = new VTXPOSTEX[m_iNumVertices];

	const _float fRadius = 0.5f; // 반지름 0.5 (지름 1.0)
	const _float fStackAngle = XM_PI / iStacks;
	const _float fSliceAngle = (XM_PI * 2.0f) / iSlices;

	_uint iIndex = 0;

	// 구면 좌표계를 이용한 정점 생성
	for (_uint i = 0; i <= iStacks; ++i)
	{
		_float phi = i * fStackAngle; // 0 ~ PI
		_float y = fRadius * cosf(phi);
		_float r = fRadius * sinf(phi); // 해당 높이에서의 반지름

		for (_uint j = 0; j <= iSlices; ++j)
		{
			_float theta = j * fSliceAngle; // 0 ~ 2PI
			_float x = r * cosf(theta);
			_float z = r * sinf(theta);

			// 위치 설정
			pVertices[iIndex].vPosition = _float3(x, y, z);

			// UV 설정 (텍스처 매핑용)
			pVertices[iIndex].vTexcoord = _float2((_float)j / iSlices, (_float)i / iStacks);

			iIndex++;
		}
	}

	D3D11_SUBRESOURCE_DATA VBInitialData = {};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
#pragma endregion

#pragma region INDEX
	m_iNumIndices = iStacks * iSlices * 6; // 사각형(삼각형 2개) * 개수
	m_iIndexStride = 2; // 16bit
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_BUFFER_DESC IBDesc = {};
	IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = m_iIndexStride;

	_ushort* pIndices = new _ushort[m_iNumIndices];
	_uint iIndicesIndex = 0;

	for (_uint i = 0; i < iStacks; ++i)
	{
		for (_uint j = 0; j < iSlices; ++j)
		{
			// 현재 스택과 다음 스택의 정점 인덱스 계산
			_uint iTopLeft = i * (iSlices + 1) + j;
			_uint iTopRight = iTopLeft + 1;
			_uint iBottomLeft = (i + 1) * (iSlices + 1) + j;
			_uint iBottomRight = iBottomLeft + 1;

			// 삼각형 1 (CCW)
			pIndices[iIndicesIndex++] = (_ushort)iTopLeft;
			pIndices[iIndicesIndex++] = (_ushort)iTopRight;
			pIndices[iIndicesIndex++] = (_ushort)iBottomLeft;

			// 삼각형 2 (CCW)
			pIndices[iIndicesIndex++] = (_ushort)iBottomLeft;
			pIndices[iIndicesIndex++] = (_ushort)iTopRight;
			pIndices[iIndicesIndex++] = (_ushort)iBottomRight;
		}
	}

	D3D11_SUBRESOURCE_DATA IBInitialData = {};
	IBInitialData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);
#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Sphere::Initialize_Clone(void* pArg)
{
	return S_OK;
}

CVIBuffer_Sphere* CVIBuffer_Sphere::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVIBuffer_Sphere* pInstance = new CVIBuffer_Sphere(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : VIBuffer_Sphere");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Sphere::Clone(void* pArg)
{
	CVIBuffer_Sphere* pClone = new CVIBuffer_Sphere(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : VIBuffer_Sphere (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CVIBuffer_Sphere::Free()
{
	__super::Free();
}