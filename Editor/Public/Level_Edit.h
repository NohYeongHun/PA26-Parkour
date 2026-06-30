#pragma once
#include "Level.h"

NS_BEGIN(Editor)

class CLevel_Edit final : public CLevel
{
private:
	explicit CLevel_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Edit() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;

private:
	// Test
	class CModelLoader*	m_pLoader = { nullptr };

public:
	static		CLevel_Edit*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END