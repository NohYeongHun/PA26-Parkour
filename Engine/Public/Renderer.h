#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CRendererSubResource;
class CTexture;

class CRenderer final : public CBase
{
private:
	explicit CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;

public:
	HRESULT				Initialize(_uint iNumThread);
	HRESULT				Add_Render_Object(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	//HRESULT				Add_Render_StaticObject(class CStaticObject* pRenderObject);
	//HRESULT				Add_Render_StaticObject(const vector<class CStaticObject*>& Container);
	HRESULT				Add_Render_StaticObject(class CStaticObject* pRenderObject);
	HRESULT				Add_Render_StaticObject(class CStaticObject* pRenderObject, _uint iNumLODIndex);
	HRESULT				Add_Render_StaticObject(vector<class CStaticObject*>* Container);
	HRESULT				Add_Render_ShadowMapObject(class CGameObject* pRenderObject);
	void				Render();
	void				Add_Effects(const _wstring& strEffectTag, const vector<ID3DX11Effect*> Effects);
	ID3DX11Effect*		Get_Shader_Effect(const _wstring& strEffectTag, _uint iIndex);
	ID3D11ShaderResourceView* Get_CurrentSceneSRV() { return m_pCurrentSceneSRV; }
	void				SettingFog(_bool IsOn) { m_IsFog = IsOn; }
	void				SettingSSS(_bool IsOn) { m_IsSSS = IsOn; }
	void				SettingHDR(_float fExposure) { m_fExposure = fExposure; }
	void				Setting_LUT(_uint iIndex, _float fIntensity, _bool IsDynamicLUT) { m_iLUT_Index = iIndex, m_fLutLerpIntensity = fIntensity, m_IsDynamicLUT = IsDynamicLUT; }
	void				Get_Current_LutSetting(_uint* pOutIndex, _float* pOutIntensity, _bool* pOutIsDynamicLut);
	void				Render_ShadowMap();
	void				Render_LOD(_uint iLODIndex);
	void				Render_LOD_Weight();
	void				Clear_Resource();

#ifdef _DEBUG
	HRESULT			Add_Render_Debug(class CComponent* pDebugComponent);
	HRESULT			Bind_RawValue(const _char* pConstantName, void* pValue, _uint iLength);
	void			IsSSAO(_bool IsSSao) { m_IsSSAO = IsSSao; }
	void			IsSSAO_Blur(_bool IsBlur) { m_IsSSAO_Blur = IsBlur; }
#endif

private:
	ID3D11Device*								m_pDevice = { nullptr };
	ID3D11DeviceContext*						m_pContext = { nullptr };
	ID3D11DeviceContext**						m_pDeferredContext = { nullptr };
	class CGameInstance*						m_pGameInstance = { nullptr };

	ID3D11ShaderResourceView*					m_pCurrentSceneSRV = { nullptr };

	// 비동기 렌더링
	_uint										m_iNumThread = {};
	atomic<_int>								m_iNumEndThread = {};
	vector<ID3D11CommandList*>					m_CommandLists;
	map<const _wstring, vector<ID3DX11Effect*>> m_Effects;
	mutex										m_RenderMutex;
	mutex										m_RenderAddMutex;
	condition_variable							m_CV;

	// Culling
	list<class CGameObject*>					m_RenderObjects[ENUM_CLASS(RENDERGROUP::END)];
	//vector<class CStaticObject*>				m_StaticObjects[2];
	vector<class CStaticObject*>				m_StaticObjects[2][4];
	atomic<_uint>								m_iDoubleBufferIndex = {};
	atomic<_uint>								m_iCullStack = {};
	atomic<_bool>								m_isCompleteFrustumCull = { false };
	_uint										m_iNumPreRenderObject = {};
	list<class CGameObject*>					m_ShadowMapObjects;

	class CShader*							m_pShader = { nullptr };
	class CVIBuffer_Rect*					m_pVIBuffer = { nullptr };

	_float4x4								m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	_uint									m_iWinSizeX{}, m_iWinSizeY{};
	_float									m_fWinSizeX{}, m_fWinSizeY{};
	
	CRendererSubResource*					m_pSubResource = { nullptr };
	_uint									m_iLUT_Index = {};
	_float									m_fLutLerpIntensity = {};
	_bool									m_IsDynamicLUT = {};


	recursive_mutex							m_RecursiveMutex;

	_uint									m_iCurTime = {};
	_uint									m_iInterval = {};
	_bool									m_IsFog = { true };
	_bool									m_IsSSS = { false };		// 카툰이라 큰 차이가 없음,,

	_bool									m_IsSSAO = { true };
	_bool									m_IsOutLine = { true };
	_bool									m_IsLight = { true };

	_float4									m_vBackBufferColor = {};

	_bool									m_isRenderDebug = { false };

	_float									m_fExposure = {};

#ifdef _DEBUG
	list<class CComponent*>					m_DebugComponents;


	_bool									m_IsSSAO_Blur = { true };	 
	_bool									m_IsStylized = { true };
	_float									m_fDebugRoughness[2] = {0.f, 0.4f};
	_float									m_fDebugMetallic[2] = {0.8f, 0.3f};
	_float									m_fShadowMapBias = {0.001f};
#endif
private:
	// Viewport Size 
	void						Setting_Viewport(_uint iWinSizeX, _uint iWinSizeY);
	void						Setting_Viewport(ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY);
	// Command List Merge
	void						Merge_CommandList(ID3D11CommandList* pCL, _uint iIndex);

private:
	void						Render_Priority();
	void						Render_Shadow();
	void						Render_NonBlend();	// 임시
	void						Render_Static();
	void						Render_Decal();
	void						Render_SSAO();
	void						Render_Outline_NonCompare();
	void						Render_Dynamic();
	void						Render_Light();
	void						Render_SSS();

	void						Render_Combined();
	void						Render_Water();
	void						Render_SSR();
	void						Render_Outline();
	void						Render_NonLight();
	void						Render_Emissive();	// 단독 Emissive
	void						Render_Effect();	// Backbuffer + Emissive + Distoriton
	void						Render_EffectResolve();	//Weight Blend Test
	void						Render_Bloom();		// Emissvie 처리
	void						Render_BloomCombined();
	void						Render_DistortionObject();
	void						Render_Blend();
	void						Render_LUT();
	void						Render_SFX();
	void						Render_Fog();
	void						Render_Distortion();
	void						Render_ScreenEffect();
	void						Render_UI();
	void						Render_UI_Post();
	void						Render_Fade();
	void						Render_NonStatic();

	void						Render_Setting();
#ifdef _DEBUG
	void						Render_Debug();
#endif

private:
	void						Render_ObjectList(_uint iRG_Index);

	HRESULT						Ready_RT();
	HRESULT						Ready_MRT();
	HRESULT						Ready_SubResource();
	HRESULT						Ready_RCS();
	HRESULT						Ready_Shadow_DSV();
	HRESULT						Ready_DC();

public:
	static		CRenderer*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumThread);
	virtual		void			Free() override;

};

NS_END