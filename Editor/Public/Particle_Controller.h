#pragma once
#include "Base.h"
#include "Particle.h"

NS_BEGIN(Editor)

class CParticle_Controller final : public CBase
{
public:
	typedef struct ParticleTextureTag {
		_char szName[MAX_PATH] = {};
		_tchar strTextureTag[MAX_PATH] = {};
		class CTexture* pTexture;

	}PARTICLE_TEXTURE;

private:
	explicit CParticle_Controller(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CParticle_Controller() = default;

#pragma region
public:
	HRESULT Initialize();
	void Update();
	void Render();

#pragma endregion

private:
	//void Texture_Loading(const char* TextureName, const _tchar* pFilePath);
	void Load_AllTextureFromFolder(const _string& strFolderPath);

public:
	void Particle_Tab();

	void Particle_Base_Tab(CParticle::PARTICLE_DESC& tParticleDesc, _bool& IsCreate);

public:
	void UpdateSelected_ParticleFormTag(_wstring ParticleTag);
	
	CParticle::PARTICLE_DESC* Get_ParticleDesc(_wstring& ParticleTag);
	CVIBuffer_Point_Instance::POINT_INSTANCE_DESC* Get_VBDesc(_wstring& VBTag);
	void	Set_ParticleDesc(_wstring& ParticleTag, CParticle::PARTICLE_DESC& ParticleDesc);
	void	Set_VBDesc(_wstring& ParticleTag, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC& ParticlVBeDesc);

	void Set_ParticleTag(const _char* szParticleTag);

	void Remove_Desc(const _wstring& DescTag);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	vector<PARTICLE_TEXTURE>									m_Textures = {};
	_int														m_iSelectedTexture = -1;
	_bool														m_TexturPopOpend = false;

	_char														m_ParticleTag[MAX_PATH] = {};
	_bool														m_bTagFlag = false;

	map<const _wstring, CParticle::PARTICLE_DESC>						m_tParticleDesc = {};
	map<const _wstring, CVIBuffer_Point_Instance::POINT_INSTANCE_DESC>	m_tVBDesc = {};

	_int														m_iSelectedParticle = 0;
	_bool														m_bSelectedParticle = false;
	CParticle::PARTICLE_DESC*									m_pSelectedParticleDesc = { nullptr };
	CVIBuffer_Point_Instance::POINT_INSTANCE_DESC*				m_pSelectedVBDesc = { nullptr };

	_float														m_fColor[4] = {0.f, 0.f, 0.f, 1.f};		

public:
	static CParticle_Controller* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};

NS_END