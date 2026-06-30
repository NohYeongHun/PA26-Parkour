#pragma once
#include "Component.h"
NS_BEGIN(Engine)
class ENGINE_DLL CModel_Instance_FireFly final: public CComponent
{
private:
	CModel_Instance_FireFly(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel_Instance_FireFly(const CModel_Instance_FireFly& Prototype);
	virtual ~CModel_Instance_FireFly() = default;


public:
	HRESULT Initialize_Prototype(_fmatrix PreTransformMatrix, const _char* pFilePath, _bool IsEdit, void* pArg);
	HRESULT Initialize_Clone(void* pArg);
	HRESULT							Render(_uint iMeshIndex);
	HRESULT							Render(_uint iMeshIndex, ID3D11DeviceContext* pDC);
public:
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex);
	HRESULT							Bind_Materials(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType);

	HRESULT							Bind_Materials(class CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, _uint iTextureIndex, ID3DX11Effect* pEffect);
	HRESULT							Bind_Materials(class CDeferredShader* pShader, const _char* pConstantName, _uint iMeshIndex, TEXTURETYPE eTextureType, ID3DX11Effect* pEffect);
#ifdef _DEBUG
	void							Change_Pos(_fvector vPos);
#endif
private:
	_uint									m_iNumMeshes = {};
	vector<class CMesh_Instance_FireFly*>			m_Meshes;

	_uint									m_iNumMaterials = {};
	vector<class CMaterial*>			m_Materials;

	_float4x4								m_PreTransformMatrix = {};

	_float m_MinPos[3] = { FLT_MAX,FLT_MAX ,FLT_MAX };
	_float m_MaxPos[3] = { FLT_MIN ,FLT_MIN ,FLT_MIN };

private:
	HRESULT							Ready_Mesh(ifstream& InputFile, _bool IsEdit, void* pArg);
	HRESULT							Ready_Material(const _char* pFilePath);

public:
	static CModel_Instance_FireFly* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fmatrix PreTransformMatrix, const _char* pFilePath, _bool IsEdit = false, void* pArg = nullptr);
	virtual CComponent* Clone(void* pArg)override;
	virtual void Free()override;
};

NS_END