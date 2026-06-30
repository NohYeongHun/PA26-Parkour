#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class CComputeShader;

class CRendererCS : public CBase
{
public:
	typedef struct tagRendererCSDesc {
		const _tchar* pFilePath;
		SHADER_MACRO eShaderMacro;
		_string strEntryPoint;
		_float fDefinitionX;
		_float fDefinitionY;
		_uint iWidth;
		_uint iHeight;
		DXGI_FORMAT eFormat;
		_uint iMipLevels;
		_float4 vClearColor;
	}RCS_DESC;

private:
	typedef pair<_uint, ID3D11Buffer*> BUFFER_DATA;
	typedef map<const _string, BUFFER_DATA> BUFFERS;
	typedef pair<_string, ID3D11ShaderResourceView*> SRV_DATA;
	typedef pair<_string, vector<ID3D11UnorderedAccessView*>> UAV_DATA;

private:
	explicit CRendererCS(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRendererCS() = default;

public:
	void							Add_BufferData(const _char* pConstantName, void* pData, _uint iLength);
	void							Add_SRVData(const _char* pConstantName, ID3D11ShaderResourceView* pSRV);
	void							Add_SamplerState(_uint iSlotIndex, ID3D11SamplerState* pSampler);
	void							Setting_UAV_Data(const _char* pConstantName) { m_UAV.first = pConstantName;}

	ID3D11ShaderResourceView*		Get_SRV(_uint iMipLevel);

public:
	HRESULT							Initialize(void* pDesc);

	void							Bind_Resources(_uint iMipLevel);
	void							Dispatch(_uint iWidth, _uint iHeight);
	void							Clear(_uint iMipLevel);
	void							Clear_Resource();

	HRESULT							Debug_Render(const _wstring& strRCS_Name);

private:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };
	CComputeShader*						m_pComputeShader = { nullptr };
	
	BUFFERS								m_Buffers;
	vector<SRV_DATA>					m_SRVs;
	map<_uint, ID3D11SamplerState*>		m_Samplers;

	_float								m_fWidht = {};
	_float								m_fHeight = {};
	_float								m_fDefinitionX = {};
	_float								m_fDefinitionY = {};

	_uint								m_iMipLevels = {};

	vector<ID3D11Texture2D*>			m_Texture2Ds;
	UAV_DATA							m_UAV;
	vector<ID3D11ShaderResourceView*>	m_ComputeSRVs;
	_float4								m_vClearColor = {};

private:
	HRESULT							Ready_BindTexture(_uint iWidth, _uint iHeight, DXGI_FORMAT eFormat);
	HRESULT							Ready_Buffer(ID3D11Buffer** ppOut, _uint iLength);
	BUFFER_DATA*					Find_Buffer(const _char* pConstantName);


public:
	static CRendererCS*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pDesc);
	virtual void					Free() override;
};

NS_END