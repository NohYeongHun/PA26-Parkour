#include "EnginePch.h"
#include "HZB.h"

#include "GameInstance.h"
#include "StaticObject.h"

CHZB::CHZB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }, m_pContext { pContext },
	m_pGameInstance { CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);
}

HRESULT CHZB::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	Ready_DefaultSetting();
	Ready_OcclusionCulling();

	m_pBoxInfos = new BOXINFO[MAX_OBJECT];
    return S_OK;
}

void CHZB::Update()
{
	_float4 vClearColor = _float4(0.f, 0.f, 0.f, 0.f);
	for (_uint i = 0; i < MAX_MIPLEVEL; ++i)
	{
		m_pContext->ClearUnorderedAccessViewFloat(m_pUAV[i], reinterpret_cast<_float*>(&vClearColor));

		ID3D11ShaderResourceView* pSRV = 0 == i ? m_pGameInstance->Get_RT_SRV(TEXT("RT_Depth")) : m_pSRV[i - 1];

		CAMERA_FAR CameraFar = {};
		CameraFar.fFar = m_pGameInstance->Get_CurrentCamera_Far();
		m_pContext->UpdateSubresource(m_pCameraFarBuffer, 0, nullptr, &CameraFar, 0, 0);
		m_pComputeShader[HZB_CS_TYPE::MIPMAP]->Set_ConstantBuffer("CameraFar", m_pCameraFarBuffer);

		0 == i ? m_pComputeShader[HZB_CS_TYPE::MIPMAP]->Set_SRV("InputTexture", pSRV) : m_pComputeShader[HZB_CS_TYPE::MIPMAP]->Set_SRV("InputMipTexture", pSRV);
	
		m_pComputeShader[HZB_CS_TYPE::MIPMAP]->Set_UAV("OutputTexture", m_pUAV[i]);

		_uint iSizeX = max(m_iWinSizeX >> (i + 1), 1);
		_uint iSizeY = max(m_iWinSizeY >> (i + 1), 1);

		_uint iThreadGroupX = (iSizeX + 7) / 8;
		_uint iThreadGroupY = (iSizeY + 7) / 8;

		m_pComputeShader[HZB_CS_TYPE::MIPMAP]->Dispatch(iThreadGroupX, iThreadGroupY, 1);
	}
}

void CHZB::Occlusion_Culling(vector<class CStaticObject*>& Objects)
{
	return;
	_uint iNumObjects = Objects.size();

	// Create StructuredBuffer (BoxPoints)

	//D3D11_MAPPED_SUBRESOURCE BoxPointsSub = {};
	//m_pContext->Map(m_pBoxPointsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &BoxPointsSub);
	//pBoxPoints = static_cast<CORNERS*>(BoxPointsSub.pData);
	_matrix ViewMatrix = m_pGameInstance->Get_TransformState_Matrix(D3DTS::VIEW);
	for (_uint i = 0; i < iNumObjects; ++i)
	{
		BOXINFO BoxInfo = {};

		_float3 vCorners[8] = {};
		Objects[i]->Get_BoundingBox()->GetCorners(vCorners);
		for (_uint j = 0; j < 8; ++j)
		{
			memcpy(&BoxInfo.vCorner[j], &vCorners[j], sizeof(_float3));
			BoxInfo.vCorner[j].w = 1.f;
			XMStoreFloat4(&BoxInfo.vCorner[j], XMVector4Transform(XMLoadFloat4(&BoxInfo.vCorner[j]), ViewMatrix));
		}
		XMStoreFloat3(&BoxInfo.vCenter, XMVector3TransformCoord(XMLoadFloat3(&Objects[i]->Get_BoundingBox()->Center), ViewMatrix));
		BoxInfo.fRadius = max(Objects[i]->Get_BoundingBox()->Extents.x, max(Objects[i]->Get_BoundingBox()->Extents.y, Objects[i]->Get_BoundingBox()->Extents.z));
		//BoxInfo.fRadius = Objects[i]->Get_BoundingBox()->Extents.z;
		m_pBoxInfos[i] = BoxInfo;
	}
	//m_pContext->Unmap(m_pBoxPointsBuffer, 0);
	m_pContext->UpdateSubresource(m_pBoxPointsBuffer, 0, nullptr, m_pBoxInfos, 0, 0);

	ZeroMemory(m_pBoxInfos, sizeof(BOXINFO) * 3000);

	// Dispatch
	OC_DESC OCDesc = {};
	OCDesc.ProjMatrix = *m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ);
	OCDesc.iNumObjects = iNumObjects;
	OCDesc.iHZBMipLevel = 3;
	OCDesc.fMip0SizeX = static_cast<_float>(m_iWinSizeX >> 1);
	OCDesc.fMip0SizeY = static_cast<_float>(m_iWinSizeY >> 1);
	m_pContext->UpdateSubresource(m_pOCDescBuffer, 0, nullptr, &OCDesc, 0, 0);
	
	m_pComputeShader[HZB_CS_TYPE::OCCLUSION]->Set_ConstantBuffer("OCDesc", m_pOCDescBuffer);
	m_pComputeShader[HZB_CS_TYPE::OCCLUSION]->Set_SRV("g_BoxPoints", m_pBoxPointsSRV);
	m_pComputeShader[HZB_CS_TYPE::OCCLUSION]->Set_SRV("InputTexture", m_pMMSRV);
	m_pComputeShader[HZB_CS_TYPE::OCCLUSION]->Set_UAV("OutputTexture", m_pOcclusionResultUAV);
	_uint iGroupX = (iNumObjects + 255) / 256;
	m_pComputeShader[HZB_CS_TYPE::OCCLUSION]->Dispatch(iGroupX, 1, 1);

	// Flag Copy
	m_pContext->CopyResource(m_pOcclusionStageBuffer[m_iWriteIndex], m_pOcclusionFlagBuffer);

	//m_pContext->Flush();
	// Update NonCull Objects
	vector<CStaticObject*> CullObjects;

	_uint* pFlags = new _uint[iNumObjects];
	ZeroMemory(pFlags, sizeof(_uint) * iNumObjects);

	D3D11_MAPPED_SUBRESOURCE SubResource = {};
	m_pContext->Map(m_pOcclusionStageBuffer[m_iReadIndex], 0, D3D11_MAP_READ, 0, &SubResource);
	memcpy(pFlags, SubResource.pData, sizeof(_uint) * iNumObjects);
	for (_uint i = 0; i < iNumObjects; ++i)
	{
		size_t ObjectAddress = reinterpret_cast<size_t>(Objects[i]);
		auto iter = m_PreVisible.find(ObjectAddress);
		if (iter == m_PreVisible.end())
			m_PreVisible.emplace(ObjectAddress, VISIBLE_COUNT());

		if (m_PreVisible[ObjectAddress].isVisible || pFlags[i] == 1 || m_PreVisible[ObjectAddress].iCount < 10) // Visible
			CullObjects.push_back(Objects[i]);
		
		m_PreVisible[ObjectAddress].isVisible = static_cast<_bool>(pFlags[i]);
		if (false == m_PreVisible[ObjectAddress].isVisible)
			++m_PreVisible[ObjectAddress].iCount;
		else
			m_PreVisible[ObjectAddress].iCount = 0;
	}
	m_pContext->Unmap(m_pOcclusionStageBuffer[m_iReadIndex], 0);

	Safe_Delete_Array(pFlags);

	// Swap Vector
	_uint iNumPre = Objects.size();
	_uint iNumCur = CullObjects.size();
	//cout << "Pre : " << iNumPre << " / Cur : " << iNumCur << endl;
	Objects.clear();
	Objects.reserve(CullObjects.size());
	Objects.swap(CullObjects);

	swap(m_iWriteIndex, m_iReadIndex);
}

void CHZB::Ready_DefaultSetting()
{
	// Create Constant Buffer
	D3D11_BUFFER_DESC CameraFarBufferDesc = {};
	CameraFarBufferDesc.ByteWidth = sizeof(CAMERA_FAR);
	CameraFarBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	CameraFarBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	CameraFarBufferDesc.CPUAccessFlags = 0;
	CameraFarBufferDesc.StructureByteStride = sizeof(CAMERA_FAR);
	CameraFarBufferDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateBuffer(&CameraFarBufferDesc, nullptr, &m_pCameraFarBuffer)))
		CRASH("CameraFar Buffer");

	// Texture2D Desc
	D3D11_TEXTURE2D_DESC TextureDesc = {};

	TextureDesc.Width = m_iWinSizeX >> 1;
	TextureDesc.Height = m_iWinSizeY >> 1;
	TextureDesc.MipLevels = MAX_MIPLEVEL;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32_FLOAT;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	// Create Texture2D
	ID3D11Texture2D* pTexture = { nullptr };

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pTexture)))
		CRASH("Texture2D");

	// Create UAVs
	for (_uint i = 0; i < MAX_MIPLEVEL; ++i)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.Format = TextureDesc.Format;
		UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		UAVDesc.Texture2D.MipSlice = i;

		if (FAILED(m_pDevice->CreateUnorderedAccessView(pTexture, &UAVDesc, &m_pUAV[i])))
			CRASH("HZB UAV");

		// Create SRV
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = TextureDesc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = i;
		SRVDesc.Texture2D.MipLevels = 1;

		if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, &SRVDesc, &m_pSRV[i])))
			CRASH("HZB SRV");
	}

	// Create MMSRV
	D3D11_SHADER_RESOURCE_VIEW_DESC MMSRVDesc = {};
	MMSRVDesc.Format = TextureDesc.Format;
	MMSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	MMSRVDesc.Texture2D.MostDetailedMip = 0;
	MMSRVDesc.Texture2D.MipLevels = MAX_MIPLEVEL;

	if (FAILED(m_pDevice->CreateShaderResourceView(pTexture, &MMSRVDesc, &m_pMMSRV)))
		CRASH("HZB MMSRV");

	Safe_Release(pTexture);

	// Create CS
	SHADER_MACRO ShaderMacro = {
		{ "THREAD_X", "8" },
		{ "THREAD_Y", "8" },
		{ "THREAD_Z", "1" },
		{ nullptr, nullptr }
	};
	m_pComputeShader[HZB_CS_TYPE::MIPMAP] = CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_HZB.hlsl"),
		ShaderMacro, "HZB");
	ASSERT_CRASH(m_pComputeShader[HZB_CS_TYPE::MIPMAP]);

}

void CHZB::Ready_OcclusionCulling()
{
	_uint iMaxObject = MAX_OBJECT;

	// Create Constant Buffer
	D3D11_BUFFER_DESC OCDescBufferDesc = {};
	OCDescBufferDesc.ByteWidth = sizeof(OC_DESC);
	OCDescBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	OCDescBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	OCDescBufferDesc.CPUAccessFlags = 0;
	OCDescBufferDesc.StructureByteStride = sizeof(OC_DESC);
	OCDescBufferDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateBuffer(&OCDescBufferDesc, nullptr, &m_pOCDescBuffer)))
		CRASH("OCDesc Buffer");

	// Create BoxPoint Buffer
	D3D11_BUFFER_DESC BoxPointsBufferDesc = {};
	BoxPointsBufferDesc.ByteWidth = sizeof(BOXINFO) * iMaxObject;
	BoxPointsBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BoxPointsBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BoxPointsBufferDesc.CPUAccessFlags = 0;
	BoxPointsBufferDesc.StructureByteStride = sizeof(BOXINFO);
	BoxPointsBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	if (FAILED(m_pDevice->CreateBuffer(&BoxPointsBufferDesc, nullptr, &m_pBoxPointsBuffer)))
		CRASH("BoxPoints Buffer");

	// Create BoxPoint SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC BoxPointsSRVDesc = {};
	BoxPointsSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	BoxPointsSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	BoxPointsSRVDesc.Buffer.FirstElement = 0;
	BoxPointsSRVDesc.Buffer.NumElements = iMaxObject;

	if (FAILED(m_pDevice->CreateShaderResourceView(m_pBoxPointsBuffer, &BoxPointsSRVDesc, &m_pBoxPointsSRV)))
		CRASH("BoxPoints SRV");

	// Create OcclusionFlag Buffer
	D3D11_BUFFER_DESC OcclusionFlagBufferDesc = {};
	OcclusionFlagBufferDesc.ByteWidth = sizeof(_uint) * iMaxObject;
	OcclusionFlagBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	OcclusionFlagBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	OcclusionFlagBufferDesc.CPUAccessFlags = 0;
	OcclusionFlagBufferDesc.StructureByteStride = sizeof(_uint);
	OcclusionFlagBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	if (FAILED(m_pDevice->CreateBuffer(&OcclusionFlagBufferDesc, nullptr, &m_pOcclusionFlagBuffer)))
		CRASH("OcclusionFlag Buffer");

	// Create OcclusionResult UAV
	D3D11_UNORDERED_ACCESS_VIEW_DESC ResultUAVDesc = {};
	ResultUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	ResultUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	ResultUAVDesc.Buffer.FirstElement = 0;
	ResultUAVDesc.Buffer.NumElements = iMaxObject;

	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pOcclusionFlagBuffer, &ResultUAVDesc, &m_pOcclusionResultUAV)))
		CRASH("Result UAV");

	// Create OcclusionStage Buffer
	D3D11_BUFFER_DESC OcclusionStageBufferDesc = {};
	OcclusionStageBufferDesc.ByteWidth = sizeof(_uint) * iMaxObject;
	OcclusionStageBufferDesc.Usage = D3D11_USAGE_STAGING;
	OcclusionStageBufferDesc.BindFlags = 0;
	OcclusionStageBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	OcclusionStageBufferDesc.StructureByteStride = sizeof(_uint);
	OcclusionStageBufferDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateBuffer(&OcclusionStageBufferDesc, nullptr, &m_pOcclusionStageBuffer[0])))
		CRASH("OcclusionStage Buffer0");

	if (FAILED(m_pDevice->CreateBuffer(&OcclusionStageBufferDesc, nullptr, &m_pOcclusionStageBuffer[1])))
		CRASH("OcclusionStage Buffer1");

	// Create CS
	SHADER_MACRO ShaderMacro = {
		{ "THREAD_X", "256" },
		{ "THREAD_Y", "1" },
		{ "THREAD_Z", "1" },
		{ nullptr, nullptr }
	};
	m_pComputeShader[HZB_CS_TYPE::OCCLUSION] = CComputeShader::Create(m_pDevice, m_pContext, TEXT("../../Engine/Bin/ShaderFiles/Engine_ComputeShader_OcclusionCulling.hlsl"),
		ShaderMacro, "Occlusion_Culling");
	ASSERT_CRASH(m_pComputeShader[HZB_CS_TYPE::OCCLUSION]);
}

#ifdef _DEBUG
void CHZB::Render()
{
	ImGui::Begin("HZB_RENDER");

	if (ImGui::BeginCombo("HZB", "List"))
	{
		for (_uint i = 0; i < MAX_MIPLEVEL; ++i)
		{
			_string strHZB = "HZB";
			strHZB += to_string(i);
			
			if (ImGui::Selectable(strHZB.c_str()))
			{
				AddRemoveHZB(strHZB, reinterpret_cast<ImTextureID>(m_pSRV[i]));
			}
		}

		ImGui::EndCombo();
	}
	ImGui::End();

	for (auto& Pair : m_RenderTextures)
	{
		ImGui::Begin(Pair.first.c_str());
		ImGui::Image(Pair.second, ImVec2(500.f, 500.f));
		ImGui::End();
	}
	
}
void CHZB::AddRemoveHZB(const _string& strHZB, ImTextureID TextureID)
{
	auto iter = m_RenderTextures.find(strHZB);
	if (iter != m_RenderTextures.end())
	{
		m_RenderTextures.erase(iter);
		return;
	}

	m_RenderTextures.emplace(strHZB, TextureID);
	
}
#endif

CHZB* CHZB::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CHZB* pInstance = new CHZB(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
		CRASH("HZB");

    return pInstance;
}

void CHZB::Free()
{
	__super::Free();

	// Default Setting
	for (_uint i = 0; i < MAX_MIPLEVEL; ++i)
		Safe_Release(m_pUAV[i]);
	for (_uint i = 0; i < MAX_MIPLEVEL; ++i)
		Safe_Release(m_pSRV[i]);
	Safe_Release(m_pMMSRV);
	for(_uint i = 0; i < HZB_CS_TYPE::END; ++i)
		Safe_Release(m_pComputeShader[i]);
	Safe_Release(m_pCameraFarBuffer);

	// Occlusion Culling
	Safe_Release(m_pOCDescBuffer);
	Safe_Release(m_pBoxPointsBuffer);
	Safe_Release(m_pBoxPointsSRV);
	Safe_Release(m_pOcclusionFlagBuffer);
	Safe_Release(m_pOcclusionResultUAV);
	Safe_Release(m_pOcclusionStageBuffer[0]);
	Safe_Release(m_pOcclusionStageBuffer[1]);

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
	Safe_Release(m_pGameInstance);
	Safe_Delete_Array(m_pBoxInfos);
}
