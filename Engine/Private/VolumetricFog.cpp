#include "EnginePch.h"
#include "VolumetricFog.h"
#include "Gameinstance.h"
#include "ComputeShader.h"

CVolumetricFog::CVolumetricFog(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
	, m_pGameInstance { CGameInstance::GetInstance()}
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

void CVolumetricFog::Set_FogFarRatioToCameraFar(_float fFogFarRatio)
{
	m_fFogFarRatioToCamera = fFogFarRatio;

	m_vFogRange.y = m_VF_Data.fCamFar * m_fFogFarRatioToCamera;

	m_VF_Data.fFar = m_vFogRange.y;
}

HRESULT CVolumetricFog::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_fFogFarRatioToCamera = 0.3f;

	m_vFroxelSize.x = iWinSizeX >> 3; // 3 = DownSacle Factor
	m_vFroxelSize.y = iWinSizeY >> 3;
	//m_vFroxelSize.z = 192;				// 64~128
	m_vFroxelSize.z = 128;				// 64~128

	m_vDefinition.x = 8;
	m_vDefinition.y = 8;
	m_vDefinition.z = 8;

	m_iMaxLight = 256;

	m_VF_Data.fWinSizeX = static_cast<_float>(iWinSizeX);
	m_VF_Data.fWinSizeY = static_cast<_float>(iWinSizeY);

	m_VF_Data.vFroxelSize = m_vFroxelSize;
	m_VF_Data.iSliceCount = m_vFroxelSize.z;
	m_VF_Data.fLightIntensity = 1.f;
	m_VF_Data.fDensity = 1.f;
	m_VF_Data.fPhaseFunctionG = 0.5f;
	m_VF_Data.fDensityScale = 0.01f;

	m_VF_Data.fFogMaxDistance = 100.f;
	m_VF_Data.fFogMaxHeight = 300.f;

	//m_VF_Data.vFogColor = _float3(1.f, 1.f, 1.f);
	m_VF_Data.vFogColor = _float3(0.7f, 0.75f, 0.87f);
	m_VF_Data.fHegihtFallOff = 0.01f;
	m_VF_Data.fGroundFallOff = 0.02f;
	//m_VF_Data.fDistanceFallOff = 0.02f;
	m_VF_Data.fDistanceFallOff = 0.001f;
	m_VF_Data.fNoiseScale = 0.002f;

	m_VF_Data.fRayPhaseFunctionG = 0.5f;
	m_VF_Data.fRayIntensity = 3.f;

	m_VF_Data.fScatterWeight = 0.5f;

	m_VF_Data.fFogBaseIntensity = 0.2f;

	m_VF_Data.fRayDensity = 0.7f;
	m_VF_Data.fRayDensityScale = 0.f;

	m_IsFirst = true;

	if (FAILED(Ready_FroxelVolume()))
		return E_FAIL;

	if (FAILED(Ready_ComputeShader()))
		return E_FAIL;

	if (FAILED(Ready_Sampler()))
		return E_FAIL;

	if (FAILED(Ready_NoiseTexture()))
		return E_FAIL;

	Make_NoiseTexture();

    return S_OK;
}

HRESULT CVolumetricFog::SetUp_FogNF()
{
	m_VF_Data.fCamNear = m_pGameInstance->Get_CurrentCamera_Near();
	m_VF_Data.fCamFar = m_pGameInstance->Get_CurrentCamera_Far();
	
	m_vFogRange.x = m_VF_Data.fCamNear;
	m_vFogRange.y = m_VF_Data.fCamFar * m_fFogFarRatioToCamera;

	m_VF_Data.fNear = m_vFogRange.x;
	m_VF_Data.fFar = m_vFogRange.y;
	
	return S_OK;
}

void CVolumetricFog::Begin_VF()
{
	m_IsUpdate = true;
}

void CVolumetricFog::Clear()
{
	m_IsUpdate = false;
	m_IsFirst = true;
	m_VF_Data.IsTemporal = false;
}

void CVolumetricFog::Update_VF(_float fTimeDelta)
{
	if (false == m_IsUpdate)
		return;

	Update_Buffer(fTimeDelta);

	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Set_ConstantBuffer("VF_Data", m_pBuffers[ENUM_CLASS(BUFFER::DATA)]);
	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Set_ConstantBuffer("ShadowMap_Data", m_pGameInstance->Get_ShadowMapDownSampleBuffer());

	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Set_SRV("g_LightDatas", m_pSRVs[ENUM_CLASS(SRV::LIGHT)]);
	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Set_SRV("g_MipDepthTexture", m_pGameInstance->Get_HZB_Resource());
	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Set_SRV("g_ShadowMapTexture", m_pGameInstance->Get_ShadowMapDownSampleSRV());
	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Set_SRV("g_NoiseTexture", m_pSRVs[ENUM_CLASS(SRV::VF_NOISE)]);
	
	if(m_VF_Data.IsTemporal)
		m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Set_SRV("PrevVFLightTexture", m_pSRVs[m_iReadIndex]);

	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Set_UAV("OutputTexture", m_pUAVs[m_iWriteIndex]);

	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Set_Sampler(0, m_pDefaultSampler);
	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Set_Sampler(1, m_pShadowSampler);

	_uint iThreadGroupCountX = static_cast<_uint>((m_vFroxelSize.x + m_vDefinition.x - 1) / m_vDefinition.x);
	_uint iThreadGroupCountY = static_cast<_uint>((m_vFroxelSize.y + m_vDefinition.y - 1) / m_vDefinition.y);
	_uint iThreadGroupCountZ = static_cast<_uint>((m_vFroxelSize.z + m_vDefinition.z - 1) / m_vDefinition.z);

	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]->Dispatch(iThreadGroupCountX, iThreadGroupCountY, iThreadGroupCountZ);

	m_pCS[ENUM_CLASS(CS::VF_BEER)]->Set_ConstantBuffer("VF_Data", m_pBuffers[ENUM_CLASS(BUFFER::DATA)]);
	m_pCS[ENUM_CLASS(CS::VF_BEER)]->Set_SRV("VFLightTexture", m_pSRVs[m_iWriteIndex]);
	m_pCS[ENUM_CLASS(CS::VF_BEER)]->Set_UAV("OutputTexture", m_pUAVs[ENUM_CLASS(UAV::VF_BEER)]);
	
	m_pCS[ENUM_CLASS(CS::VF_BEER)]->Dispatch(iThreadGroupCountX, iThreadGroupCountY, 1);

	swap(m_iWriteIndex, m_iReadIndex);
}

HRESULT CVolumetricFog::Bind_VF_Resource(CShader* pShader, const _char* pTextureName, const _char* pFogRangeName)
{
	ASSERT_CRASH(pShader);
	if (FAILED(pShader->Bind_Texture(pTextureName, m_pSRVs[ENUM_CLASS(SRV::VF_BEER)])))
		CRASH("Failed to Bind Texture Beer");

	if (FAILED(pShader->Bind_Value(pFogRangeName, &m_vFogRange, sizeof(_float2))))
		CRASH("Failed to Bind Value FogFar");

	return S_OK;
}

//#ifdef _DEBUG
void CVolumetricFog::Setting_VF()
{
	//ImGui::Begin("VolumetricFog");

	//ImGui::InputFloat("Fog_Near", &m_VF_Data.fNear, 1.f, 10.f);
	//ImGui::InputFloat("Fog_Far", &m_VF_Data.fFar, 1.f, 10.f);
	//ImGui::InputFloat("LightIntensity", &m_VF_Data.fLightIntensity, 1.f, 10.f);
	//ImGui::DragFloat("Density", &m_VF_Data.fDensity, 0.01f, 0.f, 1.f, "%.2f");
	//ImGui::DragFloat("DENSITY_SCALE", &m_VF_Data.fDensityScale, 0.01f, 0.01f, 1.f, "%.2f");
	//ImGui::DragFloat("PHASE_FUNCTION", &m_VF_Data.fPhaseFunctionG, 0.01f, -0.5f, 0.5f, "%.2f");

	//ImGui::DragFloat("DISTANCE_FALLOFF", &m_VF_Data.fDistanceFallOff, 0.01f, 0.01f, 1.f, "%.2f");
	//ImGui::DragFloat("HEIGHT_FALLOFF", &m_VF_Data.fHegihtFallOff, 0.01f, 0.01f, 1.f, "%.2f");
	//ImGui::DragFloat("GROUND_FALLOFF", &m_VF_Data.fGroundFallOff, 0.01f, 0.01f, 1.f, "%.2f");
	//ImGui::DragFloat("NOISE_SCALE", &m_VF_Data.fNoiseScale, 0.0001f, 0.00001f, 0.001f, "%.5f");

	//ImGui::DragFloat("RAY_PHASE_FUNCTION", &m_VF_Data.fRayPhaseFunctionG, 0.01f, 0.5f, 1.f, "%.2f");
	//ImGui::DragFloat("RAY_INTENSITY", &m_VF_Data.fRayIntensity, 0.1f, 2.f, 10.f, "%.5f");

	//ImGui::DragFloat("SCATTER_WEIGHT", &m_VF_Data.fScatterWeight, 0.01f, 0.1f, 1.f, "%.5f");

	//ImGui::DragFloat("RAY_DENSITY", &m_VF_Data.fRayDensity, 0.01f, 0.1f, 1.f, "%.5f");

	//ImGui::DragFloat("RAY_DENSITY_SCALE", &m_VF_Data.fRayDensityScale, 0.01f, 0.05f, 0.5f, "%.5f");

	//ImGui::End();
}
//#endif

void CVolumetricFog::Update_Buffer(_float fTimeDelta)
{
//#ifdef _DEBUG
	Setting_VF();
//#endif

	m_VF_Data.PrevViewMatrix = *m_pGameInstance->Get_PrevTransformState_Float4x4(D3DTS::VIEW);
	m_VF_Data.PrevProjMatrix = *m_pGameInstance->Get_PrevTransformState_Float4x4(D3DTS::PROJ);
	m_VF_Data.InvViewMatrix = *m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::VIEW);
	m_VF_Data.InvProjMatrix = *m_pGameInstance->Get_TransformState_Float4x4_Inv(D3DTS::PROJ);
	m_VF_Data.fNoiseTimeDelta = fmodf((m_VF_Data.fNoiseTimeDelta + (fTimeDelta * 0.05f)), 1.f);
	m_VF_Data.vCamPos = *m_pGameInstance->Get_CamPos();
	m_VF_Data.iRandCount = (m_VF_Data.iRandCount + 1) % 16;
	m_VF_Data.IsTemporal = m_IsFirst ? false : true;

	if (m_IsFirst)
		m_IsFirst = false;

	//LIGHT_DATA UPDATE
	const vector<LIGHT_DATA>* pLightDats = m_pGameInstance->Get_LightDatas();
	ASSERT_CRASH(pLightDats);

	_uint iLightCount = static_cast<_uint>((*pLightDats).size());

	m_VF_Data.iLightCount = iLightCount;

	if(iLightCount > 0)
	{
		D3D11_BOX Box = { 0, 0, 0, sizeof(LIGHT_DATA) * iLightCount, 1, 1 };
		m_pContext->UpdateSubresource(m_pBuffers[ENUM_CLASS(BUFFER::LIGHT)], 0, &Box, (*pLightDats).data(), 0, 0);
	}

	//VF_DATA UPDATE
	D3D11_MAPPED_SUBRESOURCE VF_SubResource = {};
	m_pContext->Map(m_pBuffers[ENUM_CLASS(BUFFER::DATA)], 0, D3D11_MAP_WRITE_DISCARD, 0, &VF_SubResource);
	memcpy(VF_SubResource.pData, &m_VF_Data, sizeof(VF_DATA));
	m_pContext->Unmap(m_pBuffers[ENUM_CLASS(BUFFER::DATA)], 0);

}

HRESULT CVolumetricFog::Ready_FroxelVolume()
{
	if (FAILED(Ready_FogTexture(ENUM_CLASS(UAV::VF_LIGHT_FIRST))))
		return E_FAIL;

	if (FAILED(Ready_FogTexture(ENUM_CLASS(UAV::VF_LIGHT_SECOND))))
		return E_FAIL;

	if (FAILED(Ready_FogTexture(ENUM_CLASS(UAV::VF_BEER))))
		return E_FAIL;

	if (FAILED(Ready_Buffer()))
		return E_FAIL;
	
	m_iWriteIndex = ENUM_CLASS(UAV::VF_LIGHT_FIRST);
	m_iReadIndex = ENUM_CLASS(UAV::VF_LIGHT_SECOND);

    return S_OK;
}

HRESULT CVolumetricFog::Ready_FogTexture(_uint iTextureIndex)
{
	if (iTextureIndex >= ENUM_CLASS(UAV::END))
		return E_FAIL;

	//Texture
	D3D11_TEXTURE3D_DESC TextureDesc = {};
	TextureDesc.Width = m_vFroxelSize.x;
	TextureDesc.Height = m_vFroxelSize.y;
	TextureDesc.Depth = m_vFroxelSize.z;
	TextureDesc.MipLevels = 1;
	TextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	ID3D11Texture3D* pTexture = { nullptr };
	if (FAILED(m_pDevice->CreateTexture3D(&TextureDesc, nullptr, &pTexture)))
		CRASH("Failed to Created Texture3D");

	//UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.Format = TextureDesc.Format;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	UAVDesc.Texture3D.FirstWSlice = 0;
	UAVDesc.Texture3D.MipSlice = 0;
	UAVDesc.Texture3D.WSize = TextureDesc.Depth;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(pTexture, &UAVDesc, &m_pUAVs[iTextureIndex])))
		CRASH("Failed to Created FV_UAV");

	//SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = TextureDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	SRVDesc.Texture3D.MostDetailedMip = 0;
	SRVDesc.Texture3D.MipLevels = 1;

	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, &SRVDesc, &m_pSRVs[iTextureIndex])))
		CRASH("Failed to Created FV_SRV");

	Safe_Release(pTexture);

	return S_OK;
}

HRESULT CVolumetricFog::Ready_NoiseTexture()
{
	m_vNoiseSize = _float3(64.f, 64.f, 64.f);

	D3D11_TEXTURE3D_DESC TextureDesc = {};
	TextureDesc.Width = m_vNoiseSize.x;
	TextureDesc.Height = m_vNoiseSize.y;
	TextureDesc.Depth = m_vNoiseSize.z;
	TextureDesc.MipLevels = 1;
	TextureDesc.Format = DXGI_FORMAT_R8_UNORM;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	ID3D11Texture3D* pTexture = { nullptr };
	if (FAILED(m_pDevice->CreateTexture3D(&TextureDesc, nullptr, &pTexture)))
		CRASH("Failed to Created Texture3D");

	//UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
	UAVDesc.Format = TextureDesc.Format;
	UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	UAVDesc.Texture3D.FirstWSlice = 0;
	UAVDesc.Texture3D.MipSlice = 0;
	UAVDesc.Texture3D.WSize = TextureDesc.Depth;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(pTexture, &UAVDesc, &m_pUAVs[ENUM_CLASS(UAV::VF_NOISE)])))
		CRASH("Failed to Created FV_UAV");

	//SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = TextureDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	SRVDesc.Texture3D.MostDetailedMip = 0;
	SRVDesc.Texture3D.MipLevels = 1;

	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, &SRVDesc, &m_pSRVs[ENUM_CLASS(UAV::VF_NOISE)])))
		CRASH("Failed to Created FV_SRV");

	Safe_Release(pTexture);

	return S_OK;
}

HRESULT CVolumetricFog::Ready_Buffer()
{
	//CONSTANT BUFFER
	D3D11_BUFFER_DESC CBufferDesc = {};
	CBufferDesc.ByteWidth = sizeof(VF_DATA);
	CBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	CBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	CBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(m_pDevice->CreateBuffer(&CBufferDesc, nullptr, &m_pBuffers[ENUM_CLASS(BUFFER::DATA)])))
		CRASH("Failed to Created VF_CBuffer");

	//CONSTANT BUFFER SETTING
	VF_DATA Data = {};
	ZeroMemory(&Data, sizeof(VF_DATA));

	D3D11_MAPPED_SUBRESOURCE SubResource;
	m_pContext->Map(m_pBuffers[ENUM_CLASS(BUFFER::DATA)], 0, D3D11_MAP_WRITE_DISCARD, 0, &SubResource);
	memcpy(SubResource.pData, reinterpret_cast<void*>(&Data), sizeof(VF_DATA));
	m_pContext->Unmap(m_pBuffers[ENUM_CLASS(BUFFER::DATA)], 0);

	//STRUCTURED BUFFER
	D3D11_BUFFER_DESC StructureBufferDesc = {};
	StructureBufferDesc.ByteWidth = sizeof(LIGHT_DATA) * m_iMaxLight;
	StructureBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	StructureBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	StructureBufferDesc.StructureByteStride = sizeof(LIGHT_DATA);
	StructureBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	StructureBufferDesc.CPUAccessFlags = 0;

	if (FAILED(m_pDevice->CreateBuffer(&StructureBufferDesc, nullptr, &m_pBuffers[ENUM_CLASS(BUFFER::LIGHT)])))
		CRASH("Failed to Created VF_LightBuffer");

	//SBUFFER SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC SBuffer_SRVDesc = {};
	SBuffer_SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SBuffer_SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SBuffer_SRVDesc.Buffer.FirstElement = 0;
	SBuffer_SRVDesc.Buffer.NumElements = m_iMaxLight;
	
	if(FAILED(m_pDevice->CreateShaderResourceView(m_pBuffers[ENUM_CLASS(BUFFER::LIGHT)], &SBuffer_SRVDesc, &m_pSRVs[ENUM_CLASS(SRV::LIGHT)])))
		CRASH("Failed to Created Buffer_SRV");

	return S_OK;
}

HRESULT CVolumetricFog::Ready_ComputeShader()
{
	SHADER_MACRO Macro = { { "THREAD_X", "8" }, { "THREAD_Y", "8" }, { "LIGHT_THREAD_Z", "8" }, { NULL, NULL } };

	m_pCS[ENUM_CLASS(CS::VF_LIGHT)]= CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_VF.hlsl"), Macro, "ComputeLight");
	ASSERT_CRASH(m_pCS);

	Macro = { { "THREAD_X", "8" }, { "THREAD_Y", "8" }, { "BEER_THREAD_Z", "1" }, { NULL, NULL } };

	m_pCS[ENUM_CLASS(CS::VF_BEER)] = CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_VF.hlsl"), Macro, "VolumetricFog");
	ASSERT_CRASH(m_pCS);

	Macro = { { "THREAD_X", "8" }, { "THREAD_Y", "8" }, { "THREAD_Z", "8" }, { NULL, NULL } };

	m_pCS[ENUM_CLASS(CS::VF_NOISE)] = CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_Noise.hlsl"), Macro, "SimpleXNoise");
	ASSERT_CRASH(m_pCS);

	return S_OK;
}

HRESULT CVolumetricFog::Ready_Sampler()
{
	D3D11_SAMPLER_DESC DefaultSamplerDesc = {};
	DefaultSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	DefaultSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	DefaultSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	DefaultSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	DefaultSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	DefaultSamplerDesc.MinLOD = 0;
	DefaultSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	m_pDevice->CreateSamplerState(&DefaultSamplerDesc, &m_pDefaultSampler);
	ASSERT_CRASH(m_pDefaultSampler);

	D3D11_SAMPLER_DESC ShadowSamplerDesc = {};
	ShadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	ShadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ShadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ShadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ShadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	ShadowSamplerDesc.MinLOD = 0;
	ShadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	m_pDevice->CreateSamplerState(&ShadowSamplerDesc, &m_pShadowSampler);
	ASSERT_CRASH(m_pShadowSampler);

	return S_OK;
}

void CVolumetricFog::Make_NoiseTexture()
{
	m_pCS[ENUM_CLASS(CS::VF_NOISE)]->Set_UAV("OutputTexture", m_pUAVs[ENUM_CLASS(UAV::VF_NOISE)]);

	_uint iThreadGroupCountX = static_cast<_uint>((m_vNoiseSize.x + m_vDefinition.x - 1) / m_vDefinition.x);
	_uint iThreadGroupCountY = static_cast<_uint>((m_vNoiseSize.y + m_vDefinition.y - 1) / m_vDefinition.y);
	_uint iThreadGroupCountZ = static_cast<_uint>((m_vNoiseSize.z + m_vDefinition.z - 1) / m_vDefinition.z);

	m_pCS[ENUM_CLASS(CS::VF_NOISE)]->Dispatch(iThreadGroupCountX, iThreadGroupCountY, iThreadGroupCountZ);
}

CVolumetricFog* CVolumetricFog::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, uint iWinSizeX, _uint iWinSizeY)
{
	CVolumetricFog* pInstance = new CVolumetricFog(pDevice, pContext);
	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
	{
		MSG_BOX("Failed to Created : CVolumetricFog");
		Safe_Release(pInstance);
	}
    return pInstance;
}

void CVolumetricFog::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);

	for (_uint i = 0; i < ENUM_CLASS(CS::END); ++i)
		Safe_Release(m_pCS[i]);
	
	for (_uint i = 0; i < ENUM_CLASS(UAV::END); ++i)
		Safe_Release(m_pUAVs[i]);
	
	for(_uint i=0; i< ENUM_CLASS(SRV::END); ++i)
		Safe_Release(m_pSRVs[i]);

	for (_uint j = 0; j < ENUM_CLASS(BUFFER::END); ++j)
		Safe_Release(m_pBuffers[j]);

	Safe_Release(m_pDefaultSampler);
	Safe_Release(m_pShadowSampler);
}
