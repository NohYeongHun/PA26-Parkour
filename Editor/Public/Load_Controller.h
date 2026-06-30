#pragma once
#include "Base.h"
#include "Particle.h"
#include "Effect_Prefab.h"
#include "Effect_Mesh.h"
#include "Trail_Mesh.h"
#include "Effect_Rect.h"
#include "Effect_Decal.h"
#include "Effect_Radial.h"
#include "VIBuffer_FXMesh_Instance.h"
#include "VIBuffer_Point_Instance.h"
#include "TestVA.h"
#include "Effect_Light.h"
NS_BEGIN(Editor)

//다른 곳에서 이펙트 불러오는 연동성을 위해 추가한 것.?
class CLoad_Controller final : public CBase
{
private:
	explicit CLoad_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoad_Controller() = default;

#pragma region 기본
public:
	HRESULT Initialize(LEVEL eCurrentLevel);
	void Update();
	void Render();

#pragma endregion

private:
	//다른곳에서 이용할 때면 텍스처, 매쉬 등 다양한 얘들 원형이 필요할거 같음. 고민되는건, 미리 다 올려놓지 말고
	//각각의 이펙트가 필요한 파일들의 경로를 들고있게 할까? 흠.. 고민 필요.
	void Load_TrailMesh_AllTextureFromFolder(const _string& strFolderPath);
	void Load_TrailMesh_AllMeshDatFromFolder(const _string& strFolderPath);
	void Load_TrailMesh_AllColorTextureFormFolder(const _string& strFolderPath);

public:
	void Prefab_Load_Tab(_bool* IsLoad = nullptr);

public:
	void Load_Prefab_FromJson(const _string& strFilePath, const _string& strFolderPath);

	void Load_Particle_VB_FromJson(const _string& strFilePath, const _wstring& ParticleTag);
	void Load_Particle_OB_FromJson(const _string& strFilePath, const _wstring& ParticleTag);

	void Load_FXMesh_VB_FromJson(const _string& strFilePath, const _wstring& MeshTag);
	void Load_FXMesh_OB_FromJson(const _string& strFilePath, const _wstring& MeshTag);

	void Load_TrailMesh_FromJson(const _string& strFilePath, const _wstring& TrailMeshTag);

	void Load_FXRect_FromJson(const _string& strFilePath, const _wstring& RectTag);

	void Load_FXDecal_FromJson(const _string& strFilePath, const _wstring& DecalTag);

	void Load_FXRadial_FromJson(const _string& strFilePath, const _wstring& RadialTag);
	
	void Load_FXVA_FromJson(const _string& strFilePath, const _wstring& VATag);

	void Load_FXLight_FromJson(const _string& strFilePath, const _wstring& LightTag);

public:
	void Get_Prefab_Desc(CEffect_Prefab::PREFAB_DESC& PrefabDesc);

	void Get_Particle_VB_Desc(const _wstring& ParticleTag, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC& ParticleVBDesc);
	void Get_Particle_OB_Desc(const _wstring& ParticleTag, CParticle::PARTICLE_DESC& ParticleDesc);

	void Get_FXMesh_VB_Desc(const _wstring& FXMeshTag, CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC& MeshVBDesc);
	void Get_FXMesh_OB_Desc(const _wstring& FXMeshTag, CEffect_Mesh::EFFECTMESH_DESC& MeshDesc);

	void Get_TrailMesh_Desc(const _wstring& TrailMeshTag, CTrail_Mesh::TRAILMESH_DESC& TrailMesh);

	void Get_FXRect_Desc(const _wstring& RectTag, CEffect_Rect::FXRECT_DESC& RectDesc);

	void Get_FXDecal_Desc(const _wstring& DecalTag, CEffect_Decal::DECAL_DESC& DecalDesc);

	void Get_FXRadial_Desc(const _wstring& RadialTag, CEffect_Radial::RADIAL_DESC& RadialDesc);

	void Get_FXVA_Desc(const _wstring& VATag, CTestVA::VA_DESC& VADesc);

	void Get_FXLight_Desc(const _wstring& LightTag, CEffect_Light::LIGHT_DESC& LightDesc);
public:
	void Reset_Load();

	void Add_CurrentLevel_Effect();

private:
	ID3D11Device*												m_pDevice = { nullptr };
	ID3D11DeviceContext*										m_pContext = { nullptr };
	class CGameInstance*										m_pGameInstance = { nullptr };
	LEVEL														m_eCurrentLevel = {};

	//Effect레벨에서 로드시 Desc 정보들 컨트롤러에 넘겨줘야해서 저장용
	//프리팹 저장
	CEffect_Prefab::PREFAB_DESC								   m_tPrefabDesc = {};

	//파티클 저장
	map<const _wstring, CParticle::PARTICLE_DESC>						m_tParticleDesc = {};
	map<const _wstring, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC>	m_tParticleVBDesc = {};

	//FXMesh 저장
	map<const _wstring, CVIBuffer_FXMesh_Instance::MESH_FXINSTANCE_DESC>	m_tMeshVBDesc = {};
	map<const _wstring, CEffect_Mesh::EFFECTMESH_DESC>						m_tEffectMeshDesc = {};

	//Trail 저장
	map<const _wstring, CTrail_Mesh::TRAILMESH_DESC>						m_tTrailMeshDesc = {};

	//Rect 저장
	map<const _wstring, CEffect_Rect::FXRECT_DESC>							m_tRectDesc = {};

	//Decal 저장
	map<const _wstring, CEffect_Decal::DECAL_DESC>							m_tDecalDesc = {};

	//Radial 저장
	map<const _wstring, CEffect_Radial::RADIAL_DESC>						m_tRadialDesc = {};

	//VA 저장
	map<const _wstring, CTestVA::VA_DESC>									m_tVADesc = {};

	//Light 저장
	map<const _wstring, CEffect_Light::LIGHT_DESC>							m_tLightDesc = {};

public:
	static CLoad_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eCurrentLevel);
	virtual	void Free() override;
};

NS_END