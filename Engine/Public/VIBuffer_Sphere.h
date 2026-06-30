#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Sphere final : public CVIBuffer
{
private:
	explicit CVIBuffer_Sphere(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Sphere(const CVIBuffer_Sphere& Prototype);
	virtual ~CVIBuffer_Sphere() = default;

public:
	virtual     HRESULT             Initialize_Prototype() override;
	virtual     HRESULT             Initialize_Clone(void* pArg) override;

public:
	static      CVIBuffer_Sphere* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual     CComponent* Clone(void* pArg) override;
	virtual     void                Free() override;
};

NS_END