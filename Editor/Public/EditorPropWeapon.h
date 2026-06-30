#pragma once
#include "EditorProp.h"

NS_BEGIN(Editor)
class CEditorPropWeapon final : public CEditorProp
{
public:
	typedef struct tagEditorPropWeaponDesc : public CEditorProp::PROP_DESC {

	} EDITOR_PROP_WEAPON_DESC;

protected:
	explicit CEditorPropWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEditorPropWeapon(const CPartObject& Prototype);
	virtual ~CEditorPropWeapon() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;

#ifdef _DEBUG
public:
	const vector<_string>& Get_AnimationNames() const;
	_float* Get_TrackPositionPtr(const _string& strAnimName);
	_float Get_Duration(const _string& strAnimName);
	const _string& Get_CurrentAnimationNames() const { return m_strCurrentAnimName; }
	const _float Get_CurrentAnimationDuration() const;
	void Set_TrackPosition(_float fTrackPosition);
	void Set_PlayAnimation(_bool IsPlay) { m_IsPlayAnimation = IsPlay; }
	void Change_CurrentAnimation(_string strAnimName) { m_strCurrentAnimName = strAnimName; }
#endif

private:
	_uint m_iShaderPath = {};
	_bool m_IsPlayAnimation = { true };
	_float m_fTimeDelta = {};

private:
	void Bind_Resources();
	void Ready_Components(const PROP_DESC* pDesc);
	void Ready_Variables(const PROP_DESC* pDesc);
	void Ready_Positions(const PROP_DESC* pDesc);

public:
	static CEditorPropWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		CGameObject* Clone(void* pArg) override;
	virtual		void Free() override;

};
NS_END
