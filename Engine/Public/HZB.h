#pragma once
#include "Base.h"

#define MAX_OBJECT 3000

NS_BEGIN(Engine)

class CHZB final : public CBase
{
private:
	typedef struct tagBoxInfo {
		_float4	vCorner[8] = {};
		_float3	vCenter = {};
		_float		fRadius = {};
	}BOXINFO;

	typedef struct tagOcclusionDesc {
		_float4x4		ProjMatrix = {};
		_uint			iNumObjects = {};
		_uint			iHZBMipLevel = {};
		_float			fMip0SizeX = {};
		_float			fMip0SizeY = {};
	}OC_DESC;

	typedef struct tagVisibleCount {
		_bool			isVisible = { false };
		_int			iCount = {};
		tagVisibleCount()
			: isVisible{ false }, iCount{ 0 }
		{}
	}VISIBLE_COUNT;

	typedef struct tagCameraFar {
		_float			fFar = {};
		_float3		vPadding = {};
	}CAMERA_FAR;

public:
	enum HZB_CS_TYPE
	{
		MIPMAP,
		OCCLUSION,
		END
	};

private:
	explicit CHZB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CHZB() = default;

public:
	ID3D11ShaderResourceView* Get_Resource() { return m_pMMSRV; }

public:
	HRESULT					Initialize(_uint iWinSizeX, _uint iWinSizeY);
	void						Update();

	void						Occlusion_Culling(vector<class CStaticObject*>& Objects);

#ifdef _DEBUG
public:
	void						Render();
private:
	void AddRemoveHZB(const _string& strHZB, ImTextureID TextureID);
	map<const _string, ImTextureID> m_RenderTextures;
#endif
private:
	ID3D11Device*										m_pDevice = { nullptr };
	ID3D11DeviceContext*							m_pContext = { nullptr };
	class CGameInstance*							m_pGameInstance = { nullptr };
	class CComputeShader*							m_pComputeShader[HZB_CS_TYPE::END] = {nullptr};

	_uint													m_iWinSizeX = {};
	_uint													m_iWinSizeY = {};

	// Default Setting
	ID3D11UnorderedAccessView*					m_pUAV[MAX_MIPLEVEL] = { nullptr };
	ID3D11ShaderResourceView*					m_pSRV[MAX_MIPLEVEL] = { nullptr };
	ID3D11ShaderResourceView*					m_pMMSRV = { nullptr };
	ID3D11Buffer*										m_pCameraFarBuffer = { nullptr };

	// Occlusion Culling
	ID3D11Buffer*										m_pOCDescBuffer = { nullptr };
	ID3D11Buffer*										m_pBoxPointsBuffer = { nullptr };
	ID3D11ShaderResourceView*					m_pBoxPointsSRV = { nullptr };
	ID3D11Buffer*										m_pOcclusionFlagBuffer = { nullptr };
	ID3D11UnorderedAccessView*					m_pOcclusionResultUAV = { nullptr };
	ID3D11Buffer*										m_pOcclusionStageBuffer[2] = {nullptr};
	_uint													m_iWriteIndex = { 0 };
	_uint													m_iReadIndex = { 1 };

	// Temporal Filter (Pre Visible Store)
	map<size_t, VISIBLE_COUNT>					m_PreVisible;
	BOXINFO*									m_pBoxInfos = { nullptr };
private:
	void						Ready_DefaultSetting();
	void						Ready_OcclusionCulling();

public:
	static		CHZB*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	virtual		void		Free() override;
};

NS_END