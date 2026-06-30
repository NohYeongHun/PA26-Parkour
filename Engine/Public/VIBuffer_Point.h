#pragma once
#include "VIBuffer.h"
NS_BEGIN(Engine)
class ENGINE_DLL CVIBuffer_Point final: public CVIBuffer
{
private:
	explicit CVIBuffer_Point(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Point(const CVIBuffer_Point& Prototype);
	virtual ~CVIBuffer_Point() = default;

public:
	virtual		HRESULT				Initialize_Prototype() override;
	virtual		HRESULT				Initialize_Clone(void* pArg) override;

	virtual HRESULT		Render()override;

	virtual HRESULT		Bind_Resources()override;
public:
	static		CVIBuffer_Point* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CComponent* Clone(void* pArg)override;
	virtual		void					Free() override;
};

NS_END