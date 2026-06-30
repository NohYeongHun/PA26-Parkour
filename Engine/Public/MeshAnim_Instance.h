#pragma once
#include "VIBuffer_Instance.h"

NS_BEGIN(Engine)

class CMeshAnim_Instance final : public CVIBuffer_Instance
{
	enum CORNER { LTN, RTN, RBN, LBN, LTF, RTF, RBF, LBF, END};
	typedef struct tagInstanceMesh {
		_uint iNumInstance;
		_float4x4* pTransformMatrix = { nullptr };
	}MESH_INST_DESC;
private:
	explicit CMeshAnim_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMeshAnim_Instance(const CMeshAnim_Instance& Prototype);
	virtual ~CMeshAnim_Instance() = default;

public:
	_uint							Get_MaterialIndex() { return m_iMaterialIndex; }
	const vector<_float3>&	Get_VerticesPos() { return m_VertexPositions; }
	const vector<_uint>&		Get_Indices() { return m_Indices; }

	const vector<_uint>& Get_BoneIndices() { return m_BoneIndices; }
	const vector<_float4x4>& Get_OffsetMatrices() const { return m_OffsetMatrices; }

public:
	virtual		HRESULT			Initialize_Prototype(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _uint iNumInstance, _uint iMatPadding);
	virtual		HRESULT			Initialize_Clone(void* pArg);
	virtual		HRESULT			Render() override;
	virtual		HRESULT			Render(ID3D11DeviceContext* pDC)override;

#ifdef _DEBUG
	_bool							Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
#endif

public:
	HRESULT						Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, const vector<class CBone*>& Bones);
	HRESULT						Bind_OffsetMatrix(class CShader* pShader, const _char* pConstantName);
	HRESULT						Update_InstanceData(VTXINSTANCE_ANIMMESH* pMatrices, _uint iNumRenderCount);
private:
	ID3D11Buffer*					m_pBoneLocalIdxBuf = { nullptr };
	ID3D11ShaderResourceView*		m_pBoneLocalIdxSRV = { nullptr };
	_uint							m_iMaterialIndex = {};
	_uint							m_iNumBones = {};
	_uint							m_iNumMaxInstance = {};

	vector<_uint>				m_BoneIndices;
	_float4x4					m_BoneMatrices[g_iMaxNumBones] = {};

	vector<_float4x4>			m_OffsetMatrices;

	// Mesh Shape??Container
	vector<_float3>				m_VertexPositions;
	vector<_uint>				m_Indices;

private:
	HRESULT						Ready_Mesh_Anim(const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _uint iMatPadding);

public:
	static		CMeshAnim_Instance*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix, ifstream& InputFile, _uint iNumInstance, _uint iMatPadding);
	virtual		CComponent*				Clone(void* pArg);
	virtual		void					Free() override;
};

NS_END