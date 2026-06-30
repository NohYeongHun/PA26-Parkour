#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class CMesh final : public CVIBuffer
{
	enum CORNER { LTN, RTN, RBN, LBN, LTF, RTF, RBF, LBF, END };

	enum BUFFER_TYPE // Buffer를 생성해서 SRV, UAV 생성.
	{
		BUF_BASE_VERTICES = 0,      // t0: 원본 정점 (SRV)
		BUF_MORPH_DELTAS,           // t1: 델타 데이터 (SRV)
		BUF_MORPH_OUTPUT,           // u0: 결과 저장용 (UAV/SRV)
		BUF_MORPH_INFOCB,
		BUF_END
	};

private:
	explicit CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMesh(const CMesh& Prototype);
	virtual ~CMesh() = default;

public:
	_uint					Get_NumBones() { return m_iNumBones; }
	void					Copy_BoneMatrices(_float4x4* pOutMatrices, _uint iNumBones);
	_uint					Get_MaterialIndex() { return m_iMaterialIndex; }
	const vector<_float3>& Get_VerticesPos() { return m_VertexPositions; }
	const vector<_uint>& Get_Indices() { return m_Indices; }

	const vector<_uint>& Get_BoneIndices() { return m_BoneIndices; }
	const vector<_float4x4>& Get_OffsetMatrices() const { return m_OffsetMatrices; }

	const vector<class CShapeKey*>& Get_ShapeKeys() { return m_ShapeKeys; }
	_uint	Get_NumVertices() const { return m_iNumVertices; }
	_bool	HasMorphTargets() const { return !m_ShapeKeys.empty() && m_iNumAnimMeshes > 0; }

public:
	virtual		HRESULT			Initialize_Prototype(MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile);
	virtual		HRESULT			Initialize_Clone(void* pArg);

public:
	void Update_Morph_CPU(const vector<_float>& vShapeKeyWeights);
	void Compute_Morph(class CComputeShader* pMorphComputeShaderCom, ID3D11ShaderResourceView* pWeightSRV);
#ifdef _DEBUG
	_bool							Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
#endif



public:
	HRESULT						Bind_BoneMatrices(class CShader* pShaderCom, const _char* pConstantName, const vector<class CBone*>& Bones);
	HRESULT						Bind_MorphedResult(class CShader* pShaderCom, const _char* pConstantName);

public:
	HRESULT						Ready_SharedBuffers_ForMorph();   // Prototype 에서만 생성 => Add Ref
	HRESULT						Ready_InstanceBuffers_ForMorph(); // Clone 복제체에서만 생성

private:
	_uint							m_iMaterialIndex = {};
	_uint							m_iNumBones = {};

	vector<_uint>				m_BoneIndices;
	_float4x4					m_BoneMatrices[g_iMaxNumBones] = {};

	vector<_float4x4>			m_OffsetMatrices;


	vector<_float3>				m_VertexPositions;
	vector<_uint>				m_Indices;

#pragma region SHAPE KEY 정보
private:
	MODELTYPE m_eModelType = { };
	_bool m_IsSharedReady = { false };
	_bool m_IsInstanceReady = { false };

	_uint m_iNumAnimMeshes = {};
	vector<class CShapeKey*> m_ShapeKeys;
	//map<_string, class CShapeKey*> m_ShapeKeys;

	VTXANIMMESH* m_pRestPoseVertices = {}; // T Pose 정점 데이터
	vector<ID3D11Buffer*> m_Buffers;
	vector<ID3D11ShaderResourceView*> m_SRVs;
	vector<ID3D11UnorderedAccessView*> m_UAVs;
#pragma endregion


private:
	HRESULT						Ready_Mesh_NonAnim(_fmatrix PreTransformMatrix, ifstream& InputFile);
	HRESULT						Ready_Mesh_Anim(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile);
	HRESULT						Ready_Mesh_Character(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile);
	HRESULT						Ready_Mesh_Map(_fmatrix PreTransformMatrix, ifstream& InputFile, _float* MinPos, _float* MaxPos);



private:
	HRESULT						Create_BaseVertexBuffer();
	HRESULT						Create_DeltaBuffer();





public:
	static		CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODELTYPE eType, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile);
	virtual		CComponent* Clone(void* pArg);
	virtual		void			Free() override;
};

NS_END