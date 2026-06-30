#include "EditorPch.h"
#include "Galbrena_SFX.h"

CGalbrena_SFX::CGalbrena_SFX(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEdit_ScreenEffect { pDevice, pContext }
{
}

CGalbrena_SFX::CGalbrena_SFX(const CGalbrena_SFX& Prototype)
	: CEdit_ScreenEffect { Prototype }
{
}

HRESULT CGalbrena_SFX::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	m_vEffectTime = _float2(0.f, 0.5f);

	m_vScale = _float2(1.f, 0.1f);

	m_vDrawRadians = _float2(XMConvertToRadians(25.f), XMConvertToRadians(15.f));

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Componnent_VIBuffer_Rect_Instance"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBuffer), nullptr)))
		ASSERT_CRASH(m_pVIBuffer);

	if (FAILED(CGameObject::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_SFX_Burst_Instance"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShader), nullptr)))
		ASSERT_CRASH(m_pShader);

	m_fSizeX[0] = 400.f;
	m_fSizeX[1] = 2000.f;
	m_fSizeX[2] = 300.f;
	m_fSizeX[3] = 700.f;
	m_fSizeX[4] = 400.f;

	m_fSizeY[0] = 1150.f;
	m_fSizeY[1] = 1500.f;
	m_fSizeY[2] = 700.f;
	m_fSizeY[3] = 750.f;
	m_fSizeY[4] = 1000.f;

	m_fRadians[0] = 35.f;
	m_fRadians[1] = 115.f;
	m_fRadians[2] = 135.f;
	m_fRadians[3] = 250.f;
	m_fRadians[4] = 315.f;

	m_vPivotRate = _float2(0.7f, 0.75f);

	m_vDrawRadians = _float2(XMConvertToRadians(25.f), XMConvertToRadians(15.f));

	Ready_Instance();

	m_fCurrentTime = 0.f;
	m_fCurrentScale = 1.f;
	m_fCurrentRadian = m_vDrawRadians.x;

	return S_OK;
}

HRESULT CGalbrena_SFX::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		return E_FAIL;

	return S_OK;
}

void CGalbrena_SFX::Priority_Update(_float fTimeDelta)
{

}

void CGalbrena_SFX::Update(_float fTimeDelta)
{
	if (false == m_IsPlay)
		return;

	m_fCurrentTime += fTimeDelta;
	
	_float fRatio = SmoothStep(m_vEffectTime.x, m_vEffectTime.y, m_fCurrentTime);
	
	m_fCurrentRadian = lerp(m_vDrawRadians.x, m_vDrawRadians.y, fRatio);
	m_fCurrentScale = lerp(m_vScale.x, m_vScale.y, fRatio);
}

void CGalbrena_SFX::Late_Update(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_Render_Object(RENDERGROUP::SFX, this)))
		return;
}

void CGalbrena_SFX::Render()
{
	ImGui::Begin("SLASH");

	if (ImGui::BeginCombo("SLASH_INDEX", "SLASH"))
	{
		for (_uint i = 0; i < 5; ++i)
		{
			if (ImGui::Selectable(to_string(i).c_str()))
			{
				m_iSlashIndex = i;
			}
		}

		ImGui::EndCombo();
	}


	ImGui::InputFloat("PIVOT_X", &m_vPivotRate.x, 0.01f, 0.1f);
	ImGui::InputFloat("PIVOT_Y", &m_vPivotRate.y, 0.01f, 0.1f);

	ImGui::DragFloat("SLASH_WIDTH", &m_fSizeX[m_iSlashIndex], 1.f, 100.f, 3000.f);
	ImGui::DragFloat("SLASH_HEIGHT", &m_fSizeY[m_iSlashIndex], 1.f, 700.f, 1500.f);
	ImGui::DragFloat("RADIANS", &m_fRadians[m_iSlashIndex], 1.f, 0.f, 360.f);

	ImGui::End();

	Ready_Instance();

	/*if (FAILED(m_pTransformCom->Bind_Matrix(m_pShader, "g_WorldMatrix")))
		CRASH("Failed to Bind WorldMatrix");*/

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		CRASH("Failed to Bind ViewMatrix");

	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		CRASH("Failed to Bind ProjMatrix");

	if(FAILED(m_pShader->Bind_Value("g_vColor", &m_vColor, sizeof(_float3))))
		CRASH("Failed to Bind vColor");

	if (FAILED(m_pShader->Bind_Value("g_fMaxRadian", &m_fCurrentRadian, sizeof(_float))))
		CRASH("Failed to Bind vColor");

	m_pShader->Begin(0);

	m_pVIBuffer->Bind_Resources();
	m_pVIBuffer->Render();
}

void CGalbrena_SFX::Play()
{
	m_IsPlay = true;
	m_fCurrentTime = 0.f;
	m_fCurrentScale = 1.f;
	m_fCurrentRadian = m_vDrawRadians.x;
}

void CGalbrena_SFX::Stop()
{
	m_IsPlay = false;
	m_fCurrentTime = 0.f;
	m_fCurrentScale = 1.f;
	m_fCurrentRadian = m_vDrawRadians.x;
}

void CGalbrena_SFX::Reset()
{
}


void CGalbrena_SFX::Ready_Instance()
{

	vector<VTXINSTANCE_RECT> Datas;

	m_vPivot = _float2(m_vWinSize.x * m_vPivotRate.x, m_vWinSize.y * m_vPivotRate.y);

	Datas.push_back(Make_Instance(m_vPivot, m_fSizeX[0] * m_fCurrentScale, m_fSizeY[0], XMConvertToRadians(m_fRadians[0])));
	Datas.push_back(Make_Instance(m_vPivot, m_fSizeX[1] * m_fCurrentScale, m_fSizeY[1], XMConvertToRadians(m_fRadians[1])));
	Datas.push_back(Make_Instance(m_vPivot, m_fSizeX[2] * m_fCurrentScale, m_fSizeY[2], XMConvertToRadians(m_fRadians[2])));
	Datas.push_back(Make_Instance(m_vPivot, m_fSizeX[3] * m_fCurrentScale, m_fSizeY[3], XMConvertToRadians(m_fRadians[3])));
	Datas.push_back(Make_Instance(m_vPivot, m_fSizeX[4] * m_fCurrentScale, m_fSizeY[4], XMConvertToRadians(m_fRadians[4])));

	m_pVIBuffer->Update_Buffer(Datas);
}

VTXINSTANCE_RECT CGalbrena_SFX::Make_Instance(_float2 vPivot, _float fSizeX, _float fSizeY, _float fRotateZ)
{
	VTXINSTANCE_RECT Rect = {};

	_vector vRotation = XMQuaternionRotationRollPitchYaw(0.f, 0.f, fRotateZ);		// Rotate
	_vector vAnchor = XMVectorSet(0.f, -0.5f * fSizeY, 0.f, 1.f);				// Center Bottom ( Rect Pivot )

	vAnchor = XMVector3Rotate(vAnchor, vRotation);									

	Setting_Scale(fSizeX, fSizeY);
	m_pTransformCom->Rotation_Quaternion(vRotation);
	Setting_Pos(vPivot.x - XMVectorGetX(vAnchor), vPivot.y + XMVectorGetY(vAnchor));

	memcpy(&Rect.vRight, m_pTransformCom->Get_WorldMatrixPtr(), sizeof(_matrix));

	return Rect;
}

CGalbrena_SFX* CGalbrena_SFX::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGalbrena_SFX* pInstance = new CGalbrena_SFX(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CGalbrena_SFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CGalbrena_SFX::Clone(void* pArg)
{
	CGalbrena_SFX* pInstance = new CGalbrena_SFX(*this);
	if (FAILED(pInstance->Initialize_Clone(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGalbrena_SFX");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CGalbrena_SFX::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pShader);
}
