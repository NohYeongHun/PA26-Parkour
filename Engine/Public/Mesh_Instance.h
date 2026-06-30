#pragma once
#include "VIBuffer_Instance.h"
NS_BEGIN(Engine)

class CMesh_Instance final : public CVIBuffer_Instance
{
public:
	typedef struct tagInstanceMesh {
		_uint iNumInstance;
		_float4x4* pTransformMatrix = { nullptr };
	}MESH_INST_DESC;
public:
	_uint							Get_MaterialIndex() { return m_iMaterialIndex; }

private:
	CMesh_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh_Instance(const CMesh_Instance& Prototype);
	CMesh_Instance() = default;

public:
	
		//virtual HRESULT		Initialize_Prototype(_fmatrix PreTransformMatrix, ifstream& InputFile);
	virtual HRESULT		Initialize_Prototype(_fmatrix PreTransformMatrix, _bool IsEdit, void* pArg, ifstream& InputFile, _float* MinPos, _float* MaxPos);
	virtual HRESULT		Initialize_Clone(void* pArg);
	//virtual HRESULT		Render();

	//virtual HRESULT		Bind_Resources();
#ifdef _DEBUG
	_bool								Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
	void								Change_InstanceInfo(_uint iNumInstance, _fmatrix fMatrix);
#endif

private:
	_float4x4*						m_TransformMatrices = { nullptr };

private:
	_uint							m_iMaterialIndex = {};
	_bool							m_IsEdit = { false };
	//vector<_float4x4>				m_OffsetMatrices;

#ifdef _DEBUG
	vector<_float3>			m_VertexPositions;
	vector<_uint>				m_Indices;
#endif

public:
	
		//static CMesh_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PreTransformMatrix, ifstream& InputFile);
	static CMesh_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PreTransformMatrix, _bool IsEdit, void* pArg, ifstream& InputFile, _float* MinPos, _float* MaxPos);
	virtual CMesh_Instance* Clone(void* pArg)override;
	virtual void Free()override;
};

NS_END