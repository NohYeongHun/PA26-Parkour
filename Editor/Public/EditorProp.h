#pragma once
#include "Editor_Define.h"
#include "PartObject.h"

NS_BEGIN(Editor)
class CEditorProp abstract : public CPartObject
{
public:
	typedef struct tagPropDesc : public CPartObject::PART_DESC 
	{
		const _float4x4* pSocketMatrix = { nullptr };
		pair<LEVEL, _wstring> shaderData = {};
		pair<LEVEL, _wstring> modelData = {};
		_string strBoneName = {};
		_string strFolderPath = {};
		WEAPONTYPE eWeaponType = { WEAPONTYPE::END };
		_float3 vPosition = {};
		_float3 vScale = {};
		_float3 vRotation = {};
	} PROP_DESC;

protected:
	explicit CEditorProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CEditorProp(const CPartObject& Prototype);
	virtual ~CEditorProp() = default;

public:
	virtual	HRESULT	Initialize_Prototype() override;
	virtual	HRESULT	Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;

public:
	virtual void Activate(_bool IsActive);

public:
	_bool Is_AnimationEnd() { return m_IsAnimationEnd; }
	virtual void Play_Animation(const _string& strAnimName, _float fTimeDelta, _float* pTrackPosition, _float fRootMotionRate = 1.f, _bool IsRootMotion = true, _bool IsRootMotionRotate = true, _bool IsRootMotionTranslate = true, _bool IsLoop = false); 
	void Set_SocketMatrix(const _float4x4* pSocketMatrix) { m_pSocketMatrix = pSocketMatrix; }
	void Clear_Animation(const _string& strAnimName);

#pragma region NOTIFY
public:
	virtual void Collider_Active(const _wstring& wStrColliderTag, _bool IsActive) ;
	virtual void Effect_Active(const _wstring& wStrEffectTag);
	virtual void Object_Func(const _wstring& wStrObjectTag) {}; // 임시
	void Collider_Active(_bool isActive);
#pragma endregion


protected:
	class CComputeShader* m_pComputeShaderCom = { nullptr };
	class CShader* m_pShaderCom = { nullptr };
	class CModel* m_pModelCom = { nullptr };
	class CAnimator* m_pAnimator = { nullptr };   // lazy-bound pose evaluator
	class CRigidbody* m_pRigidbodyCom = { nullptr };
	class CTransform* m_pParentTransform = { nullptr };
	WEAPONTYPE m_eWeaponType = { WEAPONTYPE::END };
	
	_float3 m_vRotationOffset = {};

	const _float4x4* m_pSocketMatrix = { nullptr };
	_float m_fTrackPosition = {};
	_bool m_IsAnimationEnd = { false };
	_string m_strCurrentAnimName = {};

protected:
	void Register_AllNotifies(const _string& strFolderPath);

public:
	static CEditorProp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	CGameObject* Clone(void* pArg) = 0;
	virtual	void Free() override;
};
NS_END

