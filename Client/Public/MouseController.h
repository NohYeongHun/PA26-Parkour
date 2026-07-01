#pragma once
#include "Base.h"

NS_BEGIN(Client)
class CMouseController : public CBase
{
private:
	explicit CMouseController();
	virtual ~CMouseController() = default;

public:
	void Register_Mouse(class CMouse* pMouse);
	void Set_MouseFix(_bool isFix);
	_bool IsFix();

private:
	class CMouse* m_pMouse = { nullptr };

public:
	static CMouseController* Create();
	virtual void Free() override;
};

NS_END


