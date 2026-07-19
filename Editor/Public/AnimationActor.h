#pragma once

#include "EditorProp.h"
#include "ContainerObject.h"
NS_BEGIN(Editor)
class CAnimationActor final : public CContainerObject
{
public:
	typedef struct tagAnimationActorDesc : CContainerObject::GAMEOBJECT_DESC
	{
		LEVEL eLevel = {};
		_wstring strShaderTag = {};
		_wstring strComputeShaderTag = {};
		_wstring strMorphComputeShaderTag = {};
		_wstring strModelTag = {};
		_string strModelDatPath = {};

		_uint iShaderPath = {};

		_float3 vPostion = {};
		_float3 vRotation = {};
		_float3 vScale = {};

		class CTransform* pParentTransform = { nullptr };
		_string strBoneName = { };
		CAnimationActor* pParentActor = { nullptr };
		_bool IsFacial = { false };
	}ANIMATION_ACTOR_DESC;



private:
	explicit CAnimationActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAnimationActor(const CAnimationActor& Prototype);
	virtual ~CAnimationActor() = default;

public:
	virtual	HRESULT Initialize_Prototype() override;
	virtual	HRESULT Initialize_Clone(void* pArg) override;
	virtual	void Priority_Update(_float fTimeDelta) override;
	virtual	void Update(_float fTimeDelta) override;
	virtual	void Late_Update(_float fTimeDelta) override;
	virtual	void Render() override;
	virtual void Render_Shadow() override;

	

#ifdef _DEBUG
public:
	const vector<_string>& Get_AnimationNames() const;
	_float* Get_TrackPositionPtr(const _string& strAnimName);
	_float	Get_Duration(const _string& strAnimName);

	const _string& Get_CurrentAnimationNames() const;
	const _float Get_CurrentAnimationDuration() const;

	//Bone GUI 연결
	HRESULT Bind_Bone_to_GUI();

	void Change_CurrentAnimation(_string strAnimName);
	// 루트모션 궤적: 애니 시작 트랜스폼 앵커로 현재 애니 궤적을 그린다.
	void Debug_DrawRootMotionTrajectory(_float fTimeStepSec, const ROOTMOTION_DESC& rootDesc);
	void Set_TrackPosition(_float fTrackPosition);
	void Set_PlayAnimation(_bool IsPlay);

	void Register_AllNotifies(const _string& strFolderPath);


	
	void Collider_Active(const _wstring& tag, _bool IsActive);
	void Effect_Active(const _wstring& tag);
	void Object_Func(const _wstring& tag);

	class CModel* Get_ModelCom() { return m_pModelCom; }
	const _float4x4* Get_BoneMatrix(const _string& strBoneName);
	const _float4x4* Get_WorldMatrixPtr();

	void Set_ChildActor(CAnimationActor* pChildActor);
	void Set_ChildActors(CAnimationActor* pChildActor);
	//_bool Is_ChildActor() { return m_pChildActor != nullptr; }
	_bool Is_ChildActor();
	class CTransform* Get_Transform() { return m_pTransformCom; }

	
	void Child_Render();
	void Render_Detail();
	void Render_Detail(_int iIndex);

	void Print_WorldMatrix();

	void Set_AnimationSpeed(_float fSpeed) { m_fAnimationSpeed = fSpeed; }

	void Change_BoneMatrixPtr(const _string& strBoneName);
	
#endif // _DEBUG


private:
	LEVEL m_eCurLevel = {LEVEL::END};
	class CModel* m_pModelCom = { nullptr };
	class CAnimator* m_pAnimator = { nullptr };   // lazy-bound pose evaluator (CPU path)
	class CShader* m_pShaderCom = { nullptr };
	class CComputeShader* m_pComputeShaderCom = { nullptr };
	class CComputeShader* m_pMorphComputeShaderCom = { nullptr };

	class CTransform* m_pParentTransform = { nullptr };
	const _float4x4* m_pSocketMatrix = { nullptr };
	CAnimationActor* m_pParentActor = { nullptr };

	vector<CAnimationActor*> m_ChildActors = { nullptr };
	CAnimationActor* m_pChildActor = { nullptr };
	_float4x4 m_CombinedMatrix = {};

	_float4x4 m_StartMatrix = {};
	_float4 m_vInitPosition = {};

	_uint m_iShaderPath = {};
	_string m_strCurrentAnimation = {};
	_string m_strCurrentRibAnimation = {};
	_float m_fTrackPosition = {};
	_bool m_IsPlayAnimation = { true };

	_float m_fTimeDelta = {};

	_string m_strModelDatPath = {}; //

	// Camera
	class CSpringCamera_Edit* m_pSpringCamera = { nullptr };
	_float	m_fOffsetY = {};


	_float m_fAnimationSpeed = { 1.f };
	_bool m_IsGPU = { true };

	// Facial
	
	_bool m_IsFacial = { false };
	

#ifdef _DEBUG
	// PartObject
	_float4x4 m_TrajectoryAnchor = {}; // 애니 변경 시점의 월드 트랜스폼 스냅샷
#endif

private:
	void Bind_Resources();
	HRESULT Ready_Components(const ANIMATION_ACTOR_DESC* pDesc);
	HRESULT Ready_Camera();

	void Render_Facial();
	void Render_Default();

public:
	virtual	CGameObject* Clone(void* pArg) override;
	static CAnimationActor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual	void Free() override;
};
NS_END

