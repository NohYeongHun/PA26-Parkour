#pragma once
#include "Interface_Edit.h"

NS_BEGIN(Editor)

class CShader_Interface final : public CInterface_Edit
{
private:
	explicit CShader_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShader_Interface() = default;

public:
	virtual	HRESULT		Initialize();
	void				Update_Shadow();

	//All
	void				Setting_Shader();

	//Part
	void				Setting_LUT();
	void				Set_ShadowBias();
	void				Set_SSAO();
	
private:
	_float				m_fBias[4] = {};
	_float				m_fMinBias[4] = {};
	_float				m_fSlopeScale = {};
	_int				m_iLUT_Index = {};
	_float				m_fLUT_Intensity = {};
	_float				m_fSigmaWeight = {0.01f};
	_float				m_fRadius = { 1.f };
	_float				m_fMaxDistance = {5.f};

	_float				m_fMinDepthWeight = {0.1f};
	_float				m_fMinNormalWeight = { 0.1f };

	_float				m_fBloomIntensity = { 0.25f };
	_int				m_iBloomWeight = { 1 };

	_float2				m_vFogDepthDistance = _float2(1000.f, 5000.f);
	_float2				m_vFogHeightDistance = _float2(0.f, 100.f);
	_float4				m_vFogColor = _float4(1.f, 1.f, 1.f, 1.f);

	_float				m_fFocusDepth = {50.f};
	_float				m_fFocusRange = { 100.f };
	_float				m_fDofDepthScale = { 0.3f };
	_float				m_fEffectIntensity = { 10.f };

	_float				m_fRoughness = { 0.2f };
	_float				m_fMetallic = { 0.f };

	_float				m_fLimitVelocity = {30.f};
	_float				m_fLimitDepth = { 300.f};
	_float				m_fBlurDistanceScale = {2.f};

	_float				m_vCenterUV[2] = {0.5f, 0.5f};
	_float				m_vRange[2] = {0.f, 0.2f};
	_float				m_fRadialScale = {-0.3f};

private:
	void				Setting_Bias(const _char* pName, _float* pFloat);

public:
	static CShader_Interface* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END