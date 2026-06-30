#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)
class ENGINE_DLL CVIBuffer_Instance abstract : public CVIBuffer
{
public:
	typedef struct tagInstanceDesc {
		_uint iNumInstance;
		_float3 vCenter;
		_float3 vRange;
		_float2 vSize;
	}INSTANCE_DESC;

protected:
	explicit CVIBuffer_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Instance(const CVIBuffer_Instance& Prototype);
	virtual ~CVIBuffer_Instance() = default;

public:
	virtual HRESULT		Initialize_Prototype();
	virtual HRESULT		Initialize_Prototype(const INSTANCE_DESC* pDesc);
	virtual HRESULT		Initialize_Clone(void* pArg) override;
	virtual HRESULT		Render() override;
	virtual HRESULT		Render(ID3D11DeviceContext* pDC)override;

	virtual HRESULT		Bind_Resources() override;
	virtual HRESULT		Bind_Resources(ID3D11DeviceContext* pDC)override;

protected:
	ID3D11Buffer* m_pVBInstance = { nullptr };
	_uint m_iNumInstance = {};
	_uint m_iNumIndexPerInstance = {};
	_uint m_iInstanceVertexStride = {};

protected:
	D3D11_BUFFER_DESC m_VBInstanceDesc = {};
	void* m_pVBInstanceVertices = { nullptr };

public:
	static CVIBuffer_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	//virtual CComponent* Clone(void* pArg)override;
	virtual void Free()override;

};

NS_END