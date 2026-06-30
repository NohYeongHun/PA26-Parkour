#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Rect_Instance final : public CVIBuffer
{
private:
	explicit CVIBuffer_Rect_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Rect_Instance(const CVIBuffer_Rect_Instance& Prototype);
	virtual ~CVIBuffer_Rect_Instance() = default;

public:
	virtual HRESULT		Initialize_Prototype(_uint iMaxInstance);
	virtual HRESULT		Initialize_Clone(void* pArg) override;
	virtual HRESULT		Render() override;
	virtual HRESULT		Bind_Resources() override;

	HRESULT				Update_Buffer(const vector<VTXINSTANCE_RECT>& Datas);
	void				Clear();

private:
	ID3D11Buffer*		m_pVBInstance = { nullptr };

	D3D11_BUFFER_DESC	m_VBInstanceDesc = {};
	_uint				m_iMaxInstance = {};
	_uint				m_iNumInstance = {};
	_uint				m_iNumIndexPerInstance = {};
	_uint				m_iInstanceVertexStride = {};

public:
	static CVIBuffer_Rect_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iMaxInstance);
	virtual CComponent*				Clone(void* pArg) override;
	virtual void					Free()override;
};

NS_END