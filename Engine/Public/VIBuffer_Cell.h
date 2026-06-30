#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Cell final : public CVIBuffer
{
private:
	explicit CVIBuffer_Cell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Cell(const CVIBuffer_Cell& Prototype);
	virtual ~CVIBuffer_Cell() = default;

public:
	HRESULT							Initialize(const _float3* pPoints);

public:
	static		CVIBuffer_Cell*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float3* pPoints);
	virtual		CComponent*		Clone(void* pArg);
	virtual		void					Free() override;
};

NS_END