#include "ClientPch.h"
#include "MouseController.h"
#include "Mouse.h"

CMouseController::CMouseController()
{
}

void CMouseController::Register_Mouse(CMouse* pMouse)
{
	Safe_Release(m_pMouse);
	m_pMouse = pMouse;
	Safe_AddRef(pMouse);
}

void CMouseController::Set_MouseFix(_bool isFix)
{
	if (nullptr == m_pMouse)
		return;

	m_pMouse->Set_MouseFix(isFix);
}

_bool CMouseController::IsFix()
{
	if (nullptr == m_pMouse)
		return false;

	return m_pMouse->IsFix();
}

CMouseController* CMouseController::Create()
{
	return new CMouseController();
}

void CMouseController::Free()
{
	__super::Free();
	Safe_Release(m_pMouse);
}
