#include "EditorPch.h"
#include "Level_Animation.h"

#include "Event_Level.h"
#include "AnimationTool.h"

CLevel_Animation::CLevel_Animation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CLevel { pDevice, pContext }
{
}

HRESULT CLevel_Animation::Initialize()
{
    m_pAnimationTool = CAnimationTool::Create(m_pDevice, m_pContext, m_eCurLevel);
	


    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
        CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl")
            , VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
    {
        CRASH("Failed Load AnimMesh Shader");
        return E_FAIL;
    }

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(m_eCurLevel), TEXT("Prototype_Component_Shader_VtxAnimMeshCharacter"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxAnimMeshCharacter.hlsl")
			, VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
	{
		CRASH("Failed Load AnimMeshCharacter Shader");
		return E_FAIL;
	}

	LIGHT_DESC LightDesc{};
	LightDesc.eType = LIGHT_DESC::DIRECTION;
	LightDesc.vAmbient = _float4(0.2f, 0.2f, 0.2f, 1.f);
	LightDesc.vDiffuse = _float4(0.8f, 0.8f, 1.f, 1.f);
	LightDesc.vDirection = _float4(1.f, -0.5f, -1.f, 0.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	m_pGameInstance->Add_Light(TEXT("Test"), LightDesc);
	m_pGameInstance->SetUp_ShadowLight(TEXT("Test"));
	m_pGameInstance->SetUp_CameraNF();

    
    return S_OK;
}

void CLevel_Animation::Update(_float fTimeDelta)
{
    SetWindowText(g_hWnd, TEXT("Anim"));

	//m_pGltfLoader->Update();

}

void CLevel_Animation::Render()
{
    m_pAnimationTool->Render();
}

CLevel_Animation* CLevel_Animation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CLevel_Animation* pInstance = new CLevel_Animation(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Create : Level_Animation");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CLevel_Animation::Free()
{
    __super::Free();
    Safe_Release(m_pAnimationTool);

}
