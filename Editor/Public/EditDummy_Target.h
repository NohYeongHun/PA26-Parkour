#pragma once
#include "EditDummy.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CCollider;
NS_END

NS_BEGIN(Editor)

class CEditDummy_Target final : public CEditDummy
{
public:
	typedef struct tagDummyTargetDesc : public DUMMY_DESC
	{
		_matrix PreTransformMatrix;
	}DUMMY_TARGET_DESC;

private:
	explicit CEditDummy_Target(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEditDummy_Target(const CEditDummy_Target& Prototype);
	virtual ~CEditDummy_Target() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;
	virtual		void				Render_Shadow() override;

private:
	CModel*						m_pModelCom = { nullptr };
	CShader*						m_pShaderCom = { nullptr };
	CCollider*					m_pColliderCom = { nullptr };

private:
	HRESULT						Ready_Component(_fmatrix PreTransformMatrix);

public:
	static		CEditDummy_Target* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void				Free() override;
};

NS_END