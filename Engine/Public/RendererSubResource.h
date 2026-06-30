#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CTexture;
class CShader;
class CGameInstance;

class CRendererSubResource final : public CBase
{
private:
	typedef vector<pair<_int, vector<_float>>> BLUR_WEIGHTS;

private:
	typedef struct tagSSaoBlurData {
		_float  fSSAO_MinDepthDistance;
		_float	fWidth;
		_float	fHeight;
		_float  Paddingblur;
	}SSAO_BLUR_DATA;
	
	typedef struct tagBlurData {
		_float2  vSize;
		_int	 iRadius;
		_float   Padding;
	}BLUR_DATA;

	typedef struct tagBloomUpData {
		_float2  vSize;
		_float	fIntensity;
		_float   Padding;
	}BLOOM_UP_DATA;

	typedef struct tagMotionBlurData {
		_float fLimitVelocity;
		_float fLimitDepth;
		_float fLengthScale;
		_float PaddingMotion;
	}MOTION_BLUR_DATA;

private:
	explicit CRendererSubResource(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRendererSubResource() = default;

public:
	HRESULT						Initialize();
	HRESULT						Bind_Ramp_Texture(CShader* pShader, const _char* pConstantName, _uint iTextureIndex);
	HRESULT						Bind_LUT_Texture(CShader* pShader, _uint iLUT_Index);
	HRESULT						Set_DefalutSampler(const _wstring& strRCSTag, _uint iSlot);

private:
	CGameInstance*				m_pGameInstance = { nullptr };
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	
#pragma region RAMP
	CTexture*					m_pRampTexture = { nullptr };
#pragma endregion

#pragma region LUT
	CTexture*					m_pLUT_Texture = { nullptr };
	ID3D11ShaderResourceView*	m_pLUT_SRV = { nullptr };
	_uint						m_iNumLUT_Textures = {};
#pragma endregion

#pragma region Sampler
	ID3D11SamplerState*			m_pDefaultSampler = { nullptr };
	ID3D11SamplerState*			m_pPointClampSampler = { nullptr };
	ID3D11SamplerState*			m_pNoiseSampler = { nullptr };
#pragma endregion

private:
	HRESULT						Ready_Shader_Filters();
	HRESULT						Ready_LUT_SRV();
	HRESULT						Ready_CS_Sampler();

public:
	static CRendererSubResource*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void						Free() override;
};

NS_END