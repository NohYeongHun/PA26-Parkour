#pragma once
#include "IKSolver.h"
NS_BEGIN(Engine)
class ENGINE_DLL CIKSovler_Fabrik final : public CIKSolver
{
private:
	explicit CIKSovler_Fabrik(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CIKSovler_Fabrik(const CIKSovler_Fabrik& Prototype);
	virtual ~CIKSovler_Fabrik() = default;

public:
	virtual HRESULT		Initialize_Prototype() override;
	virtual HRESULT		Initialize_Clone(void* pArg) override;
	virtual HRESULT		Render() override;

public:
	virtual IK_RESULT	 Solve(_float fTimeDelta) override;
	virtual const _char* Get_Name() const override;

public:
	static CIKSovler_Fabrik* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	virtual CIKSolver* Clone(void* pArg) override;
	virtual void Free() override;
};
NS_END

