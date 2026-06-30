#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	explicit CVIBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer(const CVIBuffer& Prototype);
	virtual ~CVIBuffer() = default;

public:
	virtual HRESULT		Initialize_Prototype();
	virtual HRESULT		Initialize_Clone(void* pArg);
	virtual HRESULT		Render();
	virtual HRESULT		Render(ID3D11DeviceContext* pDC);

	virtual HRESULT		Bind_Resources();
	virtual HRESULT		Bind_Resources(ID3D11DeviceContext* pDC);

protected:
	ID3D11Buffer*					m_pVB = { nullptr };
	ID3D11Buffer*					m_pIB = { nullptr };

	_uint							m_iNumVertices = {};
	_uint							m_iVertexStride = {};
	_uint							m_iNumVertexBuffers = {};

	_uint							m_iNumIndices = {};
	_uint							m_iIndexStride = {};
	DXGI_FORMAT						m_eIndexFormat = {};

	D3D11_PRIMITIVE_TOPOLOGY m_ePrimitiveType = {};

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END