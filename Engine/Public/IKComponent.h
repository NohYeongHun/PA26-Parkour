#pragma once
#include "Component.h"


NS_BEGIN(Engine)
// IK들을 관리할 컴포넌트
class ENGINE_DLL CIKComponent final : public CComponent
{
public:
	typedef struct tagIKComponentDesc : CComponent::COMPONENT_DESC
	{
		class CModel*	  pOwnerModelCom = { nullptr };
		class CTransform* pOwnerTransformCom = { nullptr };
	}IKCOMPONENT_DESC;

private:
	explicit CIKComponent(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CIKComponent(const CIKComponent& Prototype);
	virtual ~CIKComponent() = default;

public:
	void Update_ForwardKinematics(_uint iRootIdx);

public:
	void Begin_Target(const _string& strTarget, EIKTARGET_MODE eMode, _float fPosWeight, _float fRotWeight, _float fBlendSec);
	void End_Target(const _string& strTarget, _float fBlendSec);
	// 외부 구동 알파(트랙 포지션 등 연속 값) — 시간 블렌드를 우회하고 즉시 반영
	void Set_TargetAlpha(const _string& strTarget, _float fAlpha);

public:
	_bool Get_TargetEndWorldPos(const _string& strTarget, _vector& vOutWorld);
	void Set_SpaceMatrix(_fmatrix mat) { m_matModelToWorld = mat; m_matWorldToModel = XMMatrixInverse(nullptr, mat);  }
	void Set_Target(const _string& strGoal, _fvector vWorldPos, _fvector vNormal);
	void Register_Targets(const _string& strFolderPath);
	_uint Register_Target(const _string& strName, EIKSOLVER_TYPE eSolver, const vector<_string>& BoneNames);


public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;
	virtual HRESULT		Render() override;

public:
	void Execute(_float fTimeDelta);

#ifdef _DEBUG
public:
	void Debug_DrawActiveIK(_fmatrix WorldMatrix, const JPH::Color& ChainColor, const JPH::Color& TargetColor);
#endif


private:
	// 참조용 컴포넌트 (약한 참조)
	class CModel* m_pModelCom = { nullptr };
	class CTransform* m_pTransformCom = { nullptr };


private:
	// 모든 Goal이 공유할 알고리즘 클래스이므로 state-less
	vector<class CIKSolver*> m_Solvers{};

private:
	// 부위 마다 1개를 가집니다.
	vector<IK_TARGET> m_Targets{};
	IK_SOLVE_WORKSPACE m_Workspace{};	// 솔버에 빌려주는 스크래치 (컴포넌트당 1개)
	unordered_map<_string, _uint> m_TargetHandles;
	_matrix m_matWorldToModel{};
	_matrix m_matModelToWorld{};

private:
	_uint Find_BoneIndex(const char* pBoneName);
	EIKSOLVER_TYPE To_SolverType(const _string& strSolverType);

	void Parse_SolverOptions(const json& j, IK_TARGET& Target);

public:
	static CIKComponent* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

