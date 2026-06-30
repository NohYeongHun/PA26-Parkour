#pragma once
#include "GameObject.h"
NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Editor)
class CEdit_PreViewModel : public CGameObject
{
	enum VIEWTYPE { X, Y, Z };
private:
	CEdit_PreViewModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEdit_PreViewModel(const CEdit_PreViewModel& Prototype);
	virtual ~CEdit_PreViewModel() = default;

public:
	virtual		HRESULT			Initialize_Prototype(_uint iLevel);
	virtual		void			Priority_Update(_float fTimeDelta);
	virtual		void			Update(_float fTimeDelta);
	void			Late_Update(_float fTimeDelta, _wstring ModelName);
	virtual		void			Render();

	void						Add_Model(_wstring ModelName);
private:
	unordered_map<_wstring, CModel*> m_Models;
	CShader* m_pShaderCom = { nullptr };

	_wstring ProtoModelName = TEXT("Prototype_Component_Model_");
	_wstring m_szModelName;

	_float m_fViewTime = {};
	VIEWTYPE m_eViewType = {X};
	_uint m_iLevel = {};
public:
	static CEdit_PreViewModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iLevel = ENUM_CLASS(LEVEL::MAP));
	virtual CGameObject* Clone(void* pArg)override { return nullptr; }
	virtual void Free()override;
};

NS_END