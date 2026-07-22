#pragma once
#include "Base.h"
NS_BEGIN(Engine)
class CIKSolver abstract : public CBase
{
protected:
	explicit CIKSolver(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CIKSolver(const CIKSolver& Prototype);
	virtual ~CIKSolver() = default;

public:
	virtual HRESULT		Initialize_Prototype();
	virtual HRESULT		Initialize_Clone(void* pArg);
	virtual HRESULT		Render();

public:
	static _vector TwoBoneMakePoleVector(_fvector vRootPos, _fvector vMidPos, _vector  vEndPos, _float Distance = 2.f);

public:
	virtual const _char* Get_Name() const = 0;

public:
	virtual IK_RESULT Solve(const IK_SOLVE_CONTEXT& Context) = 0;
	

protected:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };
	_bool m_isClone = { false };

public:
	virtual CIKSolver* Clone(void* pArg) = 0;
	virtual void Free() override;

	

};
NS_END

