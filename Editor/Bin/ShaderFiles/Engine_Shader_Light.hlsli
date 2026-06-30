#include "Engine_Shader_Shadow.hlsli"

bool g_IsCustomRimColor = false;
float4 g_vRimColor = 0.f;
float4 g_fRimIntensity = 0.8f;

bool g_HasShadowMap;

vector g_vDynamicMtrlAmbient = 0.5f;
vector g_vStaticMtrlAmbient = 0.4f;

Texture2DArray<float> g_ShadowMap;

Texture2DArray<float> g_Cascade : register(t2);

float4 g_vShadowLightDirection;

struct LightData
{
    uint iType; // 0 = Directional, 1 = Point
    float fRange;
    float Padding[2];
    float4 vDiffuse;
    float4 vDirection;
    float4 vPosition;
    float4 vAmbient;
    float4 vSpecular;
};

StructuredBuffer<LightData> g_LightDatas : register(t8);

struct LIGHT_RESULT
{
    float3 vLightDiffuse;
    float3 vLightSpecular;
    float3 vLightAmbient;
};

LIGHT_RESULT Compute_Directional(float4 vDiffuse, float4 vNormal, float4 vWorldPos, float4 vViewPos, bool IsDynamic, float fRoughness, float fMetallic, bool IsSkin, uint iLightIndex)
{
    LIGHT_RESULT Out = (LIGHT_RESULT) 0;
    
    float3 vLook = normalize(g_vCamPosition.xyz - vWorldPos.xyz);
    
    float3 vLightDir = normalize(g_LightDatas[iLightIndex].vDirection.xyz * -1.f);
   
    float NdotL = dot(vLightDir, vNormal.xyz);
     
    float fRimPower = 0.f;
    
    if (IsDynamic)
        fRimPower = Compute_RimPower(vNormal.xyz, vLook, NdotL);

    float3 vRimColor = g_IsCustomRimColor ? g_vRimColor : g_LightDatas[iLightIndex].vDiffuse.xyz;
    float3 vRim = (vRimColor * fRimPower);
    
    float3 vAmbient = 0.f;
    
    float3 vLightDiffuse = 0.f;
    float3 vLightSpecular = 0.f;
    
    float3 vResultDiffuse = 0.f;
    float3 vResultSpecular = 0.f;
    
    float4 vAmbientColor = 0.f;
    
    float fViewZ = vViewPos.z;

    float fShadowMap = 1.f;
    
    float fShadowNdotL = saturate(dot(vNormal, g_vShadowLightDirection * -1.f));
    
    if (IsDynamic)
    {
        if (g_HasShadowMap)
        {
            fShadowMap = clamp(Compute_ShadowMap(fViewZ, fShadowNdotL, vWorldPos, g_ShadowMap), 0.5f, 1.f);
        }
 
 //       float fToonShade = 0.f; //smoothstep(-0.3f, -0.1f, NdotL);
//        bool IsSkin = all(g_SkinMaskTexture.Sample(DefaultSampler, In.vTexcoord).xy > 0.f);
        
        float fToonShade = clamp(smoothstep(cos(radians(120.f)), cos(radians(100.f)), NdotL), 0.5f, 1.f);
        
        float fFinalShadow = min(fToonShade, fShadowMap);
        
        Compute_Stylized_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, fMetallic, fRoughness, vResultDiffuse, vResultSpecular);
        vLightDiffuse = g_LightDatas[iLightIndex].vDiffuse.xyz * ((vResultDiffuse * fFinalShadow));
        vLightSpecular = g_LightDatas[iLightIndex].vDiffuse.xyz * ((vResultSpecular * fFinalShadow)) + vRim;
        
        if (false == IsSkin)      // �ݼ� �κи� PBR ó��
        {
            Out.vLightDiffuse = float4(vLightDiffuse, 1.f);
            Out.vLightSpecular = float4(vLightSpecular, 1.f);
            
            vAmbient = g_vDynamicMtrlAmbient * fFinalShadow;
        }
        else
        {
            float3 vOrigin = vDiffuse.xyz * fShadowMap;
            
            //Out.vLightDiffuse = float4(lerp(vOrigin, vLightDiffuse, g_LightDatas[iLightIndex].vAmbient.xyz), 1.f);
            //Out.vLightSpecular = float4(vLightSpecular, 1.f);
            // 
            //vAmbient = g_LightDatas[iLightIndex].vAmbient;
            
            Out.vLightDiffuse = float4(lerp(vOrigin, vLightDiffuse, 0.6f), 1.f);
            Out.vLightSpecular = float4(lerp(vRim, vLightSpecular, 0.6f), 1.f);
        
            vAmbient = g_vDynamicMtrlAmbient * 0.8f * fFinalShadow;
        }
        
        vAmbientColor = vDiffuse;
    }
    else
    {
    //    Compute_Stylized_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, fMetallic, fRoughness, vResultDiffuse, vResultSpecular);
        Compute_BRDF_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, fMetallic, fRoughness, vResultDiffuse, vResultSpecular);
    
        if (g_HasShadowMap)
        {
            fShadowMap = Compute_ShadowMap(fViewZ, fShadowNdotL, vWorldPos, g_ShadowMap);
        }
   
        float fShadow = Compute_Cascade(fViewZ, fShadowNdotL, vWorldPos, g_Cascade);
    
        float fFinalShadow = min(fShadowMap, fShadow);
    
        vLightDiffuse = g_LightDatas[iLightIndex].vDiffuse.xyz * ((vResultDiffuse * fFinalShadow));
        vLightSpecular = g_LightDatas[iLightIndex].vDiffuse.xyz * ((vResultSpecular * fFinalShadow)) + vRim;
        
        Out.vLightDiffuse = float4(vLightDiffuse, 1.f);
        Out.vLightSpecular = float4(vLightSpecular, 1.f);
        
        vAmbientColor = float4(lerp(vDiffuse.xyz, vResultDiffuse, 0.4f), 1.f);
        vAmbient = g_vStaticMtrlAmbient.xyz;
    }
    
    Out.vLightAmbient = float4((vAmbientColor.xyz * vAmbient), 1.f);
    
    return Out;
}


LIGHT_RESULT Compute_Point(float4 vDiffuse, float4 vNormal, float4 vWorldPos, float4 vViewPos, bool IsDynamic, float fRoughness, float fMetallic, bool IsSkin, uint iLightIndex)
{
    LIGHT_RESULT Out = (LIGHT_RESULT) 0;

    float3 vLook = normalize(g_vCamPosition.xyz - vWorldPos.xyz);
    
    float3 vLightDir = g_LightDatas[iLightIndex].vPosition.xyz - vWorldPos.xyz;
    
    float fDistance = length(vLightDir);
    
    vLightDir = normalize(vLightDir);
    
    float fAtt = saturate((g_LightDatas[iLightIndex].fRange - fDistance) / g_LightDatas[iLightIndex].fRange);
    
    if (fAtt == 0.f)
        return Out;
    
    float NdotL = dot(vLightDir, vNormal.xyz);
   
    float fRimPower = 0.f;
    
    if (IsDynamic)
        fRimPower = Compute_RimPower(vNormal.xyz, vLook, NdotL);
        
    float fToonShade = clamp(smoothstep(cos(radians(120.f)), cos(radians(100.f)), NdotL), 0.5f, 1.f);
 
    float3 vRimColor = g_IsCustomRimColor ? g_vRimColor : g_LightDatas[iLightIndex].vDiffuse.xyz;
 
    float3 vLightDiffuse = 0.f;
    float3 vLightSpecular = 0.f;
    
    float3 vResultDiffuse = 0.f;
    float3 vResultSpecular = 0.f;
 
    if (IsDynamic)
    {
        Compute_Stylized_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, fMetallic, fRoughness, vResultDiffuse, vResultSpecular);
        
        vLightDiffuse = g_LightDatas[iLightIndex].vDiffuse.xyz * ((vResultDiffuse * fToonShade));
        vLightSpecular = g_LightDatas[iLightIndex].vDiffuse.xyz * ((vResultSpecular  * fToonShade)) + (fRimPower * vRimColor);
        
        Out.vLightDiffuse = float4(vLightDiffuse * fAtt, 1.f);
        Out.vLightSpecular= float4(vLightSpecular * fAtt, 1.f);
    }
    else
    {
        Compute_BRDF_PBR(vNormal.xyz, vLook.xyz, vLightDir, vDiffuse.xyz, fMetallic, fRoughness, vResultDiffuse, vResultSpecular);
        
        vLightDiffuse = g_LightDatas[iLightIndex].vDiffuse.xyz * ((vResultDiffuse));
        vLightSpecular = g_LightDatas[iLightIndex].vDiffuse.xyz * ((vResultSpecular)) + (fRimPower * vRimColor);
        
        Out.vLightDiffuse = float4(vLightDiffuse * fAtt, 1.f);
        Out.vLightSpecular = float4(vLightSpecular * fAtt, 1.f);
    }

    //float4 vAmbientColor = vDiffuse * g_LightDatas[iLightIndex].vDiffuse; //Ambient ���� ������ �ӽ�
    float4 vAmbientColor = lerp(vDiffuse, g_LightDatas[iLightIndex].vDiffuse, g_LightDatas[iLightIndex].vAmbient.r);
    float4 vAmbient = float4((vAmbientColor * g_LightDatas[iLightIndex].vAmbient).xyz * fAtt, 1.f);
    //float4 vAmbient = float4((vAmbientColor.xyz * 0.5f) * fAtt, 1.f);
    
    Out.vLightAmbient = vAmbient * fToonShade;
    
    return Out;
}