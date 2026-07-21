#include "EnginePch.h"
#include "GameInstance.h"

#include "Graphic_Device.h"
#include "Input_Device.h"
#include "Sound_Manager.h"
#include "Font_Manager.h"
#include "Level_Manager.h"
#include "Prototype_Manager.h"
#include "Object_Manager.h"
#include "Pooling_Manager.h"
#include "Target_Manager.h"
#include "Renderer.h"
#include "Timer_Manager.h"
#include "PhysicsManager.h"
#include "Camera_Manager.h"
#include "Sequence_Manager.h"
#include "EventBus.h"
#include "PipeLine.h"
#include "Light_Manager.h"
#include "Picking.h"
#include "GUIManager.h"
#include "OctoTree.h"
#include "Frustrum.h"
#include "CSM.h"
#include "UI_Manager.h"
#include "RCS_Manager.h"
#include "ShadowMap.h"
#include "Decal_Manager.h"
#include "VolumetricFog.h"
#include "HZB.h"
#include "EnvironmentMap.h"

#include "Model_Manager.h"
#include "SFX_Hub.h"
#include "Resource_Manager.h"
#include "Fade.h"


IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance()
{
}

#pragma region ENGINE
HRESULT CGameInstance::Ready_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.eMode, EngineDesc.iSizeX, EngineDesc.iSizeY, ppDevice, ppContext);
	ASSERT_CRASH(m_pGraphic_Device);

	m_pInput_Device = CInput_Device::Create(EngineDesc.hInst, EngineDesc.hWnd);
	ASSERT_CRASH(m_pInput_Device);

	m_pSound_Manager = CSound_Manager::Create(EngineDesc.iNumChannel);
	ASSERT_CRASH(m_pSound_Manager);

	m_pFont_Manager = CFont_Manager::Create(*ppDevice, *ppContext, EngineDesc.iSizeX, EngineDesc.iSizeY);
	ASSERT_CRASH(m_pFont_Manager);

	m_pLevel_Manager = CLevel_Manager::Create();
	ASSERT_CRASH(m_pLevel_Manager);
	m_iNumLevel = EngineDesc.iNumLevel;

	m_pPrototype_Manager = CPrototype_Manager::Create(m_iNumLevel);
	ASSERT_CRASH(m_pPrototype_Manager);
	
	m_pObject_Manager = CObject_Manager::Create(m_iNumLevel);
	ASSERT_CRASH(m_pObject_Manager);

	m_pPooling_Manager = CPooling_Manager::Create();
	ASSERT_CRASH(m_pPooling_Manager);

	m_pOctoTree = COctoTree::Create();
	ASSERT_CRASH(m_pOctoTree);

	m_pTargetManager = CTarget_Manager::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pTargetManager);

	m_pLight_Manager = CLight_Manager::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pLight_Manager);

	m_pCamera_Manager = CCamera_Manager::Create(*ppDevice, *ppContext, EngineDesc.iNumLevel);
	ASSERT_CRASH(m_pCamera_Manager);

	m_pSequence_Manager = CSequence_Manager::Create();
	ASSERT_CRASH(m_pSequence_Manager);

	m_pTimer_Manager = CTimer_Manager::Create();
	ASSERT_CRASH(m_pTimer_Manager);

	m_pPhysicsManager = CPhysicsManager::Create(*ppDevice, *ppContext, EngineDesc.iNumCollisionLayer);
	ASSERT_CRASH(m_pPhysicsManager);

	m_pEventBus = CEventBus::Create();
	ASSERT_CRASH(m_pEventBus);

	m_pPipeLine = CPipeLine::Create();
	ASSERT_CRASH(m_pPipeLine);

	m_pPicking = CPicking::Create(*ppDevice, *ppContext, EngineDesc.hWnd, EngineDesc.iSizeX, EngineDesc.iSizeY);
	ASSERT_CRASH(m_pPicking);

	m_pGUIManager = CGUIManager::Create(*ppDevice, *ppContext, EngineDesc.hWnd);
	ASSERT_CRASH(m_pGUIManager);

	m_pFrustrum = CFrustrum::Create();
	ASSERT_CRASH(m_pFrustrum);

	m_pCSM = CCSM::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pCSM);

	m_pHZB = CHZB::Create(*ppDevice, *ppContext, EngineDesc.iSizeX, EngineDesc.iSizeY);
	ASSERT_CRASH(m_pHZB);

	m_pUI_Manager = CUI_Manager::Create();
	ASSERT_CRASH(m_pUI_Manager);

	m_pRCS_Manager = CRCS_Manager::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pRCS_Manager);

	m_pRenderer = CRenderer::Create(*ppDevice, *ppContext, m_pPooling_Manager->Get_NumThread());
	ASSERT_CRASH(m_pRenderer);

	m_pShadowMap = CShadowMap::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pShadowMap);

	m_pDecal_Manager = CDecal_Manager::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pDecal_Manager);

	m_pVF = CVolumetricFog::Create(*ppDevice, *ppContext, EngineDesc.iSizeX, EngineDesc.iSizeY);
	ASSERT_CRASH(m_pVF);

	m_pModel_Manager = CModel_Manager::Create(*ppDevice, *ppContext, EngineDesc.iNumLevel);
	ASSERT_CRASH(m_pModel_Manager);

	m_pSFX_Hub = CSFX_Hub::Create(*ppDevice, *ppContext, EngineDesc.iSizeX, EngineDesc.iSizeY);
	ASSERT_CRASH(m_pSFX_Hub);

	m_pEnvMap = CEnvironmentMap::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pEnvMap);

	m_pResource_Manager = CResource_Manager::Create(*ppDevice, *ppContext);
	ASSERT_CRASH(m_pResource_Manager);

	m_pFade = CFade::Create(*ppDevice, *ppContext, EngineDesc.iSizeX, EngineDesc.iSizeY);
	ASSERT_CRASH(m_pFade);

	return S_OK;
}

void CGameInstance::Update_Engine(_float fTimeDelta)
{
	m_pTimer_Manager->Update(fTimeDelta); // Timer Manager Update => Time Stop 관련.

	m_pHZB->Update();

	m_pGUIManager->Update();

	m_pPicking->Update();
	m_pInput_Device->Update();

	m_pObject_Manager->Priority_Update(fTimeDelta);
	
	m_pObject_Manager->Update(fTimeDelta);

	m_pCamera_Manager->Update(fTimeDelta);
	m_pPhysicsManager->Update(fTimeDelta);
	m_pPhysicsManager->Late_Update();

	m_pObject_Manager->Late_Update(fTimeDelta);
	
	m_pDecal_Manager->Update(fTimeDelta);

	m_pSequence_Manager->Update(fTimeDelta);
	m_pCamera_Manager->Late_Update(fTimeDelta);
	m_pPipeLine->Update();

	m_pFrustrum->Update();
	m_pPooling_Manager->Add_Work([this]() {m_pCSM->Update_CSM(); });
	m_pOctoTree->Update();

	m_pPooling_Manager->Update_Pooling();

	m_pLevel_Manager->Update_Level(fTimeDelta);

	m_pLight_Manager->Update_Light();

	m_pVF->Update_VF(fTimeDelta);

	//m_pModel_Manager->Update(fTimeDelta);

	m_pSFX_Hub->Update_SFX(fTimeDelta);
	m_pFade->Priority_Update(fTimeDelta);
	m_pFade->Update(fTimeDelta);

	m_pSound_Manager->Update(fTimeDelta);
}

_float CGameInstance::Rand_Normal()
{
	return static_cast<_float>(rand()) / RAND_MAX;
}
_float CGameInstance::Rand(_float fMin, _float fMax)
{
	return Rand_Normal() * (fMax - fMin) + fMin;
}
#pragma endregion


#pragma region GRAPHIC_DEVICE
void CGameInstance::Render_Begin(const _float4* pClearColor)
{
	ASSERT_CRASH(m_pGraphic_Device);
	m_pGraphic_Device->Clear_BackBuffer_View(pClearColor);
	m_pGraphic_Device->Clear_DepthStencil_View();
}

HRESULT CGameInstance::Draw()
{
	ASSERT_CRASH(m_pRenderer);
	m_pRenderer->Render();

#ifdef KSTA_DEBUG_ENABLEFONTMGR
	ASSERT_CRASH(m_pFont_Manager);
	//m_pFont_Manager->Render();
#endif // KSTA_DEBUG_ENABLEFONTMGR


	ASSERT_CRASH(m_pLevel_Manager);
	m_pLevel_Manager->Render();

	ASSERT_CRASH(m_pFade);
	m_pFade->Render();

#ifdef _DEBUG
	ASSERT_CRASH(m_pPhysicsManager);
	m_pPhysicsManager->Render();
	ASSERT_CRASH(m_pHZB);
	m_pHZB->Render();
#endif
	ASSERT_CRASH(m_pGUIManager);
	m_pGUIManager->Render();

	return S_OK;
}

void CGameInstance::Render_End()
{
	m_pGraphic_Device->Present();
}
#pragma endregion

#pragma region INPUT_DEVICE
KEYSTATE CGameInstance::Get_DIKeyState(_ubyte byKeyID)
{
	return m_pInput_Device->Get_DIKeyState(byKeyID);
}
KEYSTATE CGameInstance::Get_DIMouseState(MOUSEKEYSTATE eState)
{
	return m_pInput_Device->Get_DIMouseState(eState);
}
_long CGameInstance::Get_DIMouseMove(MOUSEMOVESTATE eState)
{
	return m_pInput_Device->Get_DIMouseMove(eState);
}
#pragma endregion

#pragma region SOUND_MANAGER
void CGameInstance::Update_Listener(CTransform* pTransform, _float fTimeDelta)
{
	m_pSound_Manager->Update_Listener(pTransform, fTimeDelta);
}
_uint CGameInstance::Register_Channel()
{
	return m_pSound_Manager->Register_Channel();
}
void CGameInstance::Return_Channel(_uint iChannelIndex)
{
	m_pSound_Manager->Return_Channel(iChannelIndex);
}
HRESULT CGameInstance::Load_Sound(const _wstring& strSoundTag, const _char* pSoundFilePath, _bool is3D)
{
	return m_pSound_Manager->Load_Sound(strSoundTag, pSoundFilePath, is3D);
}
HRESULT CGameInstance::Load_Sound_FromFolder(const _char* pFolderPath, _bool is3D)
{
	return m_pSound_Manager->Load_Sound_FromFolder(pFolderPath, is3D);
}
HRESULT CGameInstance::Load_Sound_FromFolderRecursive(const _char* pFolderPath, _bool is3D)
{
	return m_pSound_Manager->Load_Sound_FromFolderRecursive(pFolderPath, is3D);
}
void CGameInstance::Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency)
{
	m_pSound_Manager->Play_Sound(strSoundTag, iChannelID, fVolume, fFrequency);
}
void CGameInstance::Play_Sound(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, CTransform* pTransform, _float fMinDistance, _float fMaxDistance, _float fFrequency)
{
	m_pSound_Manager->Play_Sound(strSoundTag, iChannelID, fVolume, pTransform, fMinDistance, fMaxDistance, fFrequency);
}
void CGameInstance::Play_Sound_Dynamic(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency)
{
	m_pSound_Manager->Play_Sound_Dynamic(strSoundTag, iChannelID, fVolume, fFrequency);
}
void CGameInstance::Play_Sound_Dynamic(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, CTransform* pTransform, _float fMinDistance, _float fMaxDistance, _float fFrequency)
{
	m_pSound_Manager->Play_Sound_Dynamic(strSoundTag, iChannelID, fVolume, pTransform, fMinDistance, fMaxDistance, fFrequency);
}
void CGameInstance::Play_BGM(const _wstring& strSoundTag, _uint iChannelID, _float fVolume, _float fFrequency)
{
	m_pSound_Manager->Play_BGM(strSoundTag, iChannelID, fVolume, fFrequency);
}
void CGameInstance::Stop_Sound(_uint iChannelID)
{
	m_pSound_Manager->Stop_Sound(iChannelID);
}
void CGameInstance::Stop_Sound_Dynamic(_uint iChannelID)
{
	m_pSound_Manager->Stop_Sound_Dynamic(iChannelID);
}
void CGameInstance::Stop_All()
{
	m_pSound_Manager->Stop_All();
}
void CGameInstance::Set_ChannelVolume(_uint iChannelID, _float fVolume)
{
	m_pSound_Manager->Set_ChannelVolume(iChannelID, fVolume);
}
void CGameInstance::Set_ChannelVolume_Dynamic(_uint iChannelID, _float fVolume)
{
	m_pSound_Manager->Set_ChannelVolume_Dynamic(iChannelID, fVolume);
}
#pragma endregion

#pragma region FONT_MANAGER
HRESULT CGameInstance::Add_Font(const _wstring& strFontTag, const _char* pFilePath, const _int iPixelHeight)
{
	return m_pFont_Manager->Add_Font(strFontTag, pFilePath, iPixelHeight);
}
FTCUSTOM_FONT* CGameInstance::Find_Font(const _wstring& strFontTag)
{
	return m_pFont_Manager->Find_Font(strFontTag);
}
const FTCUSTOM_FONT_GLYPH* CGameInstance::Get_Glyph(const _wstring& strFontTag, _uint code)
{
	return m_pFont_Manager->Get_Glyph(strFontTag, code);
}
ID3D11ShaderResourceView* CGameInstance::Get_AtlasSRV(const _wstring& strFontTag)
{
	return m_pFont_Manager->Get_AtlasSRV(strFontTag);
}
const FTCUSTOM_FONT_GLYPH* CGameInstance::Get_GlyphAndAdvance(const _wstring& strFontTag, _uint iCodePoint, _uint prevCodePoint, _int& outAdvanceX)
{
	return m_pFont_Manager->Get_GlyphAndAdvance(strFontTag, iCodePoint, prevCodePoint, outAdvanceX);
}

//HRESULT CGameInstance::Draw_Text(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRadian, const _float2& vOrigin, const _float2& vScale)
//{
//	return m_pFont_Manager->Draw_Text(strFontTag, pText, vPosition, vColor, fRadian, vOrigin, vScale);
//}
//_bool CGameInstance::Draw_Font(_wstring strFontTag, const _tchar* pText, _float2 vPos, _float fScale, _float4 vColor, _uint iShaderFlag)
//{
//	return m_pFont_Manager->Draw_Font(strFontTag, pText, vPos, fScale, vColor, iShaderFlag);
//}
//_bool CGameInstance::Draw_Font(FONT_SINGLEDESC* pSingleDesc)
//{
//	return m_pFont_Manager->Draw_Font(pSingleDesc);
//}
//void CGameInstance::Add_FloatingText(const _wstring& strFontTag, const _wstring& strText, _float2 vScreenPos, _float fScale, _float fLifeTime, _uint iShaderFlag, _float4 vColor)
//{
//	m_pFont_Manager->Add_FloatingText(strFontTag, strText, vScreenPos, fScale, fLifeTime, iShaderFlag, vColor);
//}
//void CGameInstance::Add_FloatingText(FONT_SINGLEDESC tDesc)
//{
//	m_pFont_Manager->Add_FloatingText(tDesc);
//}

#pragma endregion

#pragma region LEVEL_MANAGER
_uint CGameInstance::Get_CurrentLevel()
{
	return m_pLevel_Manager->Get_CurrentLevel();
}
HRESULT CGameInstance::Open_Level(_uint iNextLevelID, CLevel* pLevel)
{
	return m_pLevel_Manager->Open_Level(iNextLevelID, pLevel);
}
HRESULT CGameInstance::Clear_CurrentLevel_Resources(_uint iNextLevel)
{
	return m_pLevel_Manager->Clear_CurrentLevel_Resources(iNextLevel);
}
#pragma endregion

#pragma region PROTOTYPE_MANAGER
HRESULT CGameInstance::Add_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, CBase* pPrototype)
{
	return m_pPrototype_Manager->Add_Prototype(iPrototypeLevelID, strPrototypeTag, pPrototype);
}
void CGameInstance::Remove_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag)
{
	m_pPrototype_Manager->Remove_Prototype(iPrototypeLevelID, strPrototypeTag);
}
CBase* CGameInstance::Clone_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, PROTOTYPE eType, void* pArg)
{
	return m_pPrototype_Manager->Clone_Prototype(iPrototypeLevelID, strPrototypeTag, eType, pArg);
}
#pragma endregion

#pragma region OBJECT_MANAGER
HRESULT CGameInstance::Add_GameObject_ToLayer(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, void* pArg)
{
	return m_pObject_Manager->Add_GameObject_ToLayer(iPrototypeLevelID, strPrototypeTag, iLayerLevelID, strLayerTag, pArg);
}
HRESULT CGameInstance::Add_GameObject_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, CGameObject* pObject)
{
	return m_pObject_Manager->Add_GameObject_ToLayer(iLayerLevelID, strLayerTag, pObject);
}
CComponent* CGameInstance::Get_Component(_uint iLayerLevelID, const _wstring& strLayerTag, _uint iGameObjectIndex, const _wstring& strComponentTag)
{
	return m_pObject_Manager->Get_Component(iLayerLevelID, strLayerTag, iGameObjectIndex, strComponentTag);
}
HRESULT CGameInstance::Change_TimeRatio_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, _float fTimeRatio, _bool isTimeStop)
{
	return m_pObject_Manager->Change_TimeRatio_ToLayer(iLayerLevelID, strLayerTag, fTimeRatio, isTimeStop);
}
HRESULT CGameInstance::Change_TimeRatio_ToLayer(_uint iLayerLevelID, const _wstring& strLayerTag, _float fTimeRatio, _float fDuration)
{
	return m_pObject_Manager->Change_TimeRatio_ToLayer(iLayerLevelID, strLayerTag, fTimeRatio, fDuration);
}
#pragma endregion

#pragma region POOLING_MANAGER
_uint CGameInstance::Get_NumThread()
{
	return m_pPooling_Manager->Get_NumThread();
}
HRESULT CGameInstance::Add_PoolingObject(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg)
{
	return m_pPooling_Manager->Add_PoolingObject(iPrototypeLevelID, strPrototypeTag, iLayerLevelID, strLayerTag, strPoolingTag, iNumObjects, pArg);
}
HRESULT CGameInstance::Add_PoolingObject_ForStatic(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iLayerLevelID, const _wstring& strLayerTag, const _wstring& strPoolingTag, _uint iNumObjects, void* pArg)
{
	return m_pPooling_Manager->Add_PoolingObject_ForStatic(iPrototypeLevelID, strPrototypeTag, iLayerLevelID, strLayerTag, strPoolingTag, iNumObjects, pArg);
}
 HRESULT CGameInstance::Spawn_PoolingObject(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg)
{
	return m_pPooling_Manager->Spawn_PoolingObject(strPoolingTag, WorldMatrix, pArg);
}
 HRESULT CGameInstance::Spawn_PoolingObject_ForStatic(const _wstring& strPoolingTag, const _fmatrix& WorldMatrix, void* pArg)
 {
	 return m_pPooling_Manager->Spawn_PoolingObject_ForStatic(strPoolingTag, WorldMatrix, pArg);
 }
void CGameInstance::Add_Work(function<void()> Work)
{
	m_pPooling_Manager->Add_Work(Work);
}
void CGameInstance::Add_Render_Work(function<void()> Work)
{
	m_pPooling_Manager->Add_Render_Work(Work);
}
_bool CGameInstance::IsWorkFinish()
{
	return m_pPooling_Manager->IsWorkFinish();
}
void CGameInstance::Wait_Thread_End()
{
	m_pPooling_Manager->Wait_Thread_End();
}
#pragma endregion

#pragma region OctoTree
void CGameInstance::SetUp_OctoTree(_float3 vCenter, _float3 vExtent)
{
	m_pOctoTree->SetUp_OctoTree(vCenter, vExtent);
}

void CGameInstance::Add_To_OctoTree(CStaticObject* pObject, const BoundingBox* pBox)
{
	m_pOctoTree->Add_To_OctoTree(pObject, pBox);
}
#pragma endregion


#pragma region TARGET_MANAGER
ID3D11Resource* CGameInstance::Get_RT_Resource(const _wstring& strTargetTag)
{
	return m_pTargetManager->Get_RT_Resource(strTargetTag);
}
ID3D11ShaderResourceView* CGameInstance::Get_RT_SRV(const _wstring& strTargetTag)
{
	return m_pTargetManager->Get_RT_SRV(strTargetTag);
}
HRESULT CGameInstance::Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT eFormat, const _float4& vClearColor)
{
	return m_pTargetManager->Add_RenderTarget(strTargetTag, iWidth, iHeight, eFormat, vClearColor);
}
HRESULT CGameInstance::Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag)
{
	return m_pTargetManager->Add_MRT(strMRTTag, strTargetTag);
}
HRESULT CGameInstance::Bind_RenderTarget(const _wstring& strTargetTag, CShader* pShader, const _char* pConstantName)
{
	return m_pTargetManager->Bind_Shader_Resource(strTargetTag, pShader, pConstantName);
}
HRESULT CGameInstance::Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV, _bool isClear)
{
	return m_pTargetManager->Begin_MRT(strMRTTag, pDSV, isClear);
}
HRESULT CGameInstance::SetUp_MRT(ID3D11DeviceContext* pContext, const _wstring& strMRTTag)
{
    return m_pTargetManager->SetUp_MRT(pContext, strMRTTag);
}
void CGameInstance::End_MRT()
{
	m_pTargetManager->End_MRT();
}
HRESULT CGameInstance::Clear_RT(const _wstring& strTargetTag)
{
    return m_pTargetManager->Clear_RT(strTargetTag);
}
HRESULT CGameInstance::Bind_OpenRT(OPEN_RT eRT, CShader* pShader, const _char* pConstantName)
{
	return m_pTargetManager->Bind_OpenRT(eRT, pShader, pConstantName);
}
HRESULT CGameInstance::Render_RT()
{
	return m_pTargetManager->Render();
}
#ifdef _DEBUG
HRESULT CGameInstance::Ready_Debug_RT(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	return m_pTargetManager->Ready_Debug(strTargetTag, fX, fY, fSizeX, fSizeY);
}
HRESULT CGameInstance::Render_RT(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	return m_pTargetManager->Render(pShader, pVIBuffer);
}

ID3D11ShaderResourceView* CGameInstance::Get_Debug_RT_Resource(const _wstring& strTargetTag)
{
	return m_pTargetManager->Get_Debug_RT_Resource(strTargetTag);
}
void CGameInstance::Change_BackBufferColor(const _wstring& strTargetTag, const _float4& vClearColor)
{
	m_pTargetManager->Change_BackBufferColor(strTargetTag, vClearColor);
}
#endif
#pragma endregion

#pragma region RENDERER
HRESULT CGameInstance::Add_Render_Object(RENDERGROUP eGroup, CGameObject* pObject)
{
	return m_pRenderer->Add_Render_Object(eGroup, pObject);
}
HRESULT CGameInstance::Add_Render_StaticObject(CStaticObject* pObject)
{
	return m_pRenderer->Add_Render_StaticObject(pObject);
}
//HRESULT CGameInstance::Add_Render_StaticObject(const vector<class CStaticObject*>& Container)
//{
//    return m_pRenderer->Add_Render_StaticObject(Container);
//}
HRESULT CGameInstance::Add_Render_StaticObject(vector<class CStaticObject*>* Container)
{
	return m_pRenderer->Add_Render_StaticObject(Container);
}
HRESULT CGameInstance::Add_Render_ShadowMapObject(CGameObject* pRenderObject)
{
	return m_pRenderer->Add_Render_ShadowMapObject(pRenderObject);
}
void CGameInstance::Add_Effects(const _wstring& strEffectTag, const vector<ID3DX11Effect*> Effects)
{
	m_pRenderer->Add_Effects(strEffectTag, Effects);
}
ID3DX11Effect* CGameInstance::Get_Shader_Effect(const _wstring& strEffectTag, _uint iIndex)
{
    return m_pRenderer->Get_Shader_Effect(strEffectTag, iIndex);
}
void CGameInstance::Render_ShadowMap()
{
	m_pRenderer->Render_ShadowMap();
}
void CGameInstance::SettingFog(_bool IsOn)
{
	m_pRenderer->SettingFog(IsOn);
}
void CGameInstance::SettingSSS(_bool IsOn)
{
	m_pRenderer->SettingSSS(IsOn);
}
void CGameInstance::SettingHDR(_float fExposure)
{
	m_pRenderer->SettingHDR(fExposure);
}
ID3D11ShaderResourceView* CGameInstance::Get_CurrentSceneSRV()
{
	return m_pRenderer->Get_CurrentSceneSRV();
}
void CGameInstance::Get_Current_LutSetting(_uint* pOutIndex, _float* pOutIntensity, _bool* pOutIsDnyamicLut)
{
	m_pRenderer->Get_Current_LutSetting(pOutIndex, pOutIntensity, pOutIsDnyamicLut);
}
void CGameInstance::Setting_LUT(_uint iIndex, _float fLutLerpIntensity, _bool IsDynamicLut)
{
	m_pRenderer->Setting_LUT(iIndex, fLutLerpIntensity, IsDynamicLut);
}
#ifdef _DEBUG
HRESULT CGameInstance::Add_Render_Debug(CComponent* pDebugComponent)
{
	return m_pRenderer->Add_Render_Debug(pDebugComponent);
}
HRESULT CGameInstance::Bind_RawValue_Renderer(const _char* pConstantName, void* pValue, _uint iLength)
{
	return m_pRenderer->Bind_RawValue(pConstantName, pValue, iLength);
}
void CGameInstance::IsSSAO(_bool IsSSAO)
{
	m_pRenderer->IsSSAO(IsSSAO);
}
void CGameInstance::IsSSAO_Blur(_bool IsBlur)
{
	m_pRenderer->IsSSAO_Blur(IsBlur);
}
#endif
#pragma endregion

#pragma region LIGHT_MANAGER
const LIGHT_DESC* CGameInstance::Get_LightDesc(const _wstring& strLightTag)
{
	return m_pLight_Manager->Get_LightDesc(strLightTag);
}
void CGameInstance::Set_LightActive(const _wstring& strLightTag, _bool isActive)
{
	m_pLight_Manager->Set_Active(strLightTag, isActive);
}
HRESULT	CGameInstance::Add_Light(const _wstring& strLightTag, const LIGHT_DESC& LightDesc)
{
	return m_pLight_Manager->Add_Light(strLightTag, LightDesc);
}
HRESULT CGameInstance::Render_Light(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	return m_pLight_Manager->Render(pShader, pVIBuffer);
}
HRESULT CGameInstance::Render_LightEnvMap(CShader* pShader, CVIBuffer_Rect* pVIBuffer, BoundingBox* pBounding)
{
	return m_pLight_Manager->Render_EnvMap(pShader, pVIBuffer, pBounding);
}
HRESULT CGameInstance::Bind_LightDatas(CShader* pShader)
{
	return m_pLight_Manager->Bind_LightDatas(pShader);
}
HRESULT CGameInstance::Update_LightDesc(const _wstring& strLightTag, const LIGHT_DESC& LightDesc)
{
	return m_pLight_Manager->Update_LightDesc(strLightTag, LightDesc);
}
const vector<LIGHT_DATA>* CGameInstance::Get_LightDatas()
{
	return m_pLight_Manager->Get_LightDatas();
}
#ifdef _DEBUG
LIGHT_DESC* CGameInstance::Get_LightDesc_For_Map(const _wstring& strLightTag)
{
	return m_pLight_Manager->Get_LightDesc_For_Map(strLightTag);
}
#endif
#pragma endregion

#pragma region CAMERA_MANAGER
HRESULT CGameInstance::Add_Camera(_uint iLevelID, const _wstring& strCameraTag, CCamera* pCamera)
{
	return m_pCamera_Manager->Add_Camera(iLevelID, strCameraTag, pCamera);
}
HRESULT CGameInstance::Add_Camera(_uint iLevelID, const _wstring& strCameraTag, _uint iPrototypeLevelID, const _wstring& strPrototypeTag, void* pArg)
{
	return m_pCamera_Manager->Add_Camera(iLevelID, strCameraTag, iPrototypeLevelID, strPrototypeTag, pArg);
}
HRESULT CGameInstance::Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag)
{
	return m_pCamera_Manager->Change_MainCamera(iLevelID, strCameraTag);
}
HRESULT CGameInstance::Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag, void* pArg)
{
    return m_pCamera_Manager->Change_MainCamera(iLevelID, strCameraTag, pArg);
}
_float CGameInstance::Get_CurrentCamera_Near()
{
	return m_pCamera_Manager->Get_CurrentCamera_Near();
}
_float CGameInstance::Get_CurrentCamera_Far()
{
	return m_pCamera_Manager->Get_CurrentCamera_Far();
}
void CGameInstance::Set_CurrentCamera_Far(_float fFar)
{
	m_pCamera_Manager->Set_CurrentCamera_Far(fFar);
}
void CGameInstance::OnShake(const CAMERA_SHAKE& tData)
{
	m_pCamera_Manager->OnShake(tData);
}
#pragma endregion

#pragma region SEQUENCE_MANAGER
void CGameInstance::Register_Sequence(const _wstring& strSequenceTag, const vector<SEQUENCE_ITEM_INFO>& Items, const vector<SEQUENCE_ITEM_DATA*>& ItemDatas, void* pDesc)
{
	m_pSequence_Manager->Register_Sequence(strSequenceTag, Items, ItemDatas, pDesc);
}
void CGameInstance::Play_Sequence(const _wstring& strSequenceTag)
{
	m_pSequence_Manager->Play_Sequence(strSequenceTag);
}
#pragma endregion

#pragma region TIMER_MANAGER
_float CGameInstance::Get_TimeDelta(const _wstring& strTimerTag)
{
	return m_pTimer_Manager->Get_TimeDelta(strTimerTag);
}
_double CGameInstance::Get_PlayTime()
{
	return m_pTimer_Manager->Get_PlayTime();
}
void CGameInstance::Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate)
{
	m_pTimer_Manager->Change_TimeRate(strTimerTag, fTimeRate);
}
void CGameInstance::Change_TimeRate(const _wstring& strTimerTag, _float fTimeRate, _float fDuration)
{
	m_pTimer_Manager->Change_TimeRate(strTimerTag, fTimeRate, fDuration);
}
HRESULT CGameInstance::Add_Timer(const _wstring& strTimerTag)
{
	return m_pTimer_Manager->Add_Timer(strTimerTag);
}
#pragma endregion

#pragma region PHYSICS_MANAGER
void CGameInstance::IsChangeLevel_ForPhysicX(_bool isChangeLevel)
{
	m_pPhysicsManager->IsChangeLevel(isChangeLevel);
}
void CGameInstance::SetUp_PhysicsSystem()
{
	m_pPhysicsManager->SetUp_PhysicsSystem();
}
void CGameInstance::SetUp_ObjectToBP(_uint iObjectLayer, _uint iBPLayer)
{
	m_pPhysicsManager->SetUp_ObjectToBP(iObjectLayer, iBPLayer);
}
void CGameInstance::SetUp_ObjectFilter(_uint iSrc, _uint iDst)
{
	m_pPhysicsManager->SetUp_ObjectFilter(iSrc, iDst);
}
void CGameInstance::SetUp_ObjectVsBPFilter(_uint iObjectLayer, _uint iBPLayer)
{
	m_pPhysicsManager->SetUp_ObjectVsBPFilter(iObjectLayer, iBPLayer);
}
Body* CGameInstance::Register_Body(const BodyCreationSettings& BodySetting, BodyInterface** pOut)
{
	return m_pPhysicsManager->Register_Body(BodySetting, pOut);
}
Character* CGameInstance::Register_Character(const CharacterSettings& CharacterSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData)
{
	return m_pPhysicsManager->Register_Character(CharacterSetting, vPos, vQuat, pUserData);
}
Ref<CharacterVirtual> CGameInstance::Register_Virtual(const CharacterVirtualSettings& VirtualSetting, const Vec3& vPos, const Quat& vQuat, void* pUserData, BodyInterface** pOut)
{
    return m_pPhysicsManager->Register_CharacterVirtual(VirtualSetting, vPos, vQuat, pUserData, pOut);
}
void CGameInstance::Add_Virtual(CharacterVirtual* pVirtual, _uint iObjectLayer)
{
	m_pPhysicsManager->Add_Virtual(pVirtual, iObjectLayer);
}
void CGameInstance::Register_Virtual(CharacterVirtual* pVirtual)
{
	m_pPhysicsManager->Register_Virtual(pVirtual);
}
void CGameInstance::Remove_Virtual(CharacterVirtual* pVirtual)
{
	m_pPhysicsManager->Remove_Virtual(pVirtual);
}
_bool CGameInstance::Ray_Cast(_fvector vStartPos, _fvector vEndPos, _float4* pOut)
{
	return m_pPhysicsManager->Ray_Cast(vStartPos, vEndPos, pOut);
}

_bool CGameInstance::Ray_Cast(_fvector vStartPos, _fvector vEndPos, const uint16 iTargetObjectLayer, _float4* pOut)
{
	return m_pPhysicsManager->Ray_Cast(vStartPos, vEndPos, iTargetObjectLayer, pOut);
}

RAY_CAST_HIT CGameInstance::Ray_Cast(_fvector vStartPos, _fvector vEndPos, const uint16 iTargetObjectLayer)
{
	return m_pPhysicsManager->Ray_Cast(vStartPos, vEndPos, iTargetObjectLayer);
}


_bool CGameInstance::Shape_Cast(RefConst<Shape> pShape, _fvector vQuat, _fvector vPos, _fvector vDir, _float fDistance, uint16 iObjectLayer, SHAPE_CAST_HIT& OutHit)
{
	return m_pPhysicsManager->Shape_Cast(pShape, vQuat, vPos, vDir, fDistance, iObjectLayer, OutHit);
}

_bool CGameInstance::Sphere_Cast(_fvector vPos, _fvector vDir, _float fDistance, _float fRadius, uint16 iObjectLayer, SHAPE_CAST_HIT& OutHit)
{
	return m_pPhysicsManager->Sphere_Cast(vPos, vDir, fDistance, fRadius, iObjectLayer, OutHit);
}

_bool CGameInstance::Get_Body_AABB(const BodyID& ID, _float3& vOutMin, _float3& vOutMax)
{
	return m_pPhysicsManager->Get_Body_AABB(ID, vOutMin, vOutMax);
}

#ifdef _DEBUG
void CGameInstance::DrawShape(const Shape* pShape, RMat44 Matrix, JPH::Color BodyColor)
{
	m_pPhysicsManager->DrawShape(pShape, Matrix, BodyColor);
}
void CGameInstance::DrawRay(const _fvector& vStartPos, const _fvector& vEndPos)
{
	m_pPhysicsManager->DrawRay(vStartPos, vEndPos);
}

void CGameInstance::Add_DebugSphere(const _fvector& vCenter, _float fRadius, const JPH::Color& color)
{
	m_pPhysicsManager->Add_DebugSphere(vCenter, fRadius, color);
}

void CGameInstance::Add_DebugLine(const _fvector& vStart, const _fvector& vEnd, const JPH::Color& color)
{
	m_pPhysicsManager->Add_DebugLine(vStart, vEnd, color);
}

_bool CGameInstance::IsParkourDebug()
{
	return m_pPhysicsManager->IsParkourDebug();
}
#endif

#pragma endregion

#pragma region PIPELINE
const _float4x4* CGameInstance::Get_TransformState_Float4x4(D3DTS eState) const
{
	return m_pPipeLine->Get_TransformState_Float4x4(eState);
}

_matrix CGameInstance::Get_TransformState_Matrix(D3DTS eState) const
{
	return m_pPipeLine->Get_TransformState_Matrix(eState);
}

const _float4x4* CGameInstance::Get_TransformState_Float4x4_Inv(D3DTS eState) const
{
	return m_pPipeLine->Get_TransformState_Float4x4_Inv(eState);
}

_matrix CGameInstance::Get_TransformState_Matrix_Inv(D3DTS eState) const
{
	return m_pPipeLine->Get_TransformState_Matrix_Inv(eState);
}

const _float4x4* CGameInstance::Get_PrevTransformState_Float4x4(D3DTS eState) const
{
	return m_pPipeLine->Get_PrevTransformState_Float4x4(eState);
}

_matrix CGameInstance::Get_PrevTransformState_Matrix(D3DTS eState) const
{
	return m_pPipeLine->Get_PrevTransformState_Matrix(eState);
}

void CGameInstance::Set_TransformState(D3DTS eState, _fmatrix Matrix)
{
	m_pPipeLine->Set_TransformState(eState, Matrix);
}

void CGameInstance::Set_TransformState(D3DTS eState, const _float4x4& Matrix)
{
	m_pPipeLine->Set_TransformState(eState, Matrix);
}

void CGameInstance::Set_PrevTransformState(D3DTS eState, _fmatrix Matrix)
{
	m_pPipeLine->Set_PrevTransformState(eState, Matrix);
}

void CGameInstance::Set_PrevTransformState(D3DTS eState, const _float4x4& Matrix)
{
	m_pPipeLine->Set_PrevTransformState(eState, Matrix);
}

const _float4* CGameInstance::Get_CamPos() const
{
	return m_pPipeLine->Get_CamPos();
}
_float CGameInstance::Compute_Distance_ToCam(CGameObject* pObject)
{
	return m_pPipeLine->Compute_Distance(pObject);
}
#pragma endregion

#pragma region PICKING
POINT CGameInstance::Get_MousePoint()
{
	return m_pPicking->Get_MousePoint();
}
_bool CGameInstance::isPicked(_float3* pOut)
{
	return m_pPicking->isPicked(pOut);
}
_bool CGameInstance::GetCenterPos(_float3* pOut)
{
	return m_pPicking->GetCenterPos(pOut);
}
_bool CGameInstance::Get_Points(_float fRange, vector<_float4>& pOut, _float4* pOutMousePos)
{
	return m_pPicking->Get_Points(fRange, pOut, pOutMousePos);
}
#pragma endregion

#pragma region GUIMANAGER
ImGuiContext* CGameInstance::Get_ImGuiContext()
{
	return m_pGUIManager->Get_ImGuiContext();
}
void CGameInstance::Add_GUI_Func(function<void()> func)
{
	m_pGUIManager->Add_GUI_Func(func);
}
void CGameInstance::Use_Gizmo(CTransform* pTransform)
{
	m_pGUIManager->Use_Gizmo(pTransform);
}
void CGameInstance::Use_Gizmo_Offset(_float3* pScale, _float3* pRotation, _float3* pTranslation)
{
	m_pGUIManager->Use_Gizmo_Offset(pScale, pRotation, pTranslation);
}
void CGameInstance::Render_Gizmo(const _fmatrix& Matrix)
{
	m_pGUIManager->Render_Gizmo(Matrix);
}
#pragma endregion

#pragma region FRUSTRUM
const _float4* CGameInstance::Get_Frustrum_WorldPoints() const
{
	return m_pFrustrum->Get_Frustrum_WorldPoints();
}
_bool CGameInstance::IsIn_WorldSpace(_fvector vWorldPosition, _float fRange)
{
	return m_pFrustrum->IsIn_WorldSpace(vWorldPosition, fRange);
}
_bool CGameInstance::IsIn_WorldSpace(const BoundingBox* pBoundingBox)
{
	return m_pFrustrum->IsIn_WorldSpace(pBoundingBox);
}
_bool CGameInstance::IsIn_LocalSpace(_fmatrix WorldMatrix, _fvector vLocalPosition, _float fRange)
{
	return m_pFrustrum->IsIn_LocalSpace(WorldMatrix, vLocalPosition, fRange);
}
HRESULT CGameInstance::SetUp_ShadowLight(const _wstring& strLightTag)
{
	return m_pCSM->SetUp_ShadowLight(strLightTag);
}
HRESULT CGameInstance::Bind_CSM_Resources(CShader* pShader, const _char* pViewName, const _char* pProjName, const _char* pLightDirName)
{
	return m_pCSM->Bind_CSM_Resources(pShader, pViewName, pProjName, pLightDirName);
}
HRESULT CGameInstance::Bind_ShadowDistance_Resource(CShader* pShader, const _char* pDistanceName, const _char* pLastDistanceName)
{
	return m_pCSM->Bind_ShadowDistance_Resource(pShader, pDistanceName, pLastDistanceName);
}
HRESULT CGameInstance::Bind_CSM_SRV(CShader* pShader, const _char* pConstantName)
{
	return m_pCSM->Bind_CSM_SRV(pShader, pConstantName);
}
HRESULT CGameInstance::Begin_CSM()
{
	return m_pCSM->Begin_CSM();
}
HRESULT CGameInstance::End_CSM()
{
	return m_pCSM->End_CSM();
}
void CGameInstance::Render_CSM(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	m_pCSM->Render(pShader, pVIBuffer);
}
#pragma endregion

#pragma region HZB
void CGameInstance::Occlusion_Culling(vector<class CStaticObject*>& Objects)
{
	m_pHZB->Occlusion_Culling(Objects);
}
#pragma endregion

#pragma region UI_MANAGER
HRESULT	CGameInstance::Add_RootUI(const _wstring& strName_UI, class CUIObject* pRootUI)
{
	return m_pUI_Manager->Add_RootUI(strName_UI, pRootUI);
}

HRESULT	CGameInstance::Remove_RootUI(const _wstring& strName_UI)
{
	return m_pUI_Manager->Remove_RootUI(strName_UI);
}

CUIObject* CGameInstance::Find_UIObject(const _wstring& strName_UI)
{
	return m_pUI_Manager->Find_UIObject(strName_UI);
}

void	CGameInstance::Clear_RootUI()
{
	m_pUI_Manager->Clear_RootUI();
}
#pragma endregion


#pragma region RCS_MANAGER
HRESULT CGameInstance::Add_RCS(const _wstring& strRCSTag, void* pDesc)
{
	return m_pRCS_Manager->Add_RCS(strRCSTag, pDesc);
}
HRESULT CGameInstance::Add_BufferData(const _wstring& strRCSTag, const _char* pConstantName, void* pData, _uint iLength)
{
	return m_pRCS_Manager->Add_BufferData(strRCSTag, pConstantName, pData, iLength);
}
HRESULT CGameInstance::Add_SRVData(const _wstring& strRCSTag, const _char* pConstantName, ID3D11ShaderResourceView* pSRV)
{
	return m_pRCS_Manager->Add_SRVData(strRCSTag, pConstantName, pSRV);
}
HRESULT CGameInstance::Add_SamplerState(const _wstring& strRCSTag, _uint iSlotIndex, ID3D11SamplerState* pSampler)
{
	return m_pRCS_Manager->Add_SamplerState(strRCSTag, iSlotIndex, pSampler);
}
HRESULT CGameInstance::Setting_UAV_Data(const _wstring& strRCSTag, const _char* pConstantName)
{
	return m_pRCS_Manager->Setting_UAV_Data(strRCSTag, pConstantName);
}
HRESULT CGameInstance::Bind_RendererCS(const _wstring& strRCSTag, CShader* pShader, const _char* pConstantName, _uint iMipLevel)
{
	return m_pRCS_Manager->Bind_RendererCS(strRCSTag, pShader, pConstantName, iMipLevel);
}
HRESULT CGameInstance::Begin_RCS(const _wstring& strRCSTag, _uint iWidth, _uint iHeight, _uint iMipLevel)
{
	return m_pRCS_Manager->Begin_RCS(strRCSTag, iWidth, iHeight, iMipLevel);
}
void CGameInstance::Clear_RCS(const _wstring& strRCSTag, _uint iMipLevel)
{
	m_pRCS_Manager->Clear_RCS(strRCSTag, iMipLevel);
}
ID3D11ShaderResourceView* CGameInstance::Get_RCS_SRV(const _wstring& strRCSTag, _uint iMipLevel)
{
	return m_pRCS_Manager->Get_RCS_SRV(strRCSTag, iMipLevel);
}
HRESULT CGameInstance::Debug_Render_RCS()
{
	return m_pRCS_Manager->Debug_Render();
}
#pragma endregion

#pragma region SHADOW_MAP
HRESULT CGameInstance::Setting_ShadowMap(const SHADOW_MAP_DESC& MapDesc)
{
	return m_pShadowMap->Setting_ShadowMap(MapDesc);
}
const _uint CGameInstance::Get_ShadowMapLayer(_uint iSectorIndex)
{
	return m_pShadowMap->Get_ShadowMapLayer(iSectorIndex);
}
const vector<BoundingBox*>& CGameInstance::Get_ShadowMapSectors()
{
	return m_pShadowMap->Get_ShadowMapSectors();
}

HRESULT CGameInstance::Bind_ShadowMap_Resources_StaticObject(CShader* pShader, const _char* pViewName, const _char* pProjName, _uint iSector)
{
	return m_pShadowMap->Bind_ShadowMap_Resources(pShader, pViewName, pProjName, iSector);
}

HRESULT CGameInstance::Bind_ShadowMap_Resources_Renderer(CShader* pShader)
{
	return m_pShadowMap->Bind_ShadowMap_Resources(pShader);
}
HRESULT CGameInstance::Begin_ShadowMap()
{
	return m_pShadowMap->Begin_ShadowMap();
}

HRESULT CGameInstance::End_ShadowMap()
{
	return m_pShadowMap->End_ShadowMap();
}

void CGameInstance::Begin_DownSampleShadowMap()
{
	m_pShadowMap->DownSampleShadowMap();
}

ID3D11ShaderResourceView* CGameInstance::Get_ShadowMapDownSampleSRV()
{
	return m_pShadowMap->Get_DownSampleSRV();
}

ID3D11Buffer* CGameInstance::Get_ShadowMapDownSampleBuffer()
{
	return m_pShadowMap->Get_DownSampleBuffer();
}

void CGameInstance::Render_ShadowMap(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer)
{
	m_pShadowMap->Render(pShader, pVIBuffer);
}
#pragma endregion

#pragma region DECAL_MANAGER
HRESULT CGameInstance::Add_CustomDecal(CGameObject* pCustomDecalObject)
{
	return m_pDecal_Manager->Add_CustomDecal(pCustomDecalObject);
}
HRESULT CGameInstance::Add_Decal(const _wstring& strDecalTag, const _tchar* pFilePath[ENUM_CLASS(TEXTURETYPE::END)], _float3 vEmissiveLuminance)
{
	return m_pDecal_Manager->Add_Decal(strDecalTag, pFilePath, vEmissiveLuminance);
}
HRESULT CGameInstance::Add_DecalData(const _wstring& strDecalTag, const  DECAL_DATA& Decal)
{
	return m_pDecal_Manager->Add_DecalData(strDecalTag, Decal);
}
HRESULT CGameInstance::Render_Decal()
{
	return m_pDecal_Manager->Render();
}
#pragma endregion

#pragma region HZB
ID3D11ShaderResourceView* CGameInstance::Get_HZB_Resource()
{
	return m_pHZB->Get_Resource();
}
#pragma endregion

#pragma region VOLUMETRIC_FOG
HRESULT CGameInstance::Bind_VF_Resource(CShader* pShader, const _char* pTextureName, const _char* pFogRangeName)
{
	return m_pVF->Bind_VF_Resource(pShader, pTextureName, pFogRangeName);
}
void CGameInstance::Set_FogMaxHeight(_float fFogMaxHeight)
{
	m_pVF->Set_FogMaxHeight(fFogMaxHeight);
}
void CGameInstance::Set_FogDistanceFallOff(_float fDistanceFallOf)
{
	m_pVF->Set_FogDistanceFallOff(fDistanceFallOf);
}
void CGameInstance::Set_FogRayDensityScale(_float fFogRayDensityScale)
{
	m_pVF->Set_FogRayDensityScale(fFogRayDensityScale);
}
void CGameInstance::Set_FogScatterWeight(_float fFogScatterWeight)
{
	m_pVF->Set_FogScatterWeight(fFogScatterWeight);
}
void CGameInstance::Set_FogFarRatioToCameraFar(_float fFogFarRatio)
{
	m_pVF->Set_FogFarRatioToCameraFar(fFogFarRatio);
}
void CGameInstance::Set_FogRayIntensity(_float fRayIntensity)
{
	m_pVF->Set_FogRayIntensity(fRayIntensity);
}
void CGameInstance::Set_FogMaxDistance(_float fFogMaxDistance)
{
	m_pVF->Set_FogMaxDistance(fFogMaxDistance);
}
void CGameInstance::Begin_VF()
{
	m_pVF->Begin_VF();
}
#pragma endregion
HRESULT CGameInstance::RegisterPrototype(const _char* pFilePath, CModel_Streaming* pModel)
{
	return m_pModel_Manager->RegisterPrototype(pFilePath, pModel);
}

void CGameInstance::RequestData(CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex)
{
	m_pModel_Manager->RequestData(pModel, pFilePath, iLODIndex);
}

void CGameInstance::LoadLastLOD()
{
	m_pModel_Manager->LoadLastLOD();
 }

void CGameInstance::Bind_SharedBuffer(_uint iLODIndex, ID3D11DeviceContext** pDC, _uint iNumThread)
{
	m_pModel_Manager->Bind_SharedBuffer(iLODIndex, pDC, iNumThread);
}

void CGameInstance::Bind_SharedBuffer(_uint iLODIndex, ID3D11DeviceContext* pDC)
{
	m_pModel_Manager->Bind_SharedBuffer(iLODIndex, pDC);
}

void CGameInstance::SetUp_Data(class CModel_Streaming* pModel, const _string& pFilePath, _uint iLODIndex)
{
	m_pModel_Manager->SetUp_Data(pModel, pFilePath, iLODIndex);
}

void CGameInstance::Destroy_RigidData()
{
	m_pModel_Manager->Destroy_RigidData();
}

void CGameInstance::Model_Manager_Change_Level(_uint iLevel)
{
	m_pModel_Manager->Change_Level(iLevel);
}

#pragma region SFX_HUB
HRESULT CGameInstance::Begin_Toggle_SFX(SFX_TOGGLE eType, _float fDuration)
{
	return m_pSFX_Hub->Begin_Toggle_SFX(eType, fDuration);
}
HRESULT CGameInstance::End_SFX()
{
	return m_pSFX_Hub->End_SFX();
}
HRESULT CGameInstance::Setting_DOF(_float3 vCenterPos, _float fRange)
{
	return m_pSFX_Hub->Setting_DOF(vCenterPos, fRange);
}
HRESULT CGameInstance::Render_SFX_Toggle(CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	return m_pSFX_Hub->Render_SFX_Toggle(pVIBuffer, pShader);
}
HRESULT CGameInstance::Render_SFX(SFX_TYPE eType, CVIBuffer_Rect* pVIBuffer, CShader* pShader)
{
	return m_pSFX_Hub->Render_SFX(eType, pVIBuffer, pShader);
}
HRESULT CGameInstance::Setting_Radial(_float2 vCenterUV, _float2 vDistanceRange, _float fRadialIntensity)
{
	return m_pSFX_Hub->Setting_Radial(vCenterUV, vDistanceRange, fRadialIntensity);
}
HRESULT CGameInstance::Setting_Radial(_fvector vCenterPos, _float2 vDistanceRange, _float fRadialIntensity)
{
	return m_pSFX_Hub->Setting_Radial(vCenterPos, vDistanceRange, fRadialIntensity);
}
#ifdef _DEBUG
void CGameInstance::Set_Motion(_float fLimitVelocity, _float fLimitDepth, _float fLengthScale)
{
	m_pSFX_Hub->Set_Motion(fLimitVelocity, fLimitDepth, fLengthScale);
}
void CGameInstance::Set_SSR(_float fMinStep, _float fMaxStep, _float fStartOffset)
{
	m_pSFX_Hub->Set_SSR(fMinStep, fMaxStep, fStartOffset);
}
#endif
#pragma endregion

#pragma region ENVIRONMENT_MAP
HRESULT CGameInstance::Add_Probe(_float3 vCenter, _float fRange)
{
	return m_pEnvMap->Add_Probe(vCenter, fRange);
}
void CGameInstance::Bake_EnvMaps()
{
	m_pEnvMap->Bake_EnvMaps();
}
void CGameInstance::Add_EnvMap_SkyBox(CGameObject* pSkyBox)
{
	m_pEnvMap->Add_EnvMap_SkyBox(pSkyBox);
}
void CGameInstance::Add_EnvMap_StaticObject(CStaticObject* pStaticObject)
{
	m_pEnvMap->Add_EnvMap_StaticObject(pStaticObject);
}
HRESULT CGameInstance::Bind_EnvMapDatas(CShader* pShader, const _char* pTextureName, const _char* pBufferName, const _char* pHasEnvMapName, const _char* pNumEnvMapName)
{
	return m_pEnvMap->Bind_EnvMapDatas(pShader, pTextureName, pBufferName, pHasEnvMapName, pNumEnvMapName);
}
#pragma endregion

#pragma region RESOURCE_MANAGER
void CGameInstance::Load_Resource(const _char* pFolderPath)
{
	m_pResource_Manager->Load_Resource(pFolderPath);
}
ID3D11ShaderResourceView* CGameInstance::Get_Resource(const _string& strResourceTag)
{
	return m_pResource_Manager->Get_Resource(strResourceTag);
}
#pragma endregion

#pragma region FADE
void CGameInstance::OnFade(FADE eFade, _float fDuration, function<void()> func)
{
	m_pFade->OnFade(eFade, fDuration, func);
}
#pragma endregion

HRESULT CGameInstance::SetUp_CameraNF()
{
	m_pVF->SetUp_FogNF();
	return m_pCSM->SetUp_ShadowNF();
}

HRESULT CGameInstance::Clear_Resource(_uint iLevelID)
{
	m_pSound_Manager->Stop_All();

	if (FAILED(m_pCamera_Manager->Clear_Resource(iLevelID)))
		return E_FAIL;

	if (FAILED(m_pObject_Manager->Clear_Resource(iLevelID)))
		return E_FAIL;

	if (FAILED(m_pPrototype_Manager->Clear_Resource(iLevelID)))
		return E_FAIL;

	if (m_pModel_Manager)
		m_pModel_Manager->Clear_Resource(iLevelID);

	if(m_pEnvMap)
		m_pEnvMap->Clear();

	return S_OK;
}

HRESULT CGameInstance::Clear_Memory()
{
	m_pPhysicsManager->Clear_Resource();
	m_pResource_Manager->Clear_Resource();
	m_pRenderer->Clear_Resource();
	m_pOctoTree->Clear_OctoTree();
	m_pEventBus->Unscribe();
	m_pGUIManager->Clear_Func();
	m_pLight_Manager->Clear_Light();
	m_pCSM->Clear();
	m_pShadowMap->Clear();
	m_pEnvMap->Clear();
	m_pVF->Clear();
	
	//m_pDecal_Manager->Clear();

	if (FAILED(m_pPooling_Manager->Clear_Resource()))
		return E_FAIL;

	//if (FAILED(m_pSound_Manager->Clear_Resource()))
	//	return E_FAIL;

	return S_OK;
}

void CGameInstance::Release_Engine()
{
	m_pPooling_Manager->Drain_Works();
	Wait_Thread_End();
	Safe_Release(m_pPooling_Manager);
	Safe_Release(m_pRenderer);
	Safe_Release(m_pModel_Manager);
	Safe_Release(m_pGUIManager);																																																							
	Safe_Release(m_pLevel_Manager);
	Safe_Release(m_pFont_Manager);
	Safe_Release(m_pResource_Manager);
	Safe_Release(m_pOctoTree);
	Safe_Release(m_pObject_Manager);
	Safe_Release(m_pTargetManager);
	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pTimer_Manager);
	Safe_Release(m_pCamera_Manager);
	Safe_Release(m_pSequence_Manager);
	Safe_Release(m_pEventBus);
	Safe_Release(m_pPipeLine);
	Safe_Release(m_pPicking);
	Safe_Release(m_pShadowMap);
	Safe_Release(m_pDecal_Manager);
	Safe_Release(m_pInput_Device);
	Safe_Release(m_pFrustrum);
	Safe_Release(m_pVF);
	Safe_Release(m_pCSM);
	Safe_Release(m_pHZB);
	Safe_Release(m_pRCS_Manager);
	Safe_Release(m_pSFX_Hub);
	Safe_Release(m_pEnvMap);
	Safe_Release(m_pFade);
	Safe_Release(m_pUI_Manager);
	Safe_Release(m_pPhysicsManager);																									
	Safe_Release(m_pPrototype_Manager);
	Safe_Release(m_pSound_Manager);
	
	Safe_Release(m_pGraphic_Device);
	Release();
}

void CGameInstance::Free()
{
	__super::Free();
}