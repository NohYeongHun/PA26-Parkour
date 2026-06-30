#pragma once
#include "Interface_Edit.h"

NS_BEGIN(Editor)

class CCamera_Interface final : public CInterface_Edit
{
private:
	explicit CCamera_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCamera_Interface() = default;

public:
	virtual		HRESULT					Initialize() override;

public:


public:
	static		CCamera_Interface*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void						Free() override;
};

NS_END