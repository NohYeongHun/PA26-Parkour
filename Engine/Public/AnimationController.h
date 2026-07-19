#pragma once
#include "Component.h"
#include "State.h"
#include "StateMachine.h"

NS_BEGIN(Engine)

// 상태 독립적인 애니메이션 재생 컴포넌트
class ENGINE_DLL CAnimationController final : public CComponent
{
public:
	using FnResolveKey = function<_bool(const _string&, StateKey&)>;

	typedef struct tagAnimControllerDesc : CComponent::COMPONENT_DESC
	{
	}ANIM_CONTROLLER_DESC;

protected:
	explicit CAnimationController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAnimationController(const CAnimationController& Prototype);
	virtual ~CAnimationController() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize_Clone(void* pArg) override;

public:
	// Bind_Parameter* 는 Load 이전에 반드시 호출해야 함 — Parse 시 룩업.
	void Bind_Parameter(const _string& strName, const _float* pValue);
	void Bind_Parameter2D(const _string& strName, const _float2* pValue);

	HRESULT Load(const _string& strFilePath, const FnResolveKey& fnResolveKey);
	void    Reload();

	// 재생할 상태+애님 ID 지정 — 같은 프레임의 Tick이 실제 재생.
	void    Request(const StateKey& Key, _uint iAnimId);
	void    Tick(_float fTimeDelta);

	_bool   IsAnimEnd()         const { return m_isAnimEnd; }
	_float  Get_TrackPosition() const { return m_fTrackPosition; }
	_uint   Get_CurrentAnimId() const { return m_iCurrentAnimId; }
	const CState::ANIM_DATA* Get_CurrentAnimData() const;

	class CAnimator* Get_Animator() const { return m_pAnimator; }

private:
	HRESULT Parse(const _string& strFilePath, const FnResolveKey& fnResolveKey,
	              map<StateKey, map<_uint, CState::ANIM_DATA>>& OutRegistry,
	              _string& strOutError);

private:
	_string      m_strFilePath;
	FnResolveKey m_fnResolveKey;

	map<StateKey, map<_uint, CState::ANIM_DATA>> m_Registry;
	map<_string, const _float*>  m_Params1D;
	map<_string, const _float2*> m_Params2D;

	StateKey m_CurrentKey    { 0, 0 };
	_uint    m_iCurrentAnimId = UINT_MAX;
	_float   m_fTrackPosition = 0.f;
	_bool    m_isAnimEnd      = false;

	class CModel*     m_pModelCom     = nullptr;
	class CTransform* m_pTransformCom = nullptr;
	class CAnimator*  m_pAnimator     = nullptr;

	_uint m_iVersion = 1;

public:
	static CAnimationController* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END
