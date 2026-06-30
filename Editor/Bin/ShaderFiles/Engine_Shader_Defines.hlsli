#ifndef Engine_Shader_Defines_h__
#define Engine_Shader_Defines_h__

#include "Engine_Shader_State.hlsli"

float g_fLUT_Size = 16.f;

//���������� �������� ���� �� �������� ?
float g_fEmissiveThreshold = 0.7f;

float g_fLuminence[3] = { 0.2126f, 0.7152f, 0.0722f };

//PBR
float g_fGlobalDynamicMetallic = 0.1f;  // PBR.x
float g_fGlobalDynamicRoughness = 0.35; // PBR.y

float g_fGlobalStaticMetallic = 0.2f;
float g_fGlobalStaticRoughness = 0.5f;

//Effect
float g_WeightBlend = 0.02f;
float g_EmssiveColorWeight = 1.4f;

float Luminance(float3 vColor) 
{
    float fWeight;
    
    fWeight = (vColor.r * g_fLuminence[0]) + (vColor.g * g_fLuminence[1]) + (vColor.b * g_fLuminence[2]);
    
    return fWeight;
}

float CustomLuminance(float3 vColor, float3 vLuminance)
{
    float fWeight;
    
    fWeight = (vColor.r * vLuminance.r) + (vColor.g * vLuminance.g) + (vColor.b * vLuminance.b);
    
    return fWeight;
}

bool IsInNDC(float4 vProjPos)
{
    if (vProjPos.x > 1.f || vProjPos.x < -1.f)
        return false;
    
    if (vProjPos.y > 1.f || vProjPos.y < -1.f)
        return false;
    
    if (vProjPos.z > 1.f || vProjPos.z < 0.f)
        return false;
    
    return true;
}


#endif //Engine_Shader_Defines_h__