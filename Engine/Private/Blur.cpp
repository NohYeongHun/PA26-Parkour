#include "EnginePch.h"
#include "Blur.h"
#include "GameInstance.h"

CBlur::CBlur(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CSFX { pDevice, pContext }
{
}

HRESULT CBlur::Initialize()
{
	m_iNumWeights = 5;
	if (FAILED(Ready_BlurWeights()))
		return E_FAIL;

	return S_OK;
}

HRESULT CBlur::Add_Blur_BufferData(const _wstring& strRCSTag, _float fWidth, _float fHeight, _uint iBlurWeight)
{
	if (iBlurWeight >= m_iNumWeights)
		return E_FAIL;

	BLUR_DATA Data = {};
	Data.vSize = _float2(fWidth, fHeight);
	Data.iRadius = m_Weights[iBlurWeight].first;

	if (FAILED(m_pGameInstance->Add_BufferData(strRCSTag, "BLUR_DATA", reinterpret_cast<void*>(&Data), sizeof(BLUR_DATA))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_SRVData(strRCSTag, "g_Weights", m_WeightSRVs[iBlurWeight])))
		return E_FAIL;

	return S_OK;
}

HRESULT CBlur::Ready_BlurWeights()
{
	m_Weights.resize(m_iNumWeights);

	for (_uint i = 1; i <= m_iNumWeights; ++i)
	{
		_int iRadius = i * 3;

		_float fSum = 0.f;

		for (_int j = -iRadius; j <= iRadius; ++j)
		{
			_float fWeight = expf(-(j * j) / (2.f * i * i));
			m_Weights[i - 1].second.push_back(fWeight);
			fSum += fWeight;
		}

		for (auto& Weight : m_Weights[i - 1].second)
			Weight /= fSum;

		m_Weights[i - 1].first = iRadius;
		Create_BlurBuffer(m_Weights[i - 1].second, iRadius);
	}

	return S_OK;
}

HRESULT CBlur::Create_BlurBuffer(const vector<_float>& Weights, _uint iRadius)
{
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(_float) * (iRadius * 2 + 1);
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.StructureByteStride = sizeof(_float);
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA Data = {};
	Data.pSysMem = Weights.data();

	ID3D11Buffer* pBuffer = { nullptr };
	m_pDevice->CreateBuffer(&BufferDesc, &Data, &pBuffer);
	ASSERT_CRASH(pBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.Buffer.FirstElement = 0;
	SRVDesc.Buffer.NumElements = iRadius * 2 + 1;

	ID3D11ShaderResourceView* pSRV = { nullptr };
	m_pDevice->CreateShaderResourceView(pBuffer, &SRVDesc, &pSRV);
	ASSERT_CRASH(pSRV);

	m_WeightBuffers.push_back(pBuffer);
	m_WeightSRVs.push_back(pSRV);

	return S_OK;
}

void CBlur::Free()
{
	__super::Free();

	for (auto& pBuffer : m_WeightBuffers)
		Safe_Release(pBuffer);
	m_WeightBuffers.clear();

	for (auto& pSRV : m_WeightSRVs)
		Safe_Release(pSRV);
	m_WeightSRVs.clear();
}
