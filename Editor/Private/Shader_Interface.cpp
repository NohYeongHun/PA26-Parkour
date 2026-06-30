
#include "EditorPch.h"
#include "Shader_Interface.h"

CShader_Interface::CShader_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CInterface_Edit { pDevice, pContext }
{
}

HRESULT CShader_Interface::Initialize()
{
	return S_OK;
}

void CShader_Interface::Update_Shadow()
{

	Set_ShadowBias();
	Setting_LUT();
#ifdef _DEBUG
	Set_SSAO();
#endif // _DEBUG


}

void CShader_Interface::Setting_Shader()
{
	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD0) == KEYSTATE::DOWN)
		m_pGameInstance->End_SFX();

	if (m_pGameInstance->Get_DIKeyState(DIK_NUMPAD1) == KEYSTATE::DOWN)
		m_pGameInstance->Begin_Toggle_SFX(SFX_TOGGLE::RADIAL);


	ImGui::Begin("SHADER SETTING");

	if(ImGui::CollapsingHeader("SET_RADIAL"))
	{
		ImGui::InputFloat2("CENTER", m_vCenterUV);

		ImGui::DragFloat("MIN_RANGE", &m_vRange[0], 0.01f, 0.1f);
		ImGui::SameLine();
		ImGui::DragFloat("MAX_RANGE", &m_vRange[1], 0.01f, 0.1f, 1.f);

		ImGui::InputFloat("SCALE", &m_fRadialScale);
		m_pGameInstance->Setting_Radial(_float2(m_vCenterUV[0], m_vCenterUV[1]), _float2(m_vRange[0], m_vRange[1]), m_fRadialScale);
	}

	ImGui::End();



	/*
#pragma region CASCADE
	if (ImGui::CollapsingHeader("CASCADE"))
	{
		if (ImGui::CollapsingHeader("Base Bias"))
		{
			Setting_Bias("BIAS_CASCADE[0]", &m_fBias[0]);
			Setting_Bias("BIAS_CASCADE[1]", &m_fBias[1]);
			Setting_Bias("BIAS_CASCADE[2]", &m_fBias[2]);
			Setting_Bias("BIAS_CASCADE[3]", &m_fBias[3]);
		}

		if (ImGui::CollapsingHeader("Min Bias"))
		{
			Setting_Bias("MIN_BIAS_CASCADE[0]", &m_fMinBias[0]);
			Setting_Bias("MIN_BIAS_CASCADE[1]", &m_fMinBias[1]);
			Setting_Bias("MIN_BIAS_CASCADE[2]", &m_fMinBias[2]);
			Setting_Bias("MIN_BIAS_CASCADE[3]", &m_fMinBias[3]);
		}

		if (ImGui::CollapsingHeader("SLOPE_SCALE"))
		{
			//Setting_Bias("SLOPE_SCALE", &m_fSlopeScale);
			ImGui::InputFloat("SCALE", &m_fSlopeScale);
		}

#ifdef _DEBUG
		m_pGameInstance->Bind_RawValue_Renderer("g_fShadowBais", &m_fBias, sizeof(_float4));
		m_pGameInstance->Bind_RawValue_Renderer("g_fMinShadowBias", &m_fMinBias, sizeof(_float4));
		m_pGameInstance->Bind_RawValue_Renderer("g_DebugSlopeScale", &m_fSlopeScale, sizeof(_float));
#endif // _DEBUG
	}
#pragma endregion

#pragma region SSAO
	if (ImGui::CollapsingHeader("SSAO"))
	{

#ifdef _DEBUG
		if (ImGui::Button("SSAO_ON"))
			m_pGameInstance->IsSSAO(true);

		ImGui::SameLine();

		if (ImGui::Button("SSAO_OFF"))
			m_pGameInstance->IsSSAO(false);
#endif // _DEBUG

//		ImGui::InputFloat("RADIUS", &m_fRadius);
//
//		ImGui::DragFloat("MAX_DISTANCE", &m_fMaxDistance, 1.f, 1.f, 50.f, "%.1f");
//
//#ifdef _DEBUG
//		m_pGameInstance->Setting_SSAO(m_fRadius, m_fMaxDistance);
//#endif // _DEBUG
	}
#pragma endregion

#pragma region LUT
	if (ImGui::CollapsingHeader("LUT"))
	{
		ImGui::InputInt("INDEX", &m_iLUT_Index, 1, 1);
		if (m_iLUT_Index < 0)
			m_iLUT_Index = 0;
		if (m_iLUT_Index >= 5)
			m_iLUT_Index = 4;

		ImGui::DragFloat("LUT_INTENSITY", &m_fLUT_Intensity, 0.01f, 0.f, 1.f);
#ifdef _DEBUG
		m_pGameInstance->Set_LUT_Index(m_iLUT_Index);
		m_pGameInstance->Bind_RawValue_Renderer("g_fLutLerpIntensity", &m_fLUT_Intensity, sizeof(_float));
#endif // _DEBUG
	}
#pragma endregion

#pragma region BLOOM
//	if (ImGui::CollapsingHeader("BLOOM"))
//	{
//
//		ImGui::InputInt("WEIGHT", &m_iBloomWeight, 1, 1);
//		if (m_iBloomWeight < 0)
//			m_iBloomWeight = 0;
//		if (m_iBloomWeight >= 5)
//			m_iBloomWeight = 4;
//
//		ImGui::DragFloat("BLOOM_INTENSITY", &m_fBloomIntensity, 0.1f, 0.1f, 5.f);
//		
//#ifdef _DEBUG
//		//m_pGameInstance->SetBloomIntensity(m_fBloomIntensity);
//#endif // _DEBUG
//	}
#pragma endregion
	
#pragma region BLUR

//	if (ImGui::CollapsingHeader("SCREEN_EFFECT"))
//	{
//		if (ImGui::Button("DOF ON"))
//		{
//
//		}
//#ifdef _DEBUG
//	//		m_pGameInstance->Begin_ScreenEffect(SFX_TYPE::DOF);
//#endif
//		if (ImGui::Button("BLUR ON"))
//		{
//
//		}
//#ifdef _DEBUG
//		//	m_pGameInstance->Begin_ScreenEffect(SFX_TYPE::BLUR);
//#endif
//		if (ImGui::Button("MOTION ON"))
//		{
//
//		}
//#ifdef _DEBUG
//	//		m_pGameInstance->Begin_ScreenEffect(SFX_TYPE::MOTION);
//#endif
//			if (ImGui::Button("OFF"))
//			{
//
//		}
#ifdef _DEBUG
	//		m_pGameInstance->End_ScreenEffect();
#endif

		//ImGui::InputFloat("EFFECT_SPEED", &m_fEffectIntensity);

		//if(ImGui::CollapsingHeader("SET_DOF"))
		//{
		//	ImGui::InputFloat("FOCUS", &m_fFocusDepth);

		//	ImGui::InputFloat("RANGE", &m_fFocusRange);

		//	ImGui::InputFloat("DEPTH_SCALE", &m_fDofDepthScale);
		//}

#ifdef _DEBUG
		//m_pGameInstance->SetDof(m_fFocusDepth, m_fFocusRange, m_fDofDepthScale);
#endif
//	}

#pragma endregion
//
//#pragma region PBR
//	if (ImGui::CollapsingHeader("PBR"))
//	{
//#ifdef _DEBUG
//		if (ImGui::Button("STYLIZED"))
//			m_pGameInstance->SetPBR(true);
//
//		ImGui::SameLine();
//
//		if (ImGui::Button("DEFAULT"))
//			m_pGameInstance->SetPBR(false);
//#endif // _DEBUG
//		
//
//
//		ImGui::DragFloat("ROUGHNESS", &m_fRoughness, 0.01f, 0.1f, 1.f);
//
//		ImGui::DragFloat("METALLIC", &m_fMetallic, 0.01f, 0.f, 1.f);
//
//	}
//#pragma endregion
	/*
	if (ImGui::CollapsingHeader("MOTION_BLUR"))
	{
		ImGui::InputFloat("LIMIT_VELOCITY", &m_fLimitVelocity);

		ImGui::InputFloat("LIMIT_DEPTH", &m_fLimitDepth);

		ImGui::InputFloat("DISTANCE_SCALE", &m_fBlurDistanceScale);
#ifdef _DEBUG
		m_pGameInstance->SetMotionBlur(m_fLimitVelocity, m_fLimitDepth, m_fBlurDistanceScale);
#endif
	}
	ImGui::End();*/
}

void CShader_Interface::Set_ShadowBias()
{
	ImGui::Begin("SHADOW BIAS");

	
	if(ImGui::CollapsingHeader("Base Bias"))
	{
		Setting_Bias("BIAS_CASCADE[0]", &m_fBias[0]);
		Setting_Bias("BIAS_CASCADE[1]", &m_fBias[1]);
		Setting_Bias("BIAS_CASCADE[2]", &m_fBias[2]);
		Setting_Bias("BIAS_CASCADE[3]", &m_fBias[3]);
	}

	if (ImGui::CollapsingHeader("Min Bias"))
	{
		Setting_Bias("MIN_BIAS_CASCADE[0]", &m_fMinBias[0]);
		Setting_Bias("MIN_BIAS_CASCADE[1]", &m_fMinBias[1]);
		Setting_Bias("MIN_BIAS_CASCADE[2]", &m_fMinBias[2]);
		Setting_Bias("MIN_BIAS_CASCADE[3]", &m_fMinBias[3]);
	}

	if (ImGui::CollapsingHeader("SLOPE_SCALE"))
	{
		//Setting_Bias("SLOPE_SCALE", &m_fSlopeScale);
		ImGui::InputFloat("SCALE", &m_fSlopeScale);
	}

#ifdef _DEBUG
	m_pGameInstance->Bind_RawValue_Renderer("g_fShadowBais", &m_fBias, sizeof(_float4));
	m_pGameInstance->Bind_RawValue_Renderer("g_fMinShadowBias", &m_fMinBias, sizeof(_float4));
	m_pGameInstance->Bind_RawValue_Renderer("g_DebugSlopeScale", &m_fSlopeScale, sizeof(_float));
#endif // _DEBUG


	
	ImGui::End();
}

#ifdef _DEBUG

void CShader_Interface::Set_SSAO()
{
	ImGui::Begin("SSAO");

	if (ImGui::Button("SSAO_ON"))
		m_pGameInstance->IsSSAO(true);

	ImGui::SameLine();

	if (ImGui::Button("SSAO_OFF"))
		m_pGameInstance->IsSSAO(false);

	if (ImGui::Button("BLUR_ON"))
		m_pGameInstance->IsSSAO_Blur(true);

	ImGui::SameLine();

	if (ImGui::Button("BLUR_OFF"))
		m_pGameInstance->IsSSAO_Blur(false);


	ImGui::InputFloat("RADIUS", &m_fRadius);

	ImGui::DragFloat("MAX_DISTANCE", &m_fMaxDistance, 1.f, 1.f, 50.f, "%.1f");

	ImGui::End();
}

#endif // _DEBUG

void CShader_Interface::Setting_Bias(const _char* pName, _float* pFloat)
{
	ImGui::PushItemWidth(250.f);
	ImGui::DragFloat(pName, pFloat, 0.001f, 0.001f, 0.5f);
	ImGui::PopItemWidth();
}

void CShader_Interface::Setting_LUT()
{
	ImGui::Begin("LUT");

	if (ImGui::BeginCombo("LUT_INDEX", "LUT"))
	{
		for (_uint i = 0; i < 5; ++i)
		{

#ifdef _DEBUG
			if (ImGui::Selectable(to_string(i).c_str()))
			{
				m_iLUT_Index = i;
			}
#endif // _DEBUG
		}

		ImGui::EndCombo();
	}

	if (ImGui::CollapsingHeader("LUT_LERP_INTENSITY"))
	{
		ImGui::PushItemWidth(250.f);
		ImGui::DragFloat("LUT_INTENSITY", &m_fLUT_Intensity, 0.01f, 0.01f, 1.f);

		ImGui::PopItemWidth();

	}

	m_pGameInstance->Setting_LUT(m_iLUT_Index, m_fLUT_Intensity, true);
#ifdef _DEBUG

//	m_pGameInstance->Bind_RawValue_Renderer("g_fLutLerpIntensity", &m_fLUT_Intensity, sizeof(_float));
#endif // _DEBUG


	
	ImGui::End();

}

CShader_Interface* CShader_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CShader_Interface* pInstance = new CShader_Interface(pDevice, pContext);
	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CShader_Interface");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CShader_Interface::Free()
{
	__super::Free();

}
