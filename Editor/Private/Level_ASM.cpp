#include "EditorPch.h"
#include "Level_ASM.h"

#include "Event_Level.h"
#include "ASM_Interface.h"

CLevel_ASM::CLevel_ASM(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_ASM::Initialize()
{
    if (FAILED(Ready_Interface()))
        CRASH("Failed Interface");

    return S_OK;
}

void CLevel_ASM::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Animation State Machine"));
    m_pASM_Interface->Update_ASM(fTimeDelta);
}

void CLevel_ASM::Render()
{

}

HRESULT CLevel_ASM::Ready_Interface()
{
    m_pASM_Interface = CASM_Interface::Create(m_pDevice, m_pContext);
    ASSERT_CRASH(m_pASM_Interface);

    return S_OK;
}

HRESULT CLevel_ASM::Ready_TestObjects()
{
    return E_NOTIMPL;
}

CLevel_ASM* CLevel_ASM::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_ASM* pInstance = new CLevel_ASM(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_ASM");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_ASM::Free()
{
    __super::Free();

    Safe_Release(m_pASM_Interface);
}
