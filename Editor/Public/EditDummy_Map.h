#pragma once
#include "EditDummy.h"

NS_BEGIN(Engine)
class CModel_Streaming;
class CShader;
class CRigidbody;
NS_END

NS_BEGIN(Editor)

class CEditDummy_Map final : public CEditDummy
{
public:
	typedef struct tagDuumyMapDesc : public DUMMY_DESC
	{
		_matrix PreTransformMatrix;
	}DUMMY_MAP_DESC;

private:
	explicit CEditDummy_Map(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEditDummy_Map(const CEditDummy_Map& Prototype);
	virtual ~CEditDummy_Map() = default;

public:
	virtual		HRESULT			Initialize_Prototype() override;
	virtual		HRESULT			Initialize_Clone(void* pArg) override;
	virtual		void				Priority_Update(_float fTimeDelta) override;
	virtual		void				Update(_float fTimeDelta) override;
	virtual		void				Late_Update(_float fTimeDelta) override;
	virtual		void				Render() override;
	virtual		void				Render_Shadow() override;

private:
	CModel_Streaming*					m_pModelCom = { nullptr };
	CShader*						m_pShaderCom = { nullptr };
	CRigidbody*				m_pRigidbodyCom = { nullptr };

private:
	HRESULT						Ready_Component(_fmatrix PreTransformMatrix);

public:
	static		CEditDummy_Map* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void				Free() override;
};

NS_END