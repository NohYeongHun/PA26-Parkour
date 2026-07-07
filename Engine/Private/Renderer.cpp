#include "EnginePch.h"
#include "Renderer.h"

#include "GameInstance.h"
#include "GameObject.h"
#include "StaticObject.h"
#include "RendererSubResource.h"
#include "RendererCS.h"

CRenderer::CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice },
	m_pContext { pContext },
	m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CRenderer::Initialize(_uint iNumThread)
{
	m_iNumThread = iNumThread;
	m_CommandLists.resize(m_iNumThread);

	_uint iNumViewPort = { 1 };
	D3D11_VIEWPORT ViewPort = {};
	m_pContext->RSGetViewports(&iNumViewPort, &ViewPort);

	m_iWinSizeX = static_cast<_uint>(ViewPort.Width);
	m_iWinSizeY = static_cast<_uint>(ViewPort.Height);

	m_fWinSizeX = ( ViewPort.Width );
	m_fWinSizeY = ( ViewPort.Height );

	m_fExposure = 0.6f;

	if (FAILED(Ready_RT()))
		return E_FAIL;
	if (FAILED(Ready_MRT()))
		return E_FAIL;
	if (FAILED(Ready_SubResource()))
		return E_FAIL;
	if (FAILED(Ready_RCS()))
		return E_FAIL;
	if (FAILED(Ready_DC()))
		return E_FAIL;

	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		CRASH("Buffer Fail");

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Engine_Shader_Deferred.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements);
	if (nullptr == m_pShader)
		CRASH("Shader Fail");
	
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(static_cast<_float>(m_iWinSizeX), static_cast<_float>(m_iWinSizeY), 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(static_cast<_float>( m_iWinSizeX ), static_cast<_float>( m_iWinSizeY ), 0.f, 1.f));
;
	m_iInterval = 2;

#ifdef _DEBUG
	if (FAILED(m_pGameInstance->Ready_Debug_RT(TEXT("RT_Diffuse"), 150.0f, 150.0f, 300.f, 300.f)))
		return E_FAIL;
#endif

	return S_OK;
}

HRESULT CRenderer::Add_Render_Object(RENDERGROUP eRenderGroup, CGameObject* pRenderObject)
{
	if (nullptr == pRenderObject)
		return E_FAIL;

	m_RenderObjects[ENUM_CLASS(eRenderGroup)].push_back(pRenderObject);
	Safe_AddRef(pRenderObject);

	return S_OK;
}

HRESULT CRenderer::Add_Render_StaticObject(CStaticObject* pRenderObject)
{
	if (8 == m_iCullStack.load(memory_order_acquire))
	{
		cout << "Cut!" << endl;
		return S_OK;
	}

	_int iWriteIndex = m_iDoubleBufferIndex.load(memory_order_acquire);
	{
		lock_guard<recursive_mutex> lock(m_RecursiveMutex);
		m_StaticObjects[iWriteIndex][pRenderObject->Get_LOD()].push_back(pRenderObject);
	}

	return S_OK;
}

HRESULT CRenderer::Add_Render_StaticObject(CStaticObject* pRenderObject, _uint iNumLODIndex)
{
	_int iWriteIndex = m_iDoubleBufferIndex.load(memory_order_acquire);
	{
		lock_guard<recursive_mutex> lock(m_RecursiveMutex);
		m_StaticObjects[iWriteIndex][iNumLODIndex].push_back(pRenderObject);
	}

	return S_OK;
}

HRESULT CRenderer::Add_Render_StaticObject(vector<class CStaticObject*>* Container)
{
	if (8 == m_iCullStack.load(memory_order_acquire))
		return S_OK;

	_int iWriteIndex = m_iDoubleBufferIndex.load(memory_order_acquire);
	{
		lock_guard<recursive_mutex> lock(m_RecursiveMutex);
		for (_uint i = 0; i < 4; ++i)
			m_StaticObjects[iWriteIndex][i].insert(m_StaticObjects[iWriteIndex][i].end(), Container[i].begin(), Container[i].end());
		m_iCullStack.fetch_add(1, memory_order_release);
	}

	if (8 <= m_iCullStack.load(memory_order_acquire))
	{
		//m_iNumPreRenderObject = m_StaticObjects[iWriteIndex].size();
		m_isCompleteFrustumCull.exchange(true, memory_order_release);
	}

	return S_OK;
}
HRESULT CGameInstance::Add_Render_StaticObject(CStaticObject* pRenderObject, _uint iNumLODIndex)
{
	return m_pRenderer->Add_Render_StaticObject(pRenderObject, iNumLODIndex);
}
HRESULT CRenderer::Add_Render_ShadowMapObject(CGameObject* pRenderObject)
{
	{
		lock_guard<recursive_mutex> lock(m_RecursiveMutex);
		m_ShadowMapObjects.push_back(pRenderObject);
	}

	return S_OK;
}

void CRenderer::Render()
{
#ifndef _DEBUG
	if (m_pGameInstance->Get_DIKeyState(DIK_HOME) == KEYSTATE::DOWN)
	{
		//m_IsSSAO = !m_IsSSAO;
		m_IsFog = !m_IsFog;
	}
#endif // !_DEBUG

	//m_pGameInstance->Wait_Thread_End();
	m_iCurTime = (++m_iCurTime) % m_iInterval;
	
	Render_Priority();
	Render_Shadow();
	Render_NonBlend();
	Render_Static();
	Render_Decal();
	Render_NonStatic();
	Render_SSAO();			
	Render_Outline_NonCompare();
	Render_Dynamic();

	Render_Light();
	Render_SSS();

	Render_Combined();
	
	Render_Water();
	Render_SSR();
	Render_Outline();

	Render_NonLight();
	Render_LUT();
	Render_Fog();
	Render_Effect();
	Render_EffectResolve();
	Render_SFX();
	Render_Emissive();	
	Render_Bloom();
	Render_BloomCombined();
	Render_DistortionObject();
	Render_Blend(); 
	Render_Distortion();
	Render_ScreenEffect();
	Render_UI();
	Render_UI_Post();
	Render_Fade();

	Render_Setting();

#ifdef _DEBUG
	Render_Debug();
#endif
}

void CRenderer::Add_Effects(const _wstring& strEffectTag, const vector<ID3DX11Effect*> Effects)
{
	auto iter = m_Effects.find(strEffectTag);

	if (iter != m_Effects.end())
		CRASH("Tag");

	m_Effects.emplace(strEffectTag, Effects);
}

ID3DX11Effect* CRenderer::Get_Shader_Effect(const _wstring& strEffectTag, _uint iIndex)
{
	auto iter = m_Effects.find(strEffectTag);

	if (iter == m_Effects.end())
		return nullptr;

	if (iter->second.size() <= iIndex)
		return nullptr;

	return iter->second[iIndex];
}

#ifdef _DEBUG
HRESULT CRenderer::Add_Render_Debug(CComponent* pDebugComponent)
{
	if (nullptr == pDebugComponent)
		return E_FAIL;

	m_DebugComponents.push_back(pDebugComponent);
	Safe_AddRef(pDebugComponent);

	return S_OK;
}
HRESULT CRenderer::Bind_RawValue(const _char* pConstantName, void* pValue, _uint iLength)
{
	return m_pShader->Bind_Value(pConstantName, pValue, iLength);
}
#endif

void CRenderer::Setting_Viewport(_uint iWinSizeX, _uint iWinSizeY)
{
	D3D11_VIEWPORT Viewport = {};
	Viewport.TopLeftX = 0.f;
	Viewport.TopLeftY = 0.f;
	Viewport.Width = static_cast<_float>(iWinSizeX);
	Viewport.Height = static_cast<_float>(iWinSizeY);
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &Viewport);
}

void CRenderer::Setting_Viewport(ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	D3D11_VIEWPORT Viewport = {};
	Viewport.TopLeftX = 0.f;
	Viewport.TopLeftY = 0.f;
	Viewport.Width = static_cast<_float>(iWinSizeX);
	Viewport.Height = static_cast<_float>(iWinSizeY);
	Viewport.MinDepth = 0.f;
	Viewport.MaxDepth = 1.f;

	pContext->RSSetViewports(1, &Viewport);
}

void CRenderer::Merge_CommandList(ID3D11CommandList* pCL, _uint iIndex)
{
	lock_guard<mutex> lock(m_RenderMutex);
	m_CommandLists[iIndex] = pCL;
}

void CRenderer::Get_Current_LutSetting(_uint* pOutIndex, _float* pOutIntensity, _bool* pOutIsDynamicLut)
{
	if(nullptr != pOutIndex)
		*pOutIndex = m_iLUT_Index;
	
	if(nullptr != pOutIntensity)
		*pOutIntensity = m_fLutLerpIntensity;

	if (nullptr != pOutIsDynamicLut)
		*pOutIsDynamicLut = m_IsDynamicLUT;
}

void CRenderer::Render_ShadowMap()
{
	if (m_ShadowMapObjects.empty())
		return;

	m_pGameInstance->Begin_ShadowMap();

	for (auto& pRenderObject : m_ShadowMapObjects)
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_Shadow();
	}

	m_ShadowMapObjects.clear();

	m_pGameInstance->End_ShadowMap();
}

void CRenderer::Render_LOD(_uint iLODIndex)
{
	_int iReadIndex = (m_iDoubleBufferIndex + 1) % 2;

	// Static Object Render
	for (_uint iLOD = 0; iLOD < 4; ++iLOD)
	//_uint iLOD = 0;
	{
		_uint iNumObjects = static_cast<_uint>(max(1, m_StaticObjects[iReadIndex][iLOD].size() / m_iNumThread));
		for (_uint i = 0; i < m_iNumThread; ++i)
		{
			_uint iStartIndex = i * iNumObjects;
			_uint iEndIndex = static_cast<_uint>(min((i + 1) * iNumObjects, m_StaticObjects[iReadIndex][iLOD].size()));
			if (i == m_iNumThread - 1)
				//iEndIndex = m_pGameInstance->Render_ObjectsNum(iLOD);
				iEndIndex = static_cast<_uint>(m_StaticObjects[iReadIndex][iLOD].size());

			m_pGameInstance->Add_Render_Work([this, iLOD, iStartIndex, iEndIndex, iReadIndex, i]() {
				//쓰레드 개수로 분할해서 해야한다ㅇㅇ
				if (iStartIndex < iEndIndex)
				{
					m_pDeferredContext[i]->ClearState();
					Setting_Viewport(m_pDeferredContext[i], static_cast<_uint>(m_fWinSizeX), static_cast<_uint>(m_fWinSizeY));
					m_pGameInstance->SetUp_MRT(m_pDeferredContext[i], TEXT("MRT_Object"));
					m_pGameInstance->Bind_SharedBuffer(iLOD, m_pDeferredContext[i]);
					for (_uint iIndex = iStartIndex; iIndex < iEndIndex; ++iIndex)
					{
						m_StaticObjects[iReadIndex][iLOD][iIndex]->Set_LOD(iLOD);
						m_StaticObjects[iReadIndex][iLOD][iIndex]->Render(m_pDeferredContext[i], i);
						m_StaticObjects[iReadIndex][iLOD][iIndex]->Set_RenderTime(iLOD, static_cast<_float>(m_pGameInstance->Get_PlayTime()));
					}

					ID3D11CommandList* pCL = { nullptr };
					m_pDeferredContext[i]->FinishCommandList(false, &pCL);
					Merge_CommandList(pCL, i);
				}

				{
					lock_guard<mutex> lock(m_RenderAddMutex);
					m_iNumEndThread.fetch_add(1, memory_order_relaxed);
				}
				m_CV.notify_one();
				});
		}
		//이거 지호형한테 물어볼것. 얘 떄문에 프레임 떨어지는 건지 체크 하고싶음.
		{
			unique_lock<mutex> lock(m_RenderAddMutex);
			m_CV.wait(lock, [&]() { return m_iNumEndThread == m_iNumThread; });
			m_iNumEndThread.store(0, memory_order_release);
		}
		// CommandLists Execute
		for (auto& pCL : m_CommandLists)
		{
			if (nullptr != pCL)
			{
				m_pContext->ExecuteCommandList(pCL, true);
				Safe_Release(pCL);
			}
		}
	}

}

void CRenderer::Render_LOD_Weight()
{
	_int iReadIndex = (m_iDoubleBufferIndex + 1) % 2;


	const _float fLODWeights[4] = { 3.f,2.f,1.f,0.5f };

	_float dTotalRenderCost = 0.f;
	_float dLODCost[4] = { 0.f };

	_uint iAssignedThreads[4] = { 0 };
	_uint iUsedThreads = { 0 };
	for (_uint i = 0; i < 4; ++i)
	{
		size_t ObjCnt = m_StaticObjects[iReadIndex][i].size();
		if (ObjCnt > 0)
		{
			dLODCost[i] = static_cast<_float>(ObjCnt) * fLODWeights[i];
			dTotalRenderCost += dLODCost[i];
		}
	}
	if (dTotalRenderCost == 0.f)
		return;

	for (_uint i = 0; i < 4; ++i)
	{
		if (dLODCost[i] > 0)
		{
			_float RenderRatio = dLODCost[i] / dTotalRenderCost;

			_uint ThreadCnt = static_cast<_uint>((RenderRatio * m_iNumThread));
			if (ThreadCnt == 0 && m_StaticObjects[iReadIndex][i].size() > 0)
				ThreadCnt = 1;
			iAssignedThreads[i] = ThreadCnt;
			iUsedThreads += ThreadCnt;
		}
	}

	if (iUsedThreads < m_iNumThread && m_StaticObjects[iReadIndex][0].size() > 0)
	{
		iAssignedThreads[0] += (m_iNumThread - iUsedThreads);
	}


	_uint iGlobalThreadIdx = 0;

	m_iNumEndThread.store(0);

	for (_uint iLOD = 0; iLOD < 4; ++iLOD)
	{
		_uint iThreadCntForThisLOD = iAssignedThreads[iLOD];

		_uint iTotalObjects = static_cast<_uint>(m_StaticObjects[iReadIndex][iLOD].size());
		_uint iNumObjPerThread = max(1, iTotalObjects / max(1, iThreadCntForThisLOD));

		for (_uint i = 0; i < iThreadCntForThisLOD; ++i)
		{
			if (iGlobalThreadIdx >= m_iNumThread)
				break;

			_uint iStartIndex = i * iNumObjPerThread;
			_uint iEndIndex = min((i + 1) * iNumObjPerThread, iTotalObjects);

			if (i == iThreadCntForThisLOD - 1)
				iEndIndex = iTotalObjects;

			_float fPlayTime = static_cast<_float>(m_pGameInstance->Get_PlayTime());

			if (iStartIndex < iEndIndex)
			{
				m_pGameInstance->Add_Render_Work([this, iLOD, iStartIndex, iEndIndex, iReadIndex, iGlobalThreadIdx, fPlayTime]() {
					auto pDC = m_pDeferredContext[iGlobalThreadIdx];
					pDC->ClearState();
					Setting_Viewport(pDC, static_cast<_uint>(m_fWinSizeX), static_cast<_uint>(m_fWinSizeY));
					m_pGameInstance->SetUp_MRT(pDC, TEXT("MRT_Object"));
					m_pGameInstance->Bind_SharedBuffer(iLOD, pDC);

					for (_uint iIndex = iStartIndex; iIndex < iEndIndex; ++iIndex)
					{
						m_StaticObjects[iReadIndex][iLOD][iIndex]->Set_LOD(iLOD);
						m_StaticObjects[iReadIndex][iLOD][iIndex]->Render(pDC, iGlobalThreadIdx);
						m_StaticObjects[iReadIndex][iLOD][iIndex]->Set_RenderTime(iLOD, fPlayTime);
					}


					ID3D11CommandList* pCL = nullptr;
					pDC->FinishCommandList(false, &pCL);
					Merge_CommandList(pCL, iGlobalThreadIdx);
					{
						lock_guard<mutex> lock(m_RenderAddMutex);
						m_iNumEndThread.fetch_add(1, memory_order_relaxed);
						m_CV.notify_one();
					}
					});
				iGlobalThreadIdx++;
			}
		}
	}

	if (iGlobalThreadIdx > 0)
	{
		unique_lock<mutex> lock(m_RenderAddMutex);
		m_CV.wait(lock, [&]() { return m_iNumEndThread.load() == iGlobalThreadIdx; });
	}

	for (_uint i = 0; i < iGlobalThreadIdx; ++i)
	{
		// 1차원 배열로 관리한다고 가정 (Merge_CommandList 수정 필요할 수 있음)
		if (m_CommandLists[i])
		{
			m_pContext->ExecuteCommandList(m_CommandLists[i], true);
			Safe_Release(m_CommandLists[i]);
			m_CommandLists[i] = nullptr;
		}
	}
}

void CRenderer::Clear_Resource()
{
	m_ShadowMapObjects.clear();

	for (auto& Pair : m_StaticObjects)
		for (auto& pStaticObjects : Pair)
			pStaticObjects.clear();
}

void CRenderer::Render_Priority()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"))))
		CRASH("Render Fail");

	Render_ObjectList(ENUM_CLASS(RENDERGROUP::PRIORITY));

	m_pGameInstance->End_MRT();

	m_pCurrentSceneSRV = m_pGameInstance->Get_RT_SRV(TEXT("RT_BackBuffer"));
}

void CRenderer::Render_Shadow()
{
	Setting_Viewport(g_iMaxWidth, g_iMaxHeight);

	m_pGameInstance->Begin_CSM();

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::SHADOW)])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_Shadow();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::SHADOW)].clear();

	Setting_Viewport(m_iWinSizeX, m_iWinSizeY);

	m_pGameInstance->End_CSM();
}

void CRenderer::Render_NonBlend()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Object"))))
		CRASH("Render Fail");

	Render_ObjectList(ENUM_CLASS(RENDERGROUP::NONBLEND));

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Static()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Object"), nullptr, false)))
		CRASH("Render Fail");

	// Buffer Index
	_int iReadIndex = (m_iDoubleBufferIndex + 1) % 2;

	Render_LOD_Weight();
	if (true == m_isCompleteFrustumCull.load(memory_order_acquire))
	{
		atomic_thread_fence(memory_order_acquire);
		for (auto& pObjects : m_StaticObjects[iReadIndex])
			pObjects.clear();
		m_iDoubleBufferIndex.exchange(iReadIndex, memory_order_release);

		vector<CStaticObject*> m_Temp;
		for (auto& pObjects : m_StaticObjects[(m_iDoubleBufferIndex + 1) % 2])
			m_Temp.insert(m_Temp.end(), pObjects.begin(), pObjects.end());
		
		m_pGameInstance->Occlusion_Culling(m_Temp);
		m_iCullStack.exchange(0, memory_order_release);
		m_isCompleteFrustumCull.exchange(false, memory_order_release);
	}

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Decal()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_DECAL"), nullptr, false)))
		CRASH("Failed Begin MRT_Decal");

	m_pGameInstance->Render_Decal();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_SSAO()
{
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		CRASH("Failed Bind WorldMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed Bind ViewMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed Bind ProjMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW))))
		CRASH("Failed Bind ViewMatrixInv");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::PROJ))))
		CRASH("Failed Bind ProjMatrixInv");
	if (FAILED(m_pShader->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4))))
		CRASH("Failed Bind CamPosition");

	if (false == m_IsSSAO)
	{
		m_pGameInstance->Clear_RT(TEXT("RT_SSAO"));
		m_pGameInstance->Clear_RCS(TEXT("RCS_SSAO_BLUR_X"));
		m_pGameInstance->Clear_RCS(TEXT("RCS_SSAO_BLUR_Y"));
		return;
	}

	if (!m_iCurTime)
		return;

	if (FAILED(m_pGameInstance->Render_SFX(SFX_TYPE::SSAO, m_pVIBuffer, m_pShader)))
		return;

}

void CRenderer::Render_Outline_NonCompare()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_OUTLINE_NONCOMPARE"), nullptr, false)))
		CRASH("Render Fail");


	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::OUTLINE_NONCOMPARE)])
	{
		if (nullptr != pRenderObject)
		{
			if (m_IsOutLine)
				pRenderObject->Render_OutLine();
		}

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::OUTLINE_NONCOMPARE)].clear();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Dynamic()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Object"), nullptr, false)))
		CRASH("Render Fail");

	Render_ObjectList(ENUM_CLASS(RENDERGROUP::DYNAMIC));

	m_pGameInstance->End_MRT();
}


void CRenderer::Render_Light()
{

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Light"))))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Bind_Value("Debug_IsLight", &m_IsLight, sizeof(_bool))))
		CRASH("Failed Debug Light");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_PBR"), m_pShader, "g_PBRTexture")))
		CRASH("Failed Bind RT_PBR");
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Diffuse"), m_pShader, "g_DiffuseTexture")))
		CRASH("Failed Bind RT_Diffuse");
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Normal"), m_pShader, "g_NormalTexture")))
		CRASH("Failed Bind RT_Normal");
	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Depth"), m_pShader, "g_DepthTexture")))
		CRASH("Failed Bind RT_Depth");
	if(FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_SSS"), m_pShader, "g_SkinMaskTexture")))
		CRASH("Failed Bind RT_SSS");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInv", m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW))))
		CRASH("Failed Bind ViewMatrixInv");

	//Toon Ramp Texture
	if (FAILED(m_pSubResource->Bind_Ramp_Texture(m_pShader, "g_RampTexture", 0)))
		return;

	if (FAILED(m_pGameInstance->Bind_CSM_SRV(m_pShader, "g_Cascade")))
		CRASH("Failed Bind_CSM_SRV");

	if (FAILED(m_pGameInstance->Bind_CSM_Resources(m_pShader, "g_ShadowViewMatrix", "g_ShadowProjMatrix", "g_vShadowLightDirection")))
		CRASH("Failed Bind CSM Resource");

	if (FAILED(m_pShader->Bind_Value("g_vCamPosition", m_pGameInstance->Get_CamPos(), sizeof(_float4))))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_ShadowMap_Resources_Renderer(m_pShader)))
		return;

	if (FAILED(m_pGameInstance->Bind_ShadowDistance_Resource(m_pShader, "g_vClipDistances", "g_fLastDistance")))
		return;

	if (FAILED(m_pGameInstance->Bind_LightDatas(m_pShader)))
		return;

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::LIGHT));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_SSS()
{
	if (false == m_IsSSS)
		return;

	if (FAILED(m_pGameInstance->Render_SFX(SFX_TYPE::SSS, m_pVIBuffer, m_pShader)))
		return;
}

void CRenderer::Render_Combined()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Render Fail");

	ID3D11ShaderResourceView* pDiffuse = m_IsSSS ? m_pGameInstance->Get_RCS_SRV(TEXT("RCS_SSSBlur_Y")) : m_pGameInstance->Get_RT_SRV(TEXT("RT_LightDiffuse"));

	if (FAILED(m_pShader->Bind_Texture("g_LightDiffuseTexture", pDiffuse)))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_LightSpecular"), m_pShader, "g_LightSpecularTexture")))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_LightAmbient"), m_pShader, "g_LightAmbientTexture")))
		CRASH("Render Fail");

	if(FAILED(m_pGameInstance->Bind_RendererCS(TEXT("RCS_SSAO_BLUR_Y"), m_pShader, "g_SsaoTexture")))
		CRASH("Failed Bind_SsaoTexture");

	if (FAILED(m_pSubResource->Bind_Ramp_Texture(m_pShader, "g_ColorRampTexture", 2)))
		return;

	if (FAILED(m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::COMBINED))))
		CRASH("Render Fail")

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	m_pCurrentSceneSRV = m_pGameInstance->Get_RT_SRV(TEXT("RT_BackBuffer"));
}

void CRenderer::Render_Water()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Water"), nullptr, false)))
		CRASH("Render Fail");

	Render_ObjectList(ENUM_CLASS(RENDERGROUP::WATER));

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_SSR()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Combine"))))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Render_SFX(SFX_TYPE::WATER, m_pVIBuffer, m_pShader)))
		return;

	m_pGameInstance->End_MRT();

	m_pCurrentSceneSRV = m_pGameInstance->Get_RT_SRV(TEXT("RT_Combine"));
}

void CRenderer::Render_Outline()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_OUTLINE"), nullptr, false)))
		CRASH("Failed Begin MRT");

	for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::OUTLINE)])
	{
		if (nullptr != pRenderObject)
		{
			if (m_IsOutLine)
				pRenderObject->Render_OutLine();
		}
		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(RENDERGROUP::OUTLINE)].clear();

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_NonLight()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_NonLight"), nullptr, false)))
		CRASH("Render Fail");

	Render_ObjectList(ENUM_CLASS(RENDERGROUP::NONLIGHT));

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_LUT()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Lut"))))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Bind_Texture("g_BackBufferTexture", m_pCurrentSceneSRV)))
		CRASH("Failed Bind CurrentScene");
	
	if (FAILED(m_pSubResource->Bind_LUT_Texture(m_pShader, m_iLUT_Index)))
		return;

	if (FAILED(m_pShader->Bind_Value("g_fExposure", &m_fExposure, sizeof(_float))))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Bind_Value("g_fLutLerpIntensity", &m_fLutLerpIntensity, sizeof(_float))))
		CRASH("Failed to Bind LutIntensity");

	if(FAILED(m_pShader->Bind_Value("g_IsDynamicLUT", &m_IsDynamicLUT, sizeof(_bool))))
		CRASH("Failed to Bind IsDynamicLUT");

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::LUT));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	m_pCurrentSceneSRV = m_pGameInstance->Get_RT_SRV(TEXT("RT_Lut"));
}

void CRenderer::Render_Fog()
{	
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Failed Begin MRT_BackBuffer");

	if (false == m_IsFog)
	{
		if(FAILED(m_pShader->Bind_Texture("g_Texture", m_pCurrentSceneSRV)))
			CRASH("Failed Bind CurrentScene");
		
		m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DRAW));
	}
	else
	{
		if (FAILED(m_pShader->Bind_Texture("g_BackBufferTexture", m_pCurrentSceneSRV)))
			CRASH("Failed Bind CurrentScene");

		m_pGameInstance->Bind_VF_Resource(m_pShader, "g_VoulmetricTexture", "g_vFogRange");

		m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::FOG));
	}

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	m_pCurrentSceneSRV = m_pGameInstance->Get_RT_SRV(TEXT("RT_BackBuffer"));
}

void CRenderer::Render_Effect()
{
	m_pGameInstance->Clear_RT(TEXT("RT_AccumColor"));
	m_pGameInstance->Clear_RT(TEXT("RT_AccumAlpha"));

	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_EFFECT"), nullptr, false)))
		CRASH("Render Fail");

	Render_ObjectList(ENUM_CLASS(RENDERGROUP::EFFECT));

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_EffectResolve()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_AccumColor"), m_pShader, "g_AccumColorTexture")))
		CRASH("Render Fail")

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_AccumAlpha"), m_pShader, "g_AccumAlphaTexture")))
		CRASH("Render Fail")

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::WEIGHTBLEND));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	m_pCurrentSceneSRV = m_pGameInstance->Get_RT_SRV(TEXT("RT_BackBuffer"));
}

void CRenderer::Render_SFX()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_EFFECT"), nullptr, false)))
		CRASH("Failed Begin MRT_SFX");

	Render_ObjectList(ENUM_CLASS(RENDERGROUP::SFX));

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Emissive()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Emissive"), nullptr, false)))
		CRASH("Render Fail");

	Render_ObjectList(ENUM_CLASS(RENDERGROUP::EMISSIVE));

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Bloom()
{
	if (!m_iCurTime)
		return;

	if (FAILED(m_pGameInstance->Render_SFX(SFX_TYPE::BLOOM, m_pVIBuffer, m_pShader)))
		return;
}

void CRenderer::Render_BloomCombined()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Emissive"), m_pShader, "g_BlurTexture")))
		CRASH("Render Fail");

	if (FAILED(m_pGameInstance->Bind_RendererCS(TEXT("RCS_UPSAMPLE_BLOOM"), m_pShader, "g_BloomTexture", 0)))
		CRASH("Failed Bind_RendererCS");

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::BLOOM));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	m_pCurrentSceneSRV = m_pGameInstance->Get_RT_SRV(TEXT("RT_BackBuffer"));
}

void CRenderer::Render_DistortionObject()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Distortion"), nullptr, false)))
		CRASH("Render Fail");

	Render_ObjectList(ENUM_CLASS(RENDERGROUP::DISTORTION));

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Blend()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_BackBuffer"), nullptr, false)))
		CRASH("Render Fail");

	Render_ObjectList(ENUM_CLASS(RENDERGROUP::BLEND));

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Distortion()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Combine"))))
		CRASH("Render Fail");

	if (FAILED(m_pShader->Bind_Texture("g_BackBufferTexture", m_pCurrentSceneSRV)))
		CRASH("Failed Bind CurrentScene");

	if (FAILED(m_pGameInstance->Bind_RenderTarget(TEXT("RT_Distortion"), m_pShader, "g_DistortionTexture")))
		CRASH("Render Fail");

	m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DISTORTION));

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();

	m_pGameInstance->End_MRT();

	m_pCurrentSceneSRV = m_pGameInstance->Get_RT_SRV(TEXT("RT_Combine"));
}
 
void CRenderer::Render_ScreenEffect()
{
	if(SUCCEEDED(m_pGameInstance->Render_SFX_Toggle(m_pVIBuffer, m_pShader)))
		m_pCurrentSceneSRV = m_pGameInstance->Get_RT_SRV(TEXT("RT_BackBuffer"));
		
	if (m_RenderObjects[ENUM_CLASS(RENDERGROUP::POST_SFX)].empty())		// 없을 시 그냥 Draw
	{
		//Combined
		if (FAILED(m_pShader->Bind_Texture("g_Texture", m_pCurrentSceneSRV)))
			CRASH("Failed RT_BackBuffer");

		m_pShader->Begin(ENUM_CLASS(SHADER_DEFFERED::DRAW));

		m_pVIBuffer->Bind_Resources();
		m_pVIBuffer->Render();
	}
	else
	{
		Render_ObjectList(ENUM_CLASS(RENDERGROUP::POST_SFX));			// 반 드 시 CurrentSceneSRV 받아서 그릴것..!
	}
}

void CRenderer::Render_UI()
{
	Render_ObjectList(ENUM_CLASS(RENDERGROUP::UI));
}

void CRenderer::Render_UI_Post()
{
	Render_ObjectList(ENUM_CLASS(RENDERGROUP::UI_POST));
}

void CRenderer::Render_Fade()
{
	Render_ObjectList(ENUM_CLASS(RENDERGROUP::FADE));
}

void CRenderer::Render_NonStatic()
{
	if (FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Object"), nullptr, false)))
		CRASH("Failed to Begin MRT_Object");

	m_pGameInstance->Bind_SharedBuffer(0, m_pContext);
	Render_ObjectList(ENUM_CLASS(RENDERGROUP::NONSTATIC));

	m_pGameInstance->End_MRT();
}

void CRenderer::Render_Setting()
{
//	if (m_pGameInstance->Get_DIKeyState(DIK_HOME) == KEYSTATE::PRESS)
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD4) == KEYSTATE::DOWN)
			m_IsSSAO = !m_IsSSAO;

		if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD5) == KEYSTATE::DOWN)
			m_IsFog = !m_IsFog;

		if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD6) == KEYSTATE::DOWN)
			m_IsOutLine = !m_IsOutLine;

		if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD7) == KEYSTATE::DOWN)
			m_IsLight = !m_IsLight;
	}

	if (m_pGameInstance->Get_DIKeyState(DIK_PGDN) == KEYSTATE::DOWN)
		m_isRenderDebug = !m_isRenderDebug;

	ImGui::Begin("SHADER_BOOL");

	ImGui::Checkbox("SSAO", &m_IsSSAO);
	ImGui::Checkbox("FOG", &m_IsFog);
	ImGui::Checkbox("OUTLINE", &m_IsOutLine);
	ImGui::Checkbox("LIGHT", &m_IsLight);

	ImGui::ColorPicker4("BACK_BUFFER", (&m_vBackBufferColor.x));

	ImGui::End();

#ifdef _DEBUG
	m_pGameInstance->Change_BackBufferColor(TEXT("RT_BackBuffer"), m_vBackBufferColor);
#endif

	if (false == m_isRenderDebug)
		return;

	if (FAILED(m_pGameInstance->Render_RT()))
		CRASH("Render RT");

	if (FAILED(m_pGameInstance->Debug_Render_RCS()))
		CRASH("Render RCS");

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("ViewMatrix");
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("ProjMatrix");



	m_pGameInstance->Render_CSM(m_pShader, m_pVIBuffer);

	m_pGameInstance->Render_ShadowMap(m_pShader, m_pVIBuffer);
}

#ifdef _DEBUG
void CRenderer::Render_Debug()
{
	for (auto& pComponent : m_DebugComponents)
	{
		if (nullptr != pComponent)
			pComponent->Render();
		Safe_Release(pComponent);
	}
	m_DebugComponents.clear();

	{   
		if(FAILED(m_pGameInstance->Begin_MRT(TEXT("MRT_Debug"))))
			CRASH("MRT_Debug");

		for (auto& pRenderObject : m_RenderObjects[ENUM_CLASS(RENDERGROUP::RD_DEBUG)])
		{
			if (nullptr != pRenderObject)
				pRenderObject->Render();

			Safe_Release(pRenderObject);
		}

		m_RenderObjects[ENUM_CLASS(RENDERGROUP::RD_DEBUG)].clear();

		m_pGameInstance->End_MRT();
	}


}
#endif


void CRenderer::Render_ObjectList(_uint iRG_Index)
{
	for (auto& pRenderObject : m_RenderObjects[iRG_Index])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}

	m_RenderObjects[ENUM_CLASS(iRG_Index)].clear();
}

HRESULT CRenderer::Ready_RT()
{
	/* RenderTarget Diffuse */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Diffuse"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(1.f, 0.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Normal */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Normal"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Depth */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Depth"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Metaliic */
	if(FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_PBR"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Back_Buffer */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_BackBuffer"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Emissive*/
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Emissive"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Lut */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Lut"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget SSAO */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_SSAO"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(1.f, 1.f, 1.f, 1.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Distortion */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Distortion"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT , _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Dof */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Dof"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget VelocityMap */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_VelocityMap"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget Combine */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Combine"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget SFX */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_SFX"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget SSS */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_SSS"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R32G32B32A32_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget LightDiffuse */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_LightDiffuse"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget LightSpecular */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_LightSpecular"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* RenderTarget LightAmbient */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_LightAmbient"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* Test AccumColor */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_AccumColor"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

	/* Test AccumAlpha */
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_AccumAlpha"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R16G16B16A16_FLOAT, _float4(0.f, 0.f, 0.f, 0.f))))
		ASSERT_CRASH(false);

#ifdef _DEBUG
	/* RenderTarget Debug */
	if(FAILED(m_pGameInstance->Add_RenderTarget(TEXT("RT_Debug"), m_iWinSizeX, m_iWinSizeY, DXGI_FORMAT_R8G8B8A8_UNORM, _float4(1.f, 1.f, 1.f, 0.f))))
		ASSERT_CRASH(false);
#endif
	return S_OK;
}

HRESULT CRenderer::Ready_MRT()
{
#pragma region MRT_OBJECT    
	if(FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_Diffuse"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_Normal"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_Depth"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_Emissive"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_Distortion"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_PBR"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Object"), TEXT("RT_SSS"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_WATER
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Water"), TEXT("RT_Diffuse"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Water"), TEXT("RT_Normal"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Water"), TEXT("RT_Depth"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Water"), TEXT("RT_PBR"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_LIGHT
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Light"), TEXT("RT_LightDiffuse"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Light"), TEXT("RT_LightSpecular"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Light"), TEXT("RT_LightAmbient"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_SSAO
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_SSAO"), TEXT("RT_SSAO"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_DECAL
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_DECAL"), TEXT("RT_Diffuse"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_DECAL"), TEXT("RT_Normal"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_DECAL"), TEXT("RT_Emissive"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_EFFECT
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EFFECT"), TEXT("RT_BackBuffer"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EFFECT"), TEXT("RT_Emissive"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EFFECT"), TEXT("RT_Distortion"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EFFECT"), TEXT("RT_AccumColor"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_EFFECT"), TEXT("RT_AccumAlpha"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_OUTLINE_NONCOMPARE
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_OUTLINE_NONCOMPARE"), TEXT("RT_BackBuffer"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_OUTLINE_NONCOMPARE"), TEXT("RT_Depth"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_OUTLINE_NONCOMPARE"), TEXT("RT_PBR"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_OUTLINE
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_OUTLINE"), TEXT("RT_Combine"))))
		ASSERT_CRASH(false);
	if(FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_OUTLINE"), TEXT("RT_Depth"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_OUTLINE"), TEXT("RT_PBR"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_LUT
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Lut"), TEXT("RT_Lut"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_BACKBUFFER
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_BackBuffer"), TEXT("RT_BackBuffer"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_COMBINE
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Combine"), TEXT("RT_Combine"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_NONLIGHT
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_NonLight"), TEXT("RT_Combine"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_NonLight"), TEXT("RT_Normal"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_NonLight"), TEXT("RT_Depth"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_NonLight"), TEXT("RT_PBR"))))
		ASSERT_CRASH(false);
#pragma endregion

	// RENDERGROUP::EMISSIVE
#pragma region MRT_EMISSIVE
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Emissive"), TEXT("RT_BackBuffer"))))
		ASSERT_CRASH(false);
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Emissive"), TEXT("RT_Emissive"))))
		ASSERT_CRASH(false);
#pragma endregion
	// RENDERGROUP::DISTORTION
#pragma region MRT_DISTORTION
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Distortion"), TEXT("RT_Distortion"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_DOF
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_DOF"), TEXT("RT_Dof"))))
		ASSERT_CRASH(false);
#pragma endregion

#pragma region MRT_VELOCITY_MAP
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_VELOCITY_MAP"), TEXT("RT_VelocityMap"))))
		ASSERT_CRASH(false);
#pragma endregion

#ifdef _DEBUG
#pragma region MRT_DEBUG
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Debug"), TEXT("RT_Debug"))))
		ASSERT_CRASH(false);
#pragma endregion
#endif

	return S_OK;
}

HRESULT CRenderer::Ready_RCS()
{

	return S_OK;
}

HRESULT CRenderer::Ready_Shadow_DSV()
{
	D3D11_TEXTURE2D_DESC TextureDesc = {};
	TextureDesc.Width = g_iMaxWidth;
	TextureDesc.Height = g_iMaxHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	ID3D11Texture2D* pTexture2D = { nullptr };
	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pTexture2D)))
		CRASH("Shadow DSV Texture");

	Safe_Release(pTexture2D);

	return S_OK;
}

HRESULT CRenderer::Ready_DC()
{
	m_pDeferredContext = new ID3D11DeviceContext*[m_iNumThread];

	_uint iNumViewPort = {};
	D3D11_VIEWPORT ViewPort = {};
	m_pContext->RSGetViewports(&iNumViewPort, &ViewPort);

	for (_uint i = 0; i < m_iNumThread; ++i)
		m_pDevice->CreateDeferredContext(0, &m_pDeferredContext[i]);
	
	return S_OK;
}

HRESULT CRenderer::Ready_SubResource()
{
	m_pSubResource = CRendererSubResource::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pSubResource);

	return S_OK;
}

CRenderer* CRenderer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumThread)
{
	CRenderer* pInstance = new CRenderer(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iNumThread)))
	{
		MSG_BOX("Failed to Created : Renderer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRenderer::Free()
{
	__super::Free();

#ifdef _DEBUG
	for (auto& pComponent : m_DebugComponents)
		Safe_Release(pComponent);
	m_DebugComponents.clear();
#endif

	for (_uint i = 0; i < m_iNumThread; ++i)
	{
		m_pDeferredContext[i]->ClearState();
		m_pDeferredContext[i]->Flush();
		Safe_Release(m_pDeferredContext[i]);
	}
	Safe_Delete_Array(m_pDeferredContext);

	for (auto& Pair : m_Effects)
	{
		for (size_t i = 0; i < Pair.second.size(); ++i)
			Safe_Release(Pair.second[i]);
		Pair.second.clear();
	}
	m_Effects.clear();

	for (size_t i = 0; i < ENUM_CLASS(RENDERGROUP::END); i++)
	{
		for (auto& pRenderObject : m_RenderObjects[i])
			Safe_Release(pRenderObject);

		m_RenderObjects[i].clear();
	}

	m_ShadowMapObjects.clear();

	for (auto& pCL : m_CommandLists)
		Safe_Release(pCL);
	m_CommandLists.clear();

	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pSubResource);
	
	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
}
