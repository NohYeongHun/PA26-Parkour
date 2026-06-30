#pragma once
#include "GameObject.h"

NS_BEGIN(Editor)

class CSQ_Item_Edit abstract : public CGameObject
{
protected:
	explicit CSQ_Item_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSQ_Item_Edit(const CSQ_Item_Edit& Prototype);
	virtual ~CSQ_Item_Edit() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;
	virtual		void				Render_Shadow() override;
	virtual		void				Render_OutLine() override;

	// Pooling Spawn CallBack
	virtual		void				Reset(const _fmatrix& WorldMatrix, void* pArg) {}

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void					Free() override;
};

NS_END