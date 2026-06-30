#pragma once
#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CEffect : public CGameObject
{
protected:
	explicit CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEffect(const CEffect& Prototype);
	virtual ~CEffect() = default;

public:
	virtual		HRESULT				Initialize_Prototype() override;
	virtual		HRESULT				Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;


public:
	virtual							CGameObject* Clone(void* pArg) = 0;
	virtual void					Free() override;
};

NS_END
