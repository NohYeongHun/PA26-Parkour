#pragma once
#include "EditDummy.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Editor)

class CEditDummy_Wolf final : public CEditDummy
{
public:
	typedef struct tagDuumyWolfDesc : public DUMMY_DESC
	{
		_matrix PreTransformMatrix;
	}DUMMY_WOLF_DESC;

private:
	CEditDummy_Wolf(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEditDummy_Wolf(const CEditDummy_Wolf& Prototype);
	virtual ~CEditDummy_Wolf() = default;

public:
	virtual		HRESULT		Initialize_Prototype() override;
	virtual		HRESULT		Initialize_Clone(void* pArg) override;
	virtual		void		Priority_Update(_float fTimeDelta) override;
	virtual		void		Update(_float fTimeDelta) override;
	virtual		void		Late_Update(_float fTimeDelta) override;
	virtual		void		Render() override;
	virtual		void		Render_Shadow() override;

private:
	CModel*				m_pModelCom = { nullptr };
	CShader*			m_pShaderCom = { nullptr };

	class CSpringCamera_Edit* m_pSpringCamera = { nullptr };

private:
	void					Key_Move(_float fTimeDelta);

private:
	HRESULT				Ready_Camera();
	HRESULT				Ready_Components(_fmatrix PreTransformMatrix);

public:
	static CEditDummy_Wolf* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;
};

NS_END