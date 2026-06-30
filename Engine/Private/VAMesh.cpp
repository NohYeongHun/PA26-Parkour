#include "EnginePch.h"
#include "VAMesh.h"

#include "HdrTexture.h"

CVAMesh::CVAMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer { pDevice, pContext }
{
}

CVAMesh::CVAMesh(const CVAMesh& Prototype)
	: CVIBuffer { Prototype },
	m_pHdrTexture { Prototype.m_pHdrTexture }
{
	Safe_AddRef(m_pHdrTexture);
}

_float CVAMesh::Get_MaxFrame(_uint iTextureIndex)
{
	return m_pHdrTexture->Get_MaxFrame(iTextureIndex);
}

HRESULT CVAMesh::Initialize_Prototype(const _tchar* pFilePath, _fmatrix PreTransformMatrix, _uint iNumAnimation)
{
	ifstream EMeshFile(WStringToString(pFilePath), ios::binary);
	if (false == EMeshFile.is_open())
	{
		MSG_BOX("Failed Open : VAMesh");
		return E_FAIL;
	}

	_uint MeshIndex = {};
	EMeshFile.read(reinterpret_cast<_char*>(&MeshIndex), sizeof(_uint));

	VTX_VAMESH* pVertices = { nullptr };
	EMeshFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
	pVertices = new VTX_VAMESH[m_iNumVertices];

	_uint* pIndices = { nullptr };
	EMeshFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
	m_iNumIndices = m_iNumIndices * 3;
	pIndices = new _uint[m_iNumIndices];

	_uint MaterialIndex = {};
	EMeshFile.read(reinterpret_cast<_char*>(&MaterialIndex), sizeof(_uint));

	EMeshFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTX_VAMESH) * m_iNumVertices);
	EMeshFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

	for (size_t i = 0; i < m_iNumVertices; ++i)
	{
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vTangent, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));
	}

	m_iNumVertexBuffers = 1;
	m_iVertexStride = sizeof(VTX_VAMESH);

	D3D11_BUFFER_DESC VBDesc = {};

	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = sizeof(VTX_VAMESH);
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;

	D3D11_SUBRESOURCE_DATA VBInitialData = {};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_iIndexStride = sizeof(_uint);

	D3D11_BUFFER_DESC IBDesc = {};

	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = sizeof(_uint);
	IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;

	D3D11_SUBRESOURCE_DATA IBInitialData = {};
	IBInitialData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

	// Create Texture
	_tchar szDrivePath[MAX_PATH] = {};
	_tchar szDirPath[MAX_PATH] = {};
	_tchar szFileName[MAX_PATH] = {};

	_wsplitpath_s(pFilePath, szDrivePath, MAX_PATH, szDirPath, MAX_PATH, szFileName, MAX_PATH, nullptr, 0);

	_tchar szFilePath[MAX_PATH] = {};
	wsprintf(szFilePath, TEXT("%s%s%s%s"), szDrivePath, szDirPath, szFileName, TEXT("%d.hdr"));

	m_pHdrTexture = CHdrTexture::Create(m_pDevice, m_pContext, szFilePath, iNumAnimation);
	ASSERT_CRASH(m_pHdrTexture);

	return S_OK;
}

HRESULT CVAMesh::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

    return S_OK;
}

HRESULT CVAMesh::Bind_VAT(CShader* pShader, const _char* pConstantName, _uint iTextureIndex)
{
	return m_pHdrTexture->Bind_Shader_Resource(pShader, pConstantName, iTextureIndex);;
}

CVAMesh* CVAMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pFilePath, _fmatrix PreTransformMatrix, _uint iNumAnimation)
{
	CVAMesh* pInstance = new CVAMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pFilePath, PreTransformMatrix, iNumAnimation)))
		CRASH("VAMesh");

    return pInstance;
}

CComponent* CVAMesh::Clone(void* pArg)
{
	CVAMesh* pClone = new CVAMesh(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("VAMesh (Clone)");

	return pClone;
}

void CVAMesh::Free()
{
	__super::Free();

	Safe_Release(m_pHdrTexture);
}
