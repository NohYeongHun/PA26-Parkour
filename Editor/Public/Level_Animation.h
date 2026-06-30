#pragma once
#include "Level.h"

NS_BEGIN(Editor)

class CLevel_Animation final : public CLevel
{
private:
	explicit CLevel_Animation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Animation() = default;

public:
	virtual HRESULT		Initialize() override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Render() override;

private:
	class CAnimationTool* m_pAnimationTool = { nullptr };
	LEVEL m_eCurLevel = { LEVEL::ANIMATION };

public:
	static		CLevel_Animation*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};

NS_END