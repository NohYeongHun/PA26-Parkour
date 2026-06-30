#pragma once
#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Rect final : public CVIBuffer
{
private:
	explicit CVIBuffer_Rect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CVIBuffer_Rect(const CVIBuffer_Rect& Prototype);
	virtual ~CVIBuffer_Rect() = default;

public:
	virtual		HRESULT				Initialize_Prototype() override;
	virtual		HRESULT				Initialize_Clone(void* pArg) override;

public:
	static		CVIBuffer_Rect*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CComponent*		Clone(void* pArg);
	virtual		void					Free() override;
};

NS_END