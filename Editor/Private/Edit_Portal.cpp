#include"EditorPch.h"
#include "Edit_Portal.h"

CEdit_Portal::CEdit_Portal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{
}

HRESULT CEdit_Portal::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEdit_Portal::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;
	Ready_Components(pArg);
    return S_OK;
}

void CEdit_Portal::Priority_Update(_float fTimeDelta)
{
	m_fTotalTime += fTimeDelta;
}

void CEdit_Portal::Update(_float fTimeDelta)
{
	//VTXPOSTEX 셰이더 사용하고. 디퓨즈는 모르겠지만 마스킹을 이용해서 Texcoord 밀기.
	//디졸브 수치에 비례해서 색 바꾸는 것처럼 마스킹의 Texcoord에 따라서 이미시브 효과 등 넣어보기?

	ImGui::Begin("Portal");

	ImGuiID Portal = ImGui::GetID("Container");
	ImGui::BeginChildFrame(Portal, ImVec2(100, 200));
#ifdef _DEBUG
	for (_uint i = 0; i < m_pShaderCom->Get_PassCount(); ++i)
	{
		if (ImGui::Button(m_pShaderCom->Get_PassName(i)))
			m_iShaderPassIndex = i;
	}
#endif
	ImGui::EndChildFrame();
	//DirectX::

	ImGui::ColorPicker4("Color", m_vColor);

	ImGui::End();
}

void CEdit_Portal::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_Render_Object(RENDERGROUP::NONBLEND, this);
}

void CEdit_Portal::Render()
{
	m_pTransformCom->Bind_Matrix(m_pShaderCom, "g_WorldMatrix");
	m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::VIEW));
	m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_TransformState_Float4x4(D3DTS::PROJ));

	m_pDiffuseTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_DiffuseTexture");
	m_pMaskTextureCom->Bind_Shader_Resource(m_pShaderCom, "g_MaskTexture");
	
	m_pShaderCom->Bind_Value("g_TotalTime", &m_fTotalTime, sizeof(_float));
	m_pShaderCom->Bind_Value("vColor", &m_vColor, sizeof(_float4));
	m_pVIBufferCom->Bind_Resources();
	m_pShaderCom->Begin(m_iShaderPassIndex);
	m_pVIBufferCom->Render();
}

void CEdit_Portal::Ready_Components(void* pArg)
{
	// Shader
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Shader Load Failed. The Shader may have already been loaded.\n");
	
	// Buffer
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Shader Load Failed. The Shader may have already been loaded.\n");

	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Texture_Potal_Diffuse"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Map/Potal/T_Scene_30021.png"), 1))))
		OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Shader Load Failed. The Shader may have already been loaded.\n");
	
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Texture_Potal_Mask"),
		CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Map/Potal/T_Ring_10016.png"), 1))))
		//CTexture::Create(m_pDevice, m_pContext, TEXT("../../Client/Bin/Resource/Map/Potal/T_Mask_18312.png"), 1))))
		OutputDebugString(L"[CCustom_UI::Ready_Prototypes] Shader Load Failed. The Shader may have already been loaded.\n");


	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Texture_Potal_Diffuse"),
		TEXT("Com_Diffuse"), reinterpret_cast<CComponent**>(&m_pDiffuseTextureCom), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_Texture_Potal_Mask"),
		TEXT("Com_Mask"), reinterpret_cast<CComponent**>(&m_pMaskTextureCom), nullptr)))
		CRASH("FAILED");

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::MAP), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), nullptr)))
		CRASH("FAILED");
}

CEdit_Portal* CEdit_Portal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEdit_Portal* pInstance = new CEdit_Portal(pDevice, pContext);
	pInstance->Initialize_Prototype();
    return pInstance;
}

CGameObject* CEdit_Portal::Clone(void* pArg)
{
	CEdit_Portal* pInstance = new CEdit_Portal(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
		CRASH("Failed");
	return pInstance;
}


void CEdit_Portal::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pDiffuseTextureCom);
	Safe_Release(m_pMaskTextureCom);
	Safe_Release(m_pVIBufferCom);

}
