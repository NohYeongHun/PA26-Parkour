#pragma once
#include "Component.h"
#include"Mesh_Streaming.h"

NS_BEGIN(Engine)
class ENGINE_DLL CModel_Streaming final : public CComponent
{
private:
	explicit CModel_Streaming(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CModel_Streaming(const CModel_Streaming& Prototype);
	virtual ~CModel_Streaming() = default;
public:

	_uint								Get_NumMesh(_uint iMeshIndex) { return m_pModelPrototype->m_iNumMeshes[iMeshIndex]; }
public:
	virtual		HRESULT				Initialize_Prototype(const _char* pFilePath);
	virtual		HRESULT				Initialize_Clone(void* pArg);
	HRESULT							Render(_uint iLODIndex, _uint iMeshIndex);
	HRESULT							Render(_uint iLODIndex, _uint iMeshIndex, ID3D11DeviceContext* pDC);

public:
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex);
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType);
	HRESULT							Bind_Materials(class CDeferredShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect);
	HRESULT							Bind_Materials(class CDeferredShader* pShader, const _char* pConstantName, _uint iLODIndex, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect);

	void							Ready_BoundingBox(_float* pMinPos, _float* pMaxPos);
	HRESULT							Bind_Buffer(ID3D11DeviceContext* pDeferredContext, _uint iLODIndex);
	atomic<LOADSTATE>& Get_MeshState(_uint iLODIndex) { return m_pModelPrototype->m_LodState[iLODIndex]; }
public:
	vector<CModel_Manager::SHARED_DATA_DESC>* Get_MeshDesc(_uint iLODIndex);
	void									  RequestModel(_uint iLODIndex = 99);
	void									  Set_RenderTime(_uint iLODIndex, _float fTimeDelta) { m_pModelPrototype->m_fRenderTime[iLODIndex] = fTimeDelta; }
	_bool									  Is_RenderTimeOver(_uint iLODIndex);
public:
	HRESULT							Ready_Mesh(const _char* pFilePath);
	HRESULT							Ready_Material();
	void							PlusRenderdTime(_float fTimeDelta);
	HRESULT							Get_SharedBuffers(_uint iLODIndex, ID3D11Buffer* pVertex, ID3D11Buffer* pIndex);
	_bool							Is_Overed(_uint iLODIndex, _uint iMeshIndex);
	const _string&					Find_ModelPrototype() { return m_pModelPrototype->m_ModelPath; }
	_uint							Get_LastLODIndex();
	void							Request_LOD(_uint iLODIndex);
	_uint							Get_ReadyLOD();

	void							Set_RigidData(vector<_float3>& vecVertexPos, vector<_uint>& vecIndices,_uint iMeshIndex);
	vector<_float3>					Get_VerticesPos(_uint iIndex) { return m_pModelPrototype->m_Meshes[0]->Get_VerticesPos(iIndex); }
	vector<_uint>					Get_Indices(_uint iIndex) { return m_pModelPrototype->m_Meshes[0]->Get_Indices(iIndex); }
	void							Destroy_RigidData() { m_pModelPrototype->m_Meshes[0]->Destroy_RigidData(); }
#ifdef _DEBUG
	void							Ready_BoundingBox() { m_pModelPrototype->m_Meshes[0]->Ready_BoundingBox(); }
	BoundingBox* Get_BoundingBox() { return m_pModelPrototype->m_Meshes[0]->Get_BoundingBox(); }
	_bool								Is_Picked(const _fvector& vRayPos, const _fvector& vRayDir, _float* pDistance);
#endif
private:

	_uint									m_iNumMeshes[4] = { 0,0,0,0 };
	class CMesh_Streaming*					m_Meshes[4] = { nullptr,nullptr,nullptr,nullptr };

	_uint									m_iNumMaterials = {};
	//vector<class CMaterial*> m_Materials;
	vector<class CMaterial*> m_Materials[4];
	atomic<LOADSTATE>						m_LodState[4] = { LOADSTATE::NOTLOADED,LOADSTATE::NOTLOADED ,LOADSTATE::NOTLOADED ,LOADSTATE::NOTLOADED };

	_float									m_fRenderTime[4] = { 0.f,0.f,0.f,0.f };
	_string m_ModelPath;
	_uint m_iMaxLOD = { 0 };
	CModel_Streaming* m_pModelPrototype = { nullptr };

public:
	static		CModel_Streaming* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* pFilePath);
	virtual		CComponent* Clone(void* pArg);
	virtual		void					Free() override;
};

NS_END