#pragma once
#include "VIBuffer_Decal.h"

NS_BEGIN(Engine)

class CVIBuffer_Decal_Cube final : public CVIBuffer_Decal
{
private:
	explicit CVIBuffer_Decal_Cube(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Decal_Cube(const CVIBuffer_Decal_Cube& Prototype) = delete;
	virtual ~CVIBuffer_Decal_Cube() = default;

public:
	virtual HRESULT Initialize() override;

public:
	static CVIBuffer_Decal_Cube*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*				Clone(void* pArg) { return nullptr; }
	virtual void					Free() override;
};

NS_END