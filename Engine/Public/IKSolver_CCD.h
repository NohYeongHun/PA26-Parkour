#pragma once
#include "IKSolver.h"
NS_BEGIN(Engine)
class CIKSolver_CCD final : public CIKSolver
{
private:
	explicit CIKSolver_CCD(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CIKSolver_CCD(const CIKSolver_CCD& Prototype);
	virtual ~CIKSolver_CCD() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;
	virtual HRESULT		Render() override;

public:
	virtual IK_RESULT	 Solve(const IK_SOLVE_CONTEXT& Context) override;
	virtual const _char* Get_Name() const override;

public:
	static CIKSolver_CCD* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual CIKSolver* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

