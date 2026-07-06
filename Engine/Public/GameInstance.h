#pragma once
/*
	[GameInstance]
	Client, Engine Connect
*/
#include "Prototype_Manager.h"
#include "EventBus.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)
private:
	explicit CGameInstance();
	virtual ~CGameInstance() = default;

#pragma region ENGINE
public:
	HRESULT			Ready_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	void				Update_Engine(_float fTimeDelta);

	// 0~1 Random
	_float				Rand_Normal();
	// Min, Max Random
	_float				Rand(_float fMin, _float fMax);
#pragma endregion

#pragma region GRAPHIC_DEVICE
public:
	void				Render_Begin(const _float4* pClearColor);
	HRESULT			Draw();
	void				Render_End();
#pragma endregion

#pragma region INPUT_DEVICE
public:
	KEYSTATE		Get_DIKeyState(_ubyte byKeyID);
	KEYSTATE		Get_DIMouseState(MOUSEKEYSTATE eState);
	_long			Get_DIMouseMove(MOUSEMOVESTATE eState);
#pragma endregion

#pragma region SOUND_MANAGER
	void			Update_Listener(class CTransform* pTransform, _float fTimeDelta);
	_uint			Register_Channel();
	void			Return_Channel(_uint iChannelIndex);

	HRESULT			Load_Sound(const _wstring& strSoundTag, const _char* pSoundFilePath, _bool is3D = false);
	HRESULT			Load_Sound_FromFolder(const _char* pFolderPath, _bool is3D = false);
	HRESULT			Load_Sound_FromFolderRecursive(const _char* pFolderPath, _bool is3D = false);
	void			Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency = 1.f);
	void			Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, class CTransform* pTransform, _float fMinDistance, _float fMaxDistance, _float fFrequency = 1.f);	// 3D
	void			Play_Sound_Dynamic(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency = 1.f);
	void			Play_Sound_Dynamic(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, class CTransform* pTransform, _float fMinDistance, _float fMaxDistance, _float fFrequency = 1.f);
	void			Play_BGM(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency = 1.f);
	void			Stop_Sound(_uint iChannelID);
	void			Stop_Sound_Dynamic(_uint iChannelID);
	void			Stop_All();
	void			Set_ChannelVolume(_uint iChannelID, _float fVolume);
	void			Set_ChannelVolume_Dynamic(_uint iChannelID, _float fVolume);
#pragma endregion

#pragma region FONT_MANAGER
	HRESULT		Add_Font(const _wstring& strFontTag, const _char* pFilePath, const _int iPixelHeight = 64U);
	//HRESULT		Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRadian = 0.f, const _float2& vOrigin = _float2(0.f, 0.f), const _float2& vScale = _float2(1.f, 1.f));
	//void		Add_FloatingText(const _wstring& strFontTag, const _tchar* pText, FONT_SINGLEDESC tSingleFontDesc);
	//_bool		Draw_Font(_wstring strFontTag, const _tchar* pText, _float2 vPos, _float fScale, _float4 vColor, _uint iShaderFlag);
	//_bool		Draw_Font(FONT_SINGLEDESC* pSingleDesc);
	//void		Add_FloatingText(const _wstring& strFontTag, const _wstring& strText, _float2 vScreenPos, _float fScale, _float fLifeTime, _uint iPassIndex, _float4 vColor);
	//void		Add_FloatingText(FONT_SINGLEDESC pDesc);
	FTCUSTOM_FONT*	Find_Font(const _wstring& strFontTag);
	const FTCUSTOM_FONT_GLYPH* Get_Glyph(const _wstring& strFontTag, _uint code);
	ID3D11ShaderResourceView* Get_AtlasSRV(const _wstring& strFontTag);
	const FTCUSTOM_FONT_GLYPH* Get_GlyphAndAdvance(const _wstring& strFontTag, _uint iCodePoint, _uint prevCodePoint, _int& outAdvanceX);
#pragma endregion


#pragma region LEVEL_MANAGER
public:
	_uint			Get_CurrentLevel();
	HRESULT			Open_Level(_uint iNextLevelID, class CLevel* pLevel);
	HRESULT			Clear_CurrentLevel_Resources(_uint iNextLevel);
#pragma endregion

#pragma region PROTOTYPE_MANAGER
	HRESULT		Add_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, CBase* pPrototype);
	void		Remove_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag);
	CBase*		Clone_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, PROTOTYPE eType, void* pArg = nullptr);
#pragma endregion
	 
#pragma region OBJECT_MANAGER
	HRESULT		Add_GameObject_ToLayer(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, void* pArg = nullptr);
	HRESULT		Add_GameObject_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, class CGameObject* pObject);
	class CComponent* Get_Component(_uint iLayerLevelID, const _wstring& strLayerTag, _uint iGameObjectIndex, const _wstring& strComponentTag);
	HRESULT		Change_TimeRatio_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, _float fTimeRatio, _bool isTimeStop);
	HRESULT		Change_TimeRatio_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, _float fTimeRatio, _float fDuration);
#pragma endregion

#pragma region POOLING_MANAGER
	_uint			Get_NumThread();
	HRESULT		Add_PoolingObject(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg = nullptr);
	HRESULT		Add_PoolingObject_ForStatic(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg = nullptr);
	HRESULT		Spawn_PoolingObject(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg = nullptr);
	HRESULT		Spawn_PoolingObject_ForStatic(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg = nullptr);
	// Thread Work Assign
	void			Add_Work(function<void()> Work);
	// Render Work Assing
	void			Add_Render_Work(function<void()> Work);
	// Thread Work Finish -> bool
	_bool			IsWorkFinish();
	// Thread Wait End
	void			Wait_Thread_End();
#pragma endregion

#pragma region OctoTree
	void			SetUp_OctoTree(_float3 vCenter, _float3 vExtent);
	void			Add_To_OctoTree(class CStaticObject* pObject, const BoundingBox* pBox);

#pragma endregion

#pragma region TARGET_MANAGER
	ID3D11Resource* Get_RT_Resource(const _wstring& strTargetTag);
	ID3D11ShaderResourceView* Get_RT_SRV(const _wstring& strTargetTag);
	HRESULT		Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT eFormat, const _float4& vClearColor);
	HRESULT		Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);
	HRESULT		Bind_RenderTarget(const _wstring& strTargetTag, class CShader* pShader, const _char* pConstantName);
	HRESULT		Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV = nullptr, _bool isClear = true);
	HRESULT		SetUp_MRT(ID3D11DeviceContext* pContext, const _wstring& strMRTTag);
	void		End_MRT();
	HRESULT		Clear_RT(const _wstring& strTargetTag);
	HRESULT		Bind_OpenRT(OPEN_RT eRT, CShader* pShader, const _char* pConstantName);
	HRESULT     Render_RT();
#ifdef _DEBUG
	HRESULT		Ready_Debug_RT(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT		Render_RT(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	ID3D11ShaderResourceView* Get_Debug_RT_Resource(const _wstring& strTargetTag);

	void					Change_BackBufferColor(const _wstring& strTargetTag, const _float4& vClearColor);
#endif
#pragma endregion


#pragma region RENDERER
public:
	HRESULT						Add_Render_Object(RENDERGROUP eGroup, class CGameObject* pObject);
	HRESULT						Add_Render_StaticObject(class CStaticObject* pObject);
	//HRESULT						Add_Render_StaticObject(const vector<class CStaticObject*>& Container);
	HRESULT						Add_Render_StaticObject(vector<class CStaticObject*>* Container);
	HRESULT						Add_Render_StaticObject(class CStaticObject* pRenderObject, _uint iNumLODIndex);
	HRESULT						Add_Render_ShadowMapObject(CGameObject* pRenderObject);
	void						Add_Effects(const _wstring& strEffectTag, const vector<ID3DX11Effect*> Effects);
	ID3DX11Effect*				Get_Shader_Effect(const _wstring& strEffectTag, _uint iIndex);
	void						Render_ShadowMap();
	void						SettingFog(_bool IsOn);
	void						SettingSSS(_bool IsOn);
	void						SettingHDR(_float fExposure);
	ID3D11ShaderResourceView*	Get_CurrentSceneSRV();
	void						Setting_LUT(_uint iIndex, _float fLutLerpIntensity, _bool IsDynamicLut);
	void						Get_Current_LutSetting(_uint* pOutIndex, _float* pOutIntensity, _bool* pOutIsDnyamicLut);
#ifdef _DEBUG
	HRESULT		Add_Render_Debug(class CComponent* pDebugComponent);
	HRESULT		Bind_RawValue_Renderer(const _char* pConstantName, void* pValue, _uint iLength);
	void		IsSSAO(_bool IsSSAO);
	void		IsSSAO_Blur(_bool IsBlur);
#endif
#pragma endregion

#pragma region LIGHT_MANAGER
	const LIGHT_DESC*			Get_LightDesc(const _wstring& strLightTag);
	void						Set_LightActive(const _wstring& strLightTag, _bool isActive);
	HRESULT						Add_Light(const _wstring& strLightTag, const LIGHT_DESC& LightDesc);
	HRESULT						Render_Light(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	HRESULT						Render_LightEnvMap(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer, BoundingBox* pBounding);
	HRESULT						Bind_LightDatas(class CShader* pShader);
	HRESULT						Update_LightDesc(const _wstring& strLightTag, const LIGHT_DESC& LightDesc);
	const vector<LIGHT_DATA>*	Get_LightDatas();
#ifdef _DEBUG
	LIGHT_DESC* Get_LightDesc_For_Map(const _wstring& strLightTag);
#endif
#pragma endregion

#pragma region CAMERA_MANAGER
	HRESULT			Add_Camera(_uint iLevelID, const _wstring& strCameraTag, class CCamera* pCamera);
	HRESULT			Add_Camera(_uint iLevelID, const _wstring& strCameraTag, _uint iPrototypeLevelID, const _wstring& strPrototypeTag, void* pArg);
	HRESULT			Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag);
	HRESULT			Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag, void* pArg);
	_float				Get_CurrentCamera_Near();
	_float				Get_CurrentCamera_Far();
	void				Set_CurrentCamera_Far(_float fFar);
	void				OnShake(const CAMERA_SHAKE& tData);
#pragma endregion

#pragma region SEQUENCE_MANAGER
	void				Register_Sequence(const _wstring& strSequenceTag, const vector<SEQUENCE_ITEM_INFO>& Items, const vector<SEQUENCE_ITEM_DATA*>& ItemDatas, void* pDesc);
	void				Play_Sequence(const _wstring& strSequenceTag);
#pragma endregion


#pragma region TIMER_MANAGER
public:
	_float			Get_TimeDelta(const _wstring& strTimerTag);
	_double			Get_PlayTime();
	void			Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate);
	void			Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate, _float fDuration);
	HRESULT			Add_Timer(const _wstring& strTimerTag);
#pragma endregion

#pragma region PHYSICS_MANAGER
	void					IsChangeLevel_ForPhysicX(_bool isChangeLevel);
	void					SetUp_PhysicsSystem();
	void					SetUp_ObjectToBP(_uint iObjectLayer, _uint iBPLayer);
	void					SetUp_ObjectFilter(_uint iSrc, _uint iDst);
	void					SetUp_ObjectVsBPFilter(_uint iObjectLayer, _uint iBPLayer);
	Body*					Register_Body(const BodyCreationSettings& BodySetting, BodyInterface** pOut);
	Character*				Register_Character(const CharacterSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData);
	Ref<CharacterVirtual>	Register_Virtual(const CharacterVirtualSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData, BodyInterface** pOut);
	void					Add_Virtual(CharacterVirtual* pVirtual, _uint iObjectLayer);
	void					Register_Virtual(CharacterVirtual* pVirtual);
	void					Remove_Virtual(CharacterVirtual* pVirtual);
	_bool					Ray_Cast(const _fvector& vStartPos, const _fvector& vEndPos, _float4* pOut);	// pOut->w = 0 반환함에 주의.
	//_bool					Box_Cast(const Shape* pShape, const _fvector& vPos, const _fvector& vQuat, const _fvector& vDir
	//	, _float fDistance, uint16 iObjectLayer, vector<BOX_CAST_HIT>& OutHits);
	_bool					Box_Cast(const class CRigidbody* pRigidbodyCom, const _fvector& vPos, const _fvector& vQuat, const _fvector& vDir
		, _float fDistance, uint16 iObjectLayer, vector<BOX_CAST_HIT>& OutHits);

#ifdef _DEBUG
	void					DrawShape(const Shape* pShape, RMat44 Matrix, Color BodyColor = Color(0.f, 255.f, 0.f, 1.f));
	void					DrawRay(const _fvector& vStartPos, const _fvector& vEndPos);
	_bool					IsParkourDebug();
#endif
#pragma endregion


#pragma region EVENTBUS
public:
	template<typename TEvent>
	void Subscribe(_uint iStatic, const _wstring& strEventTag, function<void(const TEvent&)> handler) { m_pEventBus->Subscribe(iStatic, strEventTag, handler); }
	void Publish(_uint iStatic, const _wstring& strEventTag, const class CEvent& event) { m_pEventBus->Publish(iStatic, strEventTag, event); }
	void Unscribe() { m_pEventBus->Unscribe(); };
#pragma endregion
	
#pragma region PIPELINE
public:
	const _float4x4*		Get_TransformState_Float4x4(D3DTS eState) const;
	_matrix					Get_TransformState_Matrix(D3DTS eState) const;

	const _float4x4*		Get_TransformState_Float4x4_Inv(D3DTS eState) const;
	_matrix					Get_TransformState_Matrix_Inv(D3DTS eState) const;

	const _float4x4*		Get_PrevTransformState_Float4x4(D3DTS eState) const;
	_matrix					Get_PrevTransformState_Matrix(D3DTS eState) const;

	void					Set_TransformState(D3DTS eState, _fmatrix Matrix);
	void					Set_TransformState(D3DTS eState, const _float4x4& Matrix);

	void					Set_PrevTransformState(D3DTS eState, _fmatrix Matrix);
	void					Set_PrevTransformState(D3DTS eState, const _float4x4& Matrix);
	
	const _float4*			Get_CamPos() const;
	_float					Compute_Distance_ToCam(class CGameObject* pObject);
#pragma endregion

#pragma region PICKING
public:
	POINT					Get_MousePoint();
	_bool					isPicked(_float3* pOut);
	_bool					GetCenterPos(_float3* pOut);
	_bool					Get_Points(_float fRange, vector<_float4>& pOut, _float4* pOutMousePos);
#pragma endregion

#pragma region GUIMANAGER
public:
	ImGuiContext*			Get_ImGuiContext();
	void					Add_GUI_Func(function<void()> func);
	void					Use_Gizmo(class CTransform* pTransform = nullptr);
	void					Use_Gizmo_Offset(_float3* pScale = nullptr, _float3* pRotation = nullptr, _float3* pTranslation = nullptr);
	void					Render_Gizmo(const _fmatrix& Matrix);
#pragma endregion

#pragma region FRUSTRUM
public:
	const _float4*		Get_Frustrum_WorldPoints() const;
	_bool				IsIn_WorldSpace(_fvector vWorldPosition, _float fRange);	
	_bool				IsIn_WorldSpace(const BoundingBox* pBoundingBox);
	_bool				IsIn_LocalSpace(_fmatrix WorldMatrix, _fvector vLocalPosition, _float fRange);	
#pragma endregion

#pragma region CSM
public:
	HRESULT				SetUp_ShadowLight(const _wstring& strLightTag);
	HRESULT				Bind_CSM_Resources(CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pLightDirName = nullptr);
	HRESULT				Bind_ShadowDistance_Resource(CShader* pShader, const _char* pDistanceName, const _char* pLastDistanceName);
	HRESULT				Bind_CSM_SRV(class CShader* pShader, const _char* pConstantName);
	HRESULT				Begin_CSM();
	HRESULT				End_CSM();

	void				Render_CSM(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#ifdef _DEBUG
#endif
#pragma endregion

#pragma region HZB
	void					Occlusion_Culling(vector<class CStaticObject*>& Objects);
#pragma endregion


#pragma region UI_MANAGER
public:
	HRESULT				Add_RootUI(const _wstring& strName_UI, class CUIObject* pRootUI);
	HRESULT				Remove_RootUI(const _wstring& strName_UI);
	class CUIObject*	Find_UIObject(const _wstring& strName_UI);
	void				Clear_RootUI();
#pragma endregion


#pragma region RCS_MANAGER
	HRESULT						Add_RCS(const _wstring& strRCSTag, void* pDesc);
	HRESULT						Add_BufferData(const _wstring& strRCSTag, const _char* pConstantName, void* pData, _uint iLength);
	HRESULT						Add_SRVData(const _wstring& strRCSTag, const _char* pConstantName, ID3D11ShaderResourceView* pSRV);
	HRESULT						Add_SamplerState(const _wstring& strRCSTag, _uint iSlotIndex, ID3D11SamplerState* pSampler);
	HRESULT						Setting_UAV_Data(const _wstring& strRCSTag, const _char* pConstantName);
	HRESULT						Bind_RendererCS(const _wstring& strRCSTag, CShader* pShader, const _char* pConstantName, _uint iMipLevel = 0);
	HRESULT						Begin_RCS(const _wstring& strRCSTag, _uint iWidth, _uint iHeight, _uint iMipLevel = 0);
	void						Clear_RCS(const _wstring& strRCSTag, _uint iMipLevel = 0);
	ID3D11ShaderResourceView*	Get_RCS_SRV(const _wstring& strRCSTag, _uint iMipLevel = 0);

	HRESULT						Debug_Render_RCS();
#pragma endregion

#pragma region SHADOW_MAP
	HRESULT						Setting_ShadowMap(const SHADOW_MAP_DESC& MapDesc);
	const _uint					Get_ShadowMapLayer(_uint iSectorIndex);
	const vector<BoundingBox*>& Get_ShadowMapSectors();
	HRESULT						Bind_ShadowMap_Resources_StaticObject(CShader* pShader, const _char* pViewName, const _char* pProjName, _uint iSector);
	HRESULT						Bind_ShadowMap_Resources_Renderer(CShader* pShader);
	HRESULT						Begin_ShadowMap();
	HRESULT						End_ShadowMap();
	void						Begin_DownSampleShadowMap();
	ID3D11ShaderResourceView*	Get_ShadowMapDownSampleSRV();
	ID3D11Buffer*				Get_ShadowMapDownSampleBuffer();
	void						Render_ShadowMap(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

#pragma endregion

#pragma region DECAL_MANAGER
public:
	HRESULT						Add_CustomDecal(class CGameObject* pCustomDecalObject);
	HRESULT						Add_Decal(const _wstring& strDecalTag, const _tchar* pFilePath[ENUM_CLASS(TEXTURETYPE::END)], _float3 vEmissiveLuminance = _float3(0.f, 0.f, 0.f));
	HRESULT						Add_DecalData(const _wstring& strDecalTag, const  DECAL_DATA& Decal);
	HRESULT						Render_Decal();
#pragma endregion

#pragma region HZB
	ID3D11ShaderResourceView*	Get_HZB_Resource();
#pragma endregion

#pragma region VOLUMETRIC_FOG
public:
	HRESULT						Bind_VF_Resource(CShader* pShader, const _char* pTextureName, const _char* pFogRangeName);
	void						Set_FogMaxHeight(_float fFogMaxHeight);
	void						Set_FogDistanceFallOff(_float fDistanceFallOf);
	void						Set_FogRayDensityScale(_float fFogRayDensityScale);
	void						Set_FogScatterWeight(_float fFogScatterWeight);
	void						Set_FogFarRatioToCameraFar(_float fFogFarRatio);
	void						Set_FogRayIntensity(_float fRayIntensity);
	void						Set_FogMaxDistance(_float fFogMaxDistance);
	void						Begin_VF();
#pragma endregion

#pragma region MODEL_STREAMING
public:
	HRESULT						RegisterPrototype(const _char* pFilePath, class CModel_Streaming* pModel);
	void						RequestData(class CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex);
	void						LoadLastLOD();
	void						Bind_SharedBuffer(_uint iLODIndex, ID3D11DeviceContext** pDC, _uint iNumThread);
	void						Bind_SharedBuffer(_uint iLODIndex, ID3D11DeviceContext* pDC);
	void						SetUp_Data(class CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex);
	void						Destroy_RigidData();
	void						Model_Manager_Change_Level(_uint iLevel);
#pragma endregion


#pragma region SFX_HUB
	HRESULT						Begin_Toggle_SFX(SFX_TOGGLE eType, _float fDuration = 0.f);
	HRESULT						End_SFX();
	HRESULT						Setting_DOF(_float3 vCenterPos, _float fRange);

	HRESULT						Render_SFX_Toggle(CVIBuffer_Rect* pVIBuffer, CShader* pShader);
	HRESULT						Render_SFX(SFX_TYPE eType, CVIBuffer_Rect* pVIBuffer, CShader* pShader);
	HRESULT						Setting_Radial(_float2 vCenterUV, _float2 vDistanceRange, _float fRadialIntensity);
	HRESULT						Setting_Radial(_fvector  vCenterPos, _float2 vDistanceRange, _float fRadialIntensity);


#ifdef _DEBUG
	void					Set_Motion(_float fLimitVelocity, _float fLimitDepth, _float fLengthScale);
	void					Set_SSR(_float fMinStep, _float fMaxStep, _float fStartOffset);
#endif
#pragma endregion

#pragma region ENVIRONMENT_MAP
	HRESULT						Add_Probe(_float3 vCenter, _float fRange);
	void						Bake_EnvMaps();
	void						Add_EnvMap_SkyBox(CGameObject* pSkyBox);
	void						Add_EnvMap_StaticObject(CStaticObject* pStaticObject);
	HRESULT						Bind_EnvMapDatas(CShader* pShader, const _char* pTextureName, const _char* pBufferName, const _char* pHasEnvMapName, const _char* pNumEnvMapName);
#pragma endregion

#pragma region RESOURCE_MANAGER
	void									Load_Resource(const _char* pFolderPath);
	ID3D11ShaderResourceView*	Get_Resource(const _string& strResourceTag);
#pragma endregion

#pragma region Fade
	void						OnFade(FADE eFade, _float fDuration, function<void()> func);
#pragma endregion


public:
	HRESULT					SetUp_CameraNF();
	HRESULT					Clear_Resource(_uint iLevelID);
	HRESULT					Clear_Memory();
	void					Release_Engine();

private:
	class CGraphic_Device*		m_pGraphic_Device = { nullptr };
	class CInput_Device*		m_pInput_Device = { nullptr };
	class CSound_Manager*		m_pSound_Manager = { nullptr };
	class CFont_Manager*		m_pFont_Manager = { nullptr };
	class CLevel_Manager*		m_pLevel_Manager = { nullptr };
	class CPrototype_Manager*	m_pPrototype_Manager = { nullptr };
	class CObject_Manager*		m_pObject_Manager = { nullptr };
	class CPooling_Manager*		m_pPooling_Manager = { nullptr };
	class COctoTree*			m_pOctoTree = { nullptr };
	class CTarget_Manager*		m_pTargetManager = { nullptr };
	class CRenderer*			m_pRenderer = { nullptr };
	class CLight_Manager*		m_pLight_Manager = { nullptr };
	class CCamera_Manager*		m_pCamera_Manager = { nullptr };
	class CSequence_Manager*	m_pSequence_Manager = { nullptr };
	class CTimer_Manager*		m_pTimer_Manager = { nullptr };
	class CPhysicsManager*		m_pPhysicsManager = { nullptr };
	class CEventBus*			m_pEventBus = { nullptr };
	class CPipeLine*			m_pPipeLine = { nullptr };
	class CPicking*				m_pPicking = { nullptr };
	class CGUIManager*			m_pGUIManager = { nullptr };
	class CFrustrum*			m_pFrustrum = { nullptr };
	class CUI_Manager*			m_pUI_Manager = { nullptr };
	class CCSM*						m_pCSM = { nullptr };
	class CHZB*						m_pHZB = { nullptr };
	class CRCS_Manager*			m_pRCS_Manager = { nullptr };
	class CShadowMap*			m_pShadowMap = { nullptr };
	class CDecal_Manager*		m_pDecal_Manager = { nullptr };
	class CVolumetricFog*		m_pVF = { nullptr };
	class CModel_Manager*		m_pModel_Manager = { nullptr };
	class CSFX_Hub*				m_pSFX_Hub = { nullptr };
	class CEnvironmentMap*	m_pEnvMap = { nullptr };
	class CResource_Manager*	m_pResource_Manager = { nullptr };
	class CFade*					m_pFade = { nullptr };

	_uint						m_iNumLevel = {};


#ifdef _DEBUG
private:
	_bool m_isParkourDebug = { false };
#endif 




public:
	virtual void Free() override;

};

NS_END