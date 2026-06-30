#include "EditorPch.h"
#include "Camera_Interface.h"

CCamera_Interface::CCamera_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CInterface_Edit { pDevice, pContext }
{
}

HRESULT CCamera_Interface::Initialize()
{
	return S_OK;
}

CCamera_Interface* CCamera_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Interface* pInstance = new CCamera_Interface(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Create : Camera_Interface");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Interface::Free()
{
	__super::Free();
}
