#pragma once
#include "Component.h"
#include"Model_Manager.h"

NS_BEGIN(Engine)
class CMesh_Streaming final : public CComponent
{
private:
	explicit CMesh_Streaming(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CMesh_Streaming(const CMesh_Streaming& Prototype);
	virtual ~CMesh_Streaming() = default;

public:
	virtual		HRESULT				Initialize_Prototype(_uint iNumMeshes);
	virtual		HRESULT				Initialize_Clone(void* pArg);

	atomic<LOADSTATE>&  IsLoaded() { return m_LoadState; }

	void Load_LastLODIndex(_string LastModelPath);
	void Set_Buffers(ID3D11Buffer* pSharedVB, ID3D11Buffer* pSharedIB);
	HRESULT Bind_Resources(_uint iMeshIndex);
	HRESULT Bind_Resources(_uint iMeshIndex, ID3D11DeviceContext* pDC);
	HRESULT Render(_uint iMeshIndex);
	HRESULT Render(_uint iMeshIndex, ID3D11DeviceContext* pDC);

public:
	vector<CModel_Manager::SHARED_DATA_DESC>* Get_MeshDesc() { return m_Desc; }
	_uint									  Get_MeshNum() { return m_iNumMeshes; }
	_bool									  Is_Overed(_uint iMeshIndex) { return iMeshIndex >= m_iNumMeshes; }
	void							Set_RigidData(vector<_float3>& vecVertexPos, vector<_uint>& vecIndices, _uint iMeshIndex);
	vector<_float3>					Get_VerticesPos(_uint iMeshIndex){ return m_vecVertexPos[iMeshIndex]; }
	vector<_uint>					Get_Indices(_uint iMeshIndex) { return m_vecIndices[iMeshIndex]; }
	void							Destroy_RigidData();

#ifdef _DEBUG
	void							Ready_BoundingBox();
	BoundingBox* Get_BoundingBox() { return m_pBoundingBox; }
	_bool Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
#endif
private:
	vector<_float3>* m_vecVertexPos;
	vector<_uint>* m_vecIndices;

	vector<CModel_Manager::SHARED_DATA_DESC>* m_Desc;
	atomic<LOADSTATE> m_LoadState = { LOADSTATE::NOTLOADED };
	_uint m_iNumMeshes = {};
	ID3D11Buffer* m_pSharedVB = { nullptr };
	ID3D11Buffer* m_pSharedIB = { nullptr };

	_uint m_iVertexStride = sizeof(VTXMESH);
	DXGI_FORMAT m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	D3D11_PRIMITIVE_TOPOLOGY m_ePrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#ifdef _DEBUG
	BoundingBox* m_pBoundingBox = { nullptr };
#endif

public:
	static CMesh_Streaming* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumMeshes);
	virtual void Free()override;

	// CComponent을(를) 통해 상속됨
	CComponent* Clone(void* pArg) override;
};

NS_END