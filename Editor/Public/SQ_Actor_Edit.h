#pragma once
#include "SQ_Item_Edit.h"

NS_BEGIN(Editor)

class CSQ_Actor_Edit final : public CSQ_Item_Edit
{
private:
	explicit CSQ_Actor_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CSQ_Actor_Edit(const CSQ_Actor_Edit& Prototype);
	virtual ~CSQ_Actor_Edit() = default;

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
	virtual		void				Reset(const _fmatrix& WorldMatrix, void* pArg) override;

public:
	static CSQ_Actor_Edit*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void					Free() override;
};

NS_END