#include "EditorPch.h"
#include "Interface_Edit.h"

CInterface_Edit::CInterface_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pGameInstance { CGameInstance::GetInstance() },
	m_pDevice { pDevice }, m_pContext { pContext }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

void CInterface_Edit::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
}
