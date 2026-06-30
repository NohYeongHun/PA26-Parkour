#include "EnginePch.h"
#include "Mesh.h"

#include "Bone.h"
#include "ShapeKey.h"
#include "Shader.h"
#include "ComputeShader.h"

#include"VIBuffer_Cube.h"

CMesh::CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CMesh::CMesh(const CMesh& Prototype)
	: CVIBuffer{ Prototype }
	, m_VertexPositions{ Prototype.m_VertexPositions }
	, m_Indices{ Prototype.m_Indices }
	, m_eModelType{ Prototype.m_eModelType }
	, m_iMaterialIndex{ Prototype.m_iMaterialIndex }
	, m_iNumBones{ Prototype.m_iNumBones }
	, m_BoneIndices{ Prototype.m_BoneIndices }
	, m_OffsetMatrices{ Prototype.m_OffsetMatrices }
	, m_ShapeKeys{ Prototype.m_ShapeKeys }
	, m_iNumAnimMeshes{ Prototype.m_iNumAnimMeshes }
	, m_pRestPoseVertices{ Prototype.m_pRestPoseVertices }
	, m_Buffers{ Prototype.m_Buffers }
	, m_SRVs{ Prototype.m_SRVs }
{

}

void CMesh::Copy_BoneMatrices(_float4x4* pOutMatrices, _uint iNumBones)
{
	if (m_iNumBones != iNumBones ||
		nullptr == pOutMatrices)
		return;

	memcpy(pOutMatrices, m_BoneMatrices, sizeof(_float4x4) * iNumBones);
}



HRESULT CMesh::Initialize_Prototype(MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile)
{
	m_eModelType = eType;

	if (MODELTYPE::NONANIM == eType)
	{
		if (FAILED(Ready_Mesh_NonAnim(PreTransformMatrix, InputFile)))
			return E_FAIL;
	}
	else if (MODELTYPE::ANIM == eType)
	{
		if (FAILED(Ready_Mesh_Anim(Bones, PreTransformMatrix, InputFile)))
			return E_FAIL;
	}
	else if (MODELTYPE::CHARACTER == eType)
	{
		// 1. 파일에서 데이터 읽어오기.
		if (FAILED(Ready_Mesh_Character(Bones, PreTransformMatrix, InputFile)))
			return E_FAIL;

		//// 1. CHARACTER 형태만 Shared Buffer로 Buffer들을 소유합니다.
		//if (FAILED(Ready_SharedBuffers()))
		//	return E_FAIL;


		//if (FAILED(Ready_InstanceBuffers()))
		//	return E_FAIL;
	}

	return S_OK;
}

HRESULT CMesh::Initialize_Clone(void* pArg)
{
	// Model Type이 캐릭터인 경우만. Instance Buffer를 생성.
	/*if (MODELTYPE::CHARACTER == m_eModelType)
	{
		if (FAILED(Ready_InstanceBuffers()))
			return E_FAIL;
	}*/

	return S_OK;
}


void CMesh::Update_Morph_CPU(const vector<_float>& vShapeKeyWeights)
{
	if (nullptr == m_pRestPoseVertices)
		return;

	// 1. 작업을 위해 원본(Rest Pose)을 임시 버퍼로 복사해옵니다.
	//    (매 프레임 원본에서 다시 계산해야 누적되지 않고 정확함)
	vector<VTXANIMMESH> vTempVertices(m_iNumVertices);
	memcpy(vTempVertices.data(), m_pRestPoseVertices, sizeof(VTXANIMMESH) * m_iNumVertices);

	// 2. 활성화된 쉐이프 키들을 순회하며 Delta 값을 더해줍니다.
	for (auto& pKey : m_ShapeKeys)
	{
		// 이 키의 글로벌 인덱스를 가져옴 (Model의 Weight 배열 인덱스)
		_uint iGlobalIndex = pKey->Get_GlobalWeightIndex();

		// 범위 체크
		if (iGlobalIndex >= vShapeKeyWeights.size()) continue;

		_float fWeight = vShapeKeyWeights[iGlobalIndex];

		// 가중치가 0이면 계산할 필요 없음
		if (fWeight <= 0.001f) continue;

		// --- 실제 연산 (Linear Interpolation) ---
		const vector<_float3>& DeltaPos = pKey->Get_DeltaPositions();
		const vector<_float3>& DeltaNormals = pKey->Get_DeltaNormals();
		_uint iNumKeyVerts = pKey->Get_NumVertices();

		// 정점 개수가 안 맞으면 큰일남
		if (iNumKeyVerts != m_iNumVertices) continue;

		for (_uint i = 0; i < m_iNumVertices; ++i)
		{
			// Pos += Delta * Weight
			vTempVertices[i].vPosition.x += DeltaPos[i].x * fWeight;
			vTempVertices[i].vPosition.y += DeltaPos[i].y * fWeight;
			vTempVertices[i].vPosition.z += DeltaPos[i].z * fWeight;

			// Normal += DeltaNormal * Weight (노말도 변해야 그림자가 자연스러움)
			if (!DeltaNormals.empty())
			{
				vTempVertices[i].vNormal.x += DeltaNormals[i].x * fWeight;
				vTempVertices[i].vNormal.y += DeltaNormals[i].y * fWeight;
				vTempVertices[i].vNormal.z += DeltaNormals[i].z * fWeight;
			}
		}
	}

	// 3. 노말 벡터 정규화 (옵션: 퀄리티 위해선 하는게 좋음)

	for (auto& Vtx : vTempVertices)
		XMStoreFloat3(&Vtx.vNormal, XMVector3Normalize(XMLoadFloat3(&Vtx.vNormal)));

	// 4. 계산이 끝난 정점들을 실제 GPU 버퍼(VB)에 덮어씌웁니다 (Map/Unmap).
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	ZeroMemory(&MappedSubResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	// DYNAMIC 버퍼는 MAP_WRITE_DISCARD를 써야 빠름 (기존 내용 버리고 새로 씀)
	if (FAILED(m_pContext->Map(m_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource)))
		return;

	memcpy(MappedSubResource.pData, vTempVertices.data(), sizeof(VTXANIMMESH) * m_iNumVertices);
	m_pContext->Unmap(m_pVB, 0);
}

void CMesh::Compute_Morph(CComputeShader* pMorphComputeShaderCom, ID3D11ShaderResourceView* pWeightSRV)
{
	// CHARACTER Type이 아니거나, Shared, Instance Buffer가 준비되지 않았다면 리턴.
	if (m_eModelType != MODELTYPE::CHARACTER) return;
	if (!m_IsSharedReady || !m_IsInstanceReady) return;
	if (m_ShapeKeys.empty() || m_iNumAnimMeshes == 0) return;

	// 1. 상수 버퍼(CB) 업데이트 (정점 개수 등)
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	if (SUCCEEDED(m_pContext->Map(m_Buffers[BUF_MORPH_INFOCB], 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource)))
	{
		MORPH_CBINFO* pInfo = static_cast<MORPH_CBINFO*>(MappedSubResource.pData);
		pInfo->iNumVertices = m_iNumVertices;
		pInfo->iNumActiveMorphs = m_iNumAnimMeshes; // CModel에서 받아오거나 ShapeKeys 사이즈로 설정
		// ※ 주의: pWeightSRV에 담긴 개수와 일치해야 함. 보통 Model의 전체 Key 개수.

		m_pContext->Unmap(m_Buffers[BUF_MORPH_INFOCB], 0);
	}


	pMorphComputeShaderCom->Set_SRV("g_BaseVertices", m_SRVs[BUF_BASE_VERTICES]);
	pMorphComputeShaderCom->Set_SRV("g_AllMorphDeltas", m_SRVs[BUF_MORPH_DELTAS]);
	pMorphComputeShaderCom->Set_SRV("g_MorphWeights", pWeightSRV); // 외부에서 주입!
	pMorphComputeShaderCom->Set_UAV("g_OutVertices", m_UAVs[BUF_MORPH_OUTPUT]);
	pMorphComputeShaderCom->Set_ConstantBuffer("MorphInfoCB", m_Buffers[BUF_MORPH_INFOCB]);

	// 3. Dispatch => 스레드 그룹 계산
	_uint iGroupX = (m_iNumVertices + pMorphComputeShaderCom->Get_ThreadInfo().iThreadGroupX - 1) / pMorphComputeShaderCom->Get_ThreadInfo().iThreadGroupX;
	pMorphComputeShaderCom->Dispatch(iGroupX, 1, 1); // UnBind 자동으로 수행.
}




#ifdef _DEBUG

_bool CMesh::Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance)
{
	_float fMin = FLT_MAX;
	for (size_t i = 0; i < m_Indices.size() - 2; i += 3)
	{
		_float3 vPos[3] = {
			m_VertexPositions[m_Indices[i]],
			m_VertexPositions[m_Indices[i + 1]],
			m_VertexPositions[m_Indices[i + 2]],
		};
		_float fDistance = {};
		if (true == TriangleTests::Intersects(vRayPos, vRayDir,
			XMVectorSetW(XMLoadFloat3(&vPos[0]), 1.f),
			XMVectorSetW(XMLoadFloat3(&vPos[1]), 1.f),
			XMVectorSetW(XMLoadFloat3(&vPos[2]), 1.f), fDistance))
		{
			if (fMin > fDistance)
				fMin = fDistance;
		}
	}
	if (fMin < FLT_MAX)
	{
		*pDistance = fMin;
		return true;
	}

	return false;
}
#endif

// 
HRESULT CMesh::Bind_BoneMatrices(CShader* pShaderCom, const _char* pConstantName, const vector<class CBone*>& Bones)
{
	for (size_t i = 0; i < m_iNumBones; ++i)
	{
		XMStoreFloat4x4(&m_BoneMatrices[i], XMLoadFloat4x4(&m_OffsetMatrices[i]) * XMLoadFloat4x4(Bones[m_BoneIndices[i]]->Get_CombinedTransformationMatrix()));
	}

	return pShaderCom->Bind_Matrices(pConstantName, m_BoneMatrices, m_iNumBones);
}

// Bind => Compute Shader 연산 결과를 Skinning Shader에 바인딩해줍니다.
HRESULT CMesh::Bind_MorphedResult(CShader* pShaderCom, const _char* pConstantName)
{
	if (MODELTYPE::CHARACTER != m_eModelType) return E_FAIL;
	if (!m_IsInstanceReady) return E_FAIL;

	if (FAILED(pShaderCom->Bind_SRV(pConstantName, m_SRVs[BUF_MORPH_OUTPUT])))
		return E_FAIL;

	return S_OK;
}


HRESULT CMesh::Ready_Mesh_NonAnim(_fmatrix PreTransformMatrix, ifstream& InputFile)
{
	VTXMESH* pVertices = { nullptr };
	_uint* pIndices = { nullptr };

	InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
	pVertices = new VTXMESH[m_iNumVertices];
	InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
	m_iNumIndices = m_iNumIndices * 3;
	pIndices = new _uint[m_iNumIndices];
	InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));

	InputFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXMESH) * m_iNumVertices);
	InputFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

	for (size_t i = 0; i < m_iNumVertices; ++i)
	{
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vTangent, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));
	}

	m_iVertexStride = sizeof(VTXMESH);
	m_iNumVertexBuffers = 1;

	D3D11_BUFFER_DESC   VBDesc = {};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	D3D11_SUBRESOURCE_DATA VBInitialData = {};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
#pragma endregion

#pragma region INDEX
	m_iIndexStride = 4;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_BUFFER_DESC IBDesc = {};
	IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = m_iIndexStride;

	D3D11_SUBRESOURCE_DATA IBInitialData = {};
	IBInitialData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

	return S_OK;
}

HRESULT CMesh::Ready_Mesh_Anim(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile)
{
	VTXANIMMESH* pVertices = { nullptr };
	_uint* pIndices = { nullptr };

	InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
	pVertices = new VTXANIMMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);
	InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
	m_iNumIndices = m_iNumIndices * 3;
	pIndices = new _uint[m_iNumIndices];
	InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));
	InputFile.read(reinterpret_cast<_char*>(&m_iNumBones), sizeof(_uint));
	for (size_t i = 0; i < m_iNumBones; ++i)
	{
		_uint iLength = {};
		// Bone Name Length
		InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
		_char szName[MAX_PATH] = {};
		// Bone Name
		InputFile.read(szName, iLength);
		auto iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool {
			return 0 == strcmp(szName, pBone->Get_Name());
			});

		if (iter == Bones.end())
			return E_FAIL;

		m_BoneIndices.push_back(iter - Bones.begin());

		// OffsetMatrix
		_float4x4 OffsetMatrix = {};
		InputFile.read(reinterpret_cast<_char*>(&OffsetMatrix), sizeof(_float4x4));
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));
		m_OffsetMatrices.push_back(OffsetMatrix);
	}

	if (0 == m_iNumBones)
	{
		_float4x4 OffsetMatrix = {};
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());
		m_OffsetMatrices.push_back(OffsetMatrix);
		m_BoneIndices.push_back(0);
	}

	InputFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXANIMMESH) * m_iNumVertices);
	InputFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);


	m_iVertexStride = sizeof(VTXANIMMESH);
	m_iNumVertexBuffers = 1;

	D3D11_BUFFER_DESC   VBDesc = {};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	D3D11_SUBRESOURCE_DATA VBInitialData = {};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
#pragma endregion

#pragma region INDEX
	m_iIndexStride = 4;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_BUFFER_DESC IBDesc = {};
	IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = m_iIndexStride;

	D3D11_SUBRESOURCE_DATA IBInitialData = {};
	IBInitialData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

	return S_OK;
}

HRESULT CMesh::Ready_Mesh_Character(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile)
{

	VTXANIMMESH* pVertices = { nullptr };
	_uint* pIndices = { nullptr };

	// 1. 기본 메쉬 정보 저장 Vertcies ~ mNumBones
	InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
	pVertices = new VTXANIMMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);
	InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
	m_iNumIndices = m_iNumIndices * 3;
	pIndices = new _uint[m_iNumIndices];
	InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));
	InputFile.read(reinterpret_cast<_char*>(&m_iNumBones), sizeof(_uint));

	// 2. Shape key 정보 저장 시작.
	InputFile.read(reinterpret_cast<_char*>(&m_iNumAnimMeshes), sizeof(_uint));

	// 3. Shape Key 개수만큼 순회돌기.
	for (_uint i = 0; i < m_iNumAnimMeshes; ++i)
	{
		// 1. Shape Key 이름 저장하기.
		_uint iLength = {};
		_char szName[MAX_PATH] = {};
		InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
		InputFile.read(szName, iLength);

		// 2. 길이만큼만 생성해서 문자열로 저장.
		_string strKeyName = string(szName, iLength);

		// 3. 정점 개수 저장.
		_uint iNumVertices = 0;
		InputFile.read(reinterpret_cast<_char*>(&iNumVertices), sizeof(_uint));

		// 정점 개수가 다르면 에러.
		if (iNumVertices != m_iNumVertices)
			CRASH("Facial Vertices Not Equal");

		// 4. Delta Position;
		vector<_float3> vecDeltaPos(iNumVertices);
		InputFile.read(reinterpret_cast<_char*>(vecDeltaPos.data()), sizeof(_float3) * iNumVertices);

		// 5. Delta Normal 
		_bool bHasNormal = { false };
		InputFile.read(reinterpret_cast<_char*>(&bHasNormal), sizeof(_bool));

		vector<_float3> vecDeltaNormal(iNumVertices, _float3(0, 0, 0)); // 값 초기화.
		if (bHasNormal) // 노말이 존재한다면?
			InputFile.read(reinterpret_cast<_char*>(vecDeltaNormal.data()), sizeof(_float3) * iNumVertices);

		// 6. CShapeKey 생성 및 등록
		CShapeKey* pShapeKey = CShapeKey::Create(m_pDevice, m_pContext, szName, iNumVertices, vecDeltaPos, vecDeltaNormal, PreTransformMatrix);

		// 7. ShapeKey를 등록.
		m_ShapeKeys.emplace_back(pShapeKey);
	}


	// 8. 본 내용 저장.
	for (size_t i = 0; i < m_iNumBones; ++i)
	{
		_uint iLength = {};
		// Bone Name Length
		InputFile.read(reinterpret_cast<_char*>(&iLength), sizeof(_uint));
		_char szName[MAX_PATH] = {};
		// Bone Name
		InputFile.read(szName, iLength);
		auto iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool {
			return 0 == strcmp(szName, pBone->Get_Name());
			});

		if (iter == Bones.end())
			return E_FAIL;

		m_BoneIndices.push_back(iter - Bones.begin());

		// OffsetMatrix
		_float4x4 OffsetMatrix = {};
		InputFile.read(reinterpret_cast<_char*>(&OffsetMatrix), sizeof(_float4x4));
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));
		m_OffsetMatrices.push_back(OffsetMatrix);
	}

	if (0 == m_iNumBones)
	{
		_float4x4 OffsetMatrix = {};
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());
		m_OffsetMatrices.push_back(OffsetMatrix);
		m_BoneIndices.push_back(0);
	}

	InputFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXANIMMESH) * m_iNumVertices);
	InputFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

	m_iVertexStride = sizeof(VTXANIMMESH);
	m_iNumVertexBuffers = 1;

	// CPU 연산을 위해 원본 데이터를 힙 메모리에 백업.
	m_pRestPoseVertices = new VTXANIMMESH[m_iNumVertices];
	memcpy(m_pRestPoseVertices, pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);

	D3D11_BUFFER_DESC   VBDesc = {};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	//VBDesc.Usage = D3D11_USAGE_DYNAMIC; // 수정 가능.
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	//VBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	D3D11_SUBRESOURCE_DATA VBInitialData = {};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;



	Safe_Delete_Array(pVertices);

#pragma region INDEX
	m_iIndexStride = 4;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_BUFFER_DESC IBDesc = {};
	IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = m_iIndexStride;

	D3D11_SUBRESOURCE_DATA IBInitialData = {};
	IBInitialData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);
#pragma endregion



	return S_OK;
}

HRESULT CMesh::Ready_Mesh_Map(_fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos, _float* MaxPos)
{
	VTXMESH* pVertices = { nullptr };
	_uint* pIndices = { nullptr };

	InputFile.read(reinterpret_cast<_char*>(&m_iNumVertices), sizeof(_uint));
	pVertices = new VTXMESH[m_iNumVertices];
	InputFile.read(reinterpret_cast<_char*>(&m_iNumIndices), sizeof(_uint));
	m_iNumIndices = m_iNumIndices * 3;
	pIndices = new _uint[m_iNumIndices];
	InputFile.read(reinterpret_cast<_char*>(&m_iMaterialIndex), sizeof(_uint));

	InputFile.read(reinterpret_cast<_char*>(pVertices), sizeof(VTXMESH) * m_iNumVertices);
	InputFile.read(reinterpret_cast<_char*>(pIndices), sizeof(_uint) * m_iNumIndices);

	for (size_t i = 0; i < m_iNumVertices; ++i)
	{
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), PreTransformMatrix));

		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vTangent, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vTangent), PreTransformMatrix));
		XMStoreFloat3(&pVertices[i].vBinormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vBinormal), PreTransformMatrix));

		// Mesh ShapeContainer
		m_VertexPositions.push_back(pVertices[i].vPosition);
#ifdef _DEBUG

		MaxPos[0] = max(pVertices[i].vPosition.x, MaxPos[0]);
		MaxPos[1] = max(pVertices[i].vPosition.y, MaxPos[1]);
		MaxPos[2] = max(pVertices[i].vPosition.z, MaxPos[2]);

		MinPos[0] = min(pVertices[i].vPosition.x, MinPos[0]);
		MinPos[1] = min(pVertices[i].vPosition.y, MinPos[1]);
		MinPos[2] = min(pVertices[i].vPosition.z, MinPos[2]);
#endif
	}

	m_iVertexStride = sizeof(VTXMESH);
	m_iNumVertexBuffers = 1;

	D3D11_BUFFER_DESC   VBDesc = {};
	VBDesc.ByteWidth = m_iNumVertices * m_iVertexStride;
	VBDesc.Usage = D3D11_USAGE_DEFAULT;
	VBDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VBDesc.CPUAccessFlags = 0;
	VBDesc.MiscFlags = 0;
	VBDesc.StructureByteStride = m_iVertexStride;

	D3D11_SUBRESOURCE_DATA VBInitialData = {};
	VBInitialData.pSysMem = pVertices;

	if (FAILED(m_pDevice->CreateBuffer(&VBDesc, &VBInitialData, &m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
#pragma endregion

	// Mesh Shape??Container
	for (size_t i = 0; i < m_iNumIndices; ++i)
		m_Indices.push_back(pIndices[i]);

#pragma region INDEX
	m_iIndexStride = 4;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	D3D11_BUFFER_DESC IBDesc = {};
	IBDesc.ByteWidth = m_iNumIndices * m_iIndexStride;
	IBDesc.Usage = D3D11_USAGE_DEFAULT;
	IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IBDesc.CPUAccessFlags = 0;
	IBDesc.MiscFlags = 0;
	IBDesc.StructureByteStride = m_iIndexStride;

	D3D11_SUBRESOURCE_DATA IBInitialData = {};
	IBInitialData.pSysMem = pIndices;

	if (FAILED(m_pDevice->CreateBuffer(&IBDesc, &IBInitialData, &m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

	return S_OK;
}


// Prototype에서 Shared Buffers 생성. => 모든 복제체가 하나의 Buffer를 공유합니다. (SRV 겠지요)
HRESULT CMesh::Ready_SharedBuffers_ForMorph()
{
	// 0. 이미 처리했다면?
	if (m_IsSharedReady)
		return E_FAIL;

	// 1. 예외 처리.
	if (m_iNumVertices == 0 || m_pRestPoseVertices == nullptr)
		return E_FAIL;

	// 0. 생성 이전 vector 초기화
	m_Buffers.resize(BUFFER_TYPE::BUF_END);
	m_SRVs.resize(BUFFER_TYPE::BUF_END);
	m_UAVs.resize(BUFFER_TYPE::BUF_END);

	// 2. Base Vertex Buffer 생성 (t0)
	if (FAILED(Create_BaseVertexBuffer()))
		return E_FAIL;

	// 3. Morph Delta Buffer 생성 (t1)
	if (FAILED(Create_DeltaBuffer()))
		return E_FAIL;

	m_IsSharedReady = true;

	return S_OK;
}



// Clone에서 Instance Buffers 생성 => 모든 복제체는 고유의 CB와 UAV를 소유해야 합니다.
HRESULT CMesh::Ready_InstanceBuffers_ForMorph()
{
	// 0. 이미 처리했다면?
	if (m_IsInstanceReady) return E_FAIL;
	if (m_eModelType != MODELTYPE::CHARACTER) return S_OK;

	// 1. Output Buffer (결과 저장용) 생성.
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
	BufferDesc.ByteWidth = sizeof(BASE_VERTEX_INFO) * m_iNumVertices; // 구조체 크기 24
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = sizeof(BASE_VERTEX_INFO);

#ifdef _DEBUG
	cout << "Mesh BASE_VERTEX_INFO Buffer : " << BufferDesc.ByteWidth << endl;
#endif // _DEBUG


	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, nullptr, &m_Buffers[BUF_MORPH_OUTPUT])))
		return E_FAIL;

	// 2. UAV 생성 (Compute Shader 용 u0)
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
	ZeroMemory(&UAVDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	UAVDesc.Buffer.NumElements = m_iNumVertices;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_Buffers[BUF_MORPH_OUTPUT], &UAVDesc, &m_UAVs[BUF_MORPH_OUTPUT])))
		return E_FAIL;

	// 3. SRV 생성 (나중에 Vertex Shader에서 읽을 때 필요)
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.NumElements = m_iNumVertices;

	if (FAILED(m_pDevice->CreateShaderResourceView(m_Buffers[BUF_MORPH_OUTPUT], &SRVDesc, &m_SRVs[BUF_MORPH_OUTPUT])))
		return E_FAIL;

	// 4. Morph Info Constant Buffer 생성 (b0)
	// - 메쉬마다 정점 개수가 다르므로 메쉬가 가지고 있는 게 편함
	D3D11_BUFFER_DESC CBDesc;
	ZeroMemory(&CBDesc, sizeof(D3D11_BUFFER_DESC));
	CBDesc.ByteWidth = sizeof(MORPH_CBINFO);
	CBDesc.Usage = D3D11_USAGE_DYNAMIC;
	CBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	CBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&CBDesc, nullptr, &m_Buffers[BUF_MORPH_INFOCB])))
		return E_FAIL;

	m_IsInstanceReady = true;

	return S_OK;
}

HRESULT CMesh::Create_BaseVertexBuffer()
{

	// 1. 기존 버텍스 버퍼. 정점 정보.
	vector<BASE_VERTEX_INFO> vBaseVertices(m_iNumVertices);

	for (_uint i = 0; i < m_iNumVertices; ++i)
	{
		vBaseVertices[i].vPosition = m_pRestPoseVertices[i].vPosition;
		vBaseVertices[i].vNormal = m_pRestPoseVertices[i].vNormal;
	}

	// 2. 버퍼 생성.
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
	BufferDesc.ByteWidth = sizeof(BASE_VERTEX_INFO) * m_iNumVertices;
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 읽기 전용, 절대 안 바뀜
	BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = sizeof(BASE_VERTEX_INFO);

	// 3. 데이터 바인딩
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = vBaseVertices.data();

	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, &InitData, &m_Buffers[BUF_BASE_VERTICES])))
		return E_FAIL;

	// 4. SRV 생성
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.NumElements = m_iNumVertices;

	if (FAILED(m_pDevice->CreateShaderResourceView(m_Buffers[BUF_BASE_VERTICES], &SRVDesc, &m_SRVs[BUF_BASE_VERTICES])))
		return E_FAIL;

	return S_OK;
}

HRESULT CMesh::Create_DeltaBuffer()
{
	if (m_ShapeKeys.empty()) return S_OK; // 쉐이프키 없으면 패스

	// 1. 최대 쉐이프키 개수 정의
	_uint iMaxGlobalKeys = 0;
	for (auto& pKey : m_ShapeKeys)
		iMaxGlobalKeys = max(iMaxGlobalKeys, pKey->Get_GlobalWeightIndex());

	iMaxGlobalKeys += 1; // 인덱스는 0부터 시작하므로 개수는 +1
	_uint iTotalDataCount = iMaxGlobalKeys * m_iNumVertices;

	// 2. 데이터 패킹 (Flattening)
	// 기본값 0으로 초기화 (빈 공간은 Delta가 0이어야 합니다.)
	vector<MORPH_DELTA_INFO> vDeltas(iTotalDataCount, { {0.f,0.f,0.f}, {0.f,0.f,0.f} });

	for (auto& pKey : m_ShapeKeys)
	{
		_uint iGlobalIndex = pKey->Get_GlobalWeightIndex();
		_uint iStartOffset = iGlobalIndex * m_iNumVertices;

		const auto& srcPos = pKey->Get_DeltaPositions();
		const auto& srcNormal = pKey->Get_DeltaNormals();

		_bool bHasNormal = !srcNormal.empty();

		for (_uint i = 0; i < m_iNumVertices; ++i)
		{
			// => 구조체 멤버 vDPoseDelta, vNormalDelta에 값 대입.
			vDeltas[iStartOffset + i].vPosDelta = srcPos[i];
			if (bHasNormal)
				vDeltas[iStartOffset + i].vNormalDelta = srcNormal[i];
		}
	}

	// 3. 버퍼 생성.
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory(&BufferDesc, sizeof(D3D11_BUFFER_DESC));
	BufferDesc.ByteWidth = sizeof(MORPH_DELTA_INFO) * static_cast<_uint>(vDeltas.size());
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = sizeof(MORPH_DELTA_INFO);

	// 4. 데이터 바인딩
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = vDeltas.data();

	if (FAILED(m_pDevice->CreateBuffer(&BufferDesc, &InitData, &m_Buffers[BUF_MORPH_DELTAS])))
		return E_FAIL;

	// 5. SRV 생성
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.NumElements = static_cast<_uint>(vDeltas.size());

	if (FAILED(m_pDevice->CreateShaderResourceView(m_Buffers[BUF_MORPH_DELTAS], &SRVDesc, &m_SRVs[BUF_MORPH_DELTAS])))
		return E_FAIL;

	return S_OK;
}




CMesh* CMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile)
{
	CMesh* pInstance = new CMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, Bones, PreTransformMatrix, InputFile)))
	{
		MSG_BOX("Failed to Create : Mesh");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CMesh::Clone(void* pArg)
{
	CMesh* pClone = new CMesh(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Create : Mesh (Clone)");
		Safe_Release(pClone);
	}

	return pClone;
}

void CMesh::Free()
{
	__super::Free();


	// 1. 힙에 백업해둔 원본 정점 데이터 삭제
	if (!m_isClone)
		Safe_Delete_Array(m_pRestPoseVertices);

	for (auto& pShapeKey : m_ShapeKeys)
		Safe_Release(pShapeKey);
	m_ShapeKeys.clear();

	/* GPU Buffer 내용 제거 */
	for (auto& pBuffer : m_Buffers)
		Safe_Release(pBuffer);
	m_Buffers.clear();

	for (auto& pSRV : m_SRVs)
		Safe_Release(pSRV);
	m_SRVs.clear();

	for (auto& pUAV : m_UAVs)
		Safe_Release(pUAV);
	m_UAVs.clear();

}
