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
	void Register_AllSolvers(const _string& strFolderPath);
	_uint Register_Goal(const _string& strName, EIKSOLVER_TYPE eSolver, const vector<_string>& BoneNames);

public:
	void Begin_Goal(const _string& strGoalName, EIKTARGET_MODE eMode, _float fPosWeight, _float fRotWeight, _float fBlendSec);
	void End_Goal(const _string& strGoalName, _float fBlendSec);

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;
	virtual HRESULT		Render() override;

public:
	void Update(_float fTimeDelta);


private:
	// 참조용 컴포넌트 (약한 참조)
	class CModel* m_pModelCom = { nullptr };
	class CTransform* m_pTransformCom = { nullptr };


private:
	// 모든 Goal이 공유할 알고리즘 클래스이므로 state-less
	vector<class CIKSolver*> m_Solvers{};
	// 부위 마다 1개를 가집니다.
	vector<IK_GOAL> m_Goals{};
	
	unordered_map<_string, _uint> m_GoalHandles;

private:
	_uint Find_BoneIndex(const char* pBoneName);

public:
	static CIKComponent* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

