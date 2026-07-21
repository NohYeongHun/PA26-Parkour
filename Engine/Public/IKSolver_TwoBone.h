#pragma once
#include "IKSolver.h"
NS_BEGIN(Engine)
class CIKSolver_TwoBone final : public CIKSolver
{
private:
	explicit CIKSolver_TwoBone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CIKSolver_TwoBone(const CIKSolver_TwoBone& Prototype);
	virtual ~CIKSolver_TwoBone() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;
	virtual HRESULT		Render() override;

public:
	virtual IK_RESULT    Solve(const IK_SOLVE_CONTEXT& Context) override;
	virtual const _char* Get_Name() const override;

public:
	static CIKSolver_TwoBone* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual CIKSolver* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

