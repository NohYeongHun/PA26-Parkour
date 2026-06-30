#include "Engine_Shader_Function.hlsli"
#include "Engine_Shader_Defines.hlsli"

#define MAX_ENVMAP 8
#define FLT_MAX 3.402823466e+38F
#define EPSILON 1e-5

float g_fMinStepSize;
float g_fMaxStepSize;
float g_fStartOffset;

float g_fMaxDepth;

float g_fMinTickness;
float g_fMaxTickness;

const static uint g_iBinaryStep = 6;
const static uint g_iStep = 24;
const static float g_fMaxDistance = 400.f;

struct ENV_MAP
{
    float4 vPosition;
    uint iIndex;
    float fRange;
    float Padding[2];
};

uint        g_iNumEnvMaps;
StructuredBuffer<ENV_MAP> g_EnvMapDatas : register(t3);

bool        g_HasEnvMap;
TextureCube g_EnvMapTexture[MAX_ENVMAP] : register(t4);

ENV_MAP Find_EnvMap(float4 vWorldPos, out bool isFind)
{
    ENV_MAP EnvMap = (ENV_MAP) 0;
    
    uint iNumEnvMap = clamp(g_iNumEnvMaps, 0, MAX_ENVMAP);
    float fMinDistance = FLT_MAX;
    
    isFind = false;
    
    for (uint i = 0; i < iNumEnvMap; ++i)
    {
        ENV_MAP Temp = (ENV_MAP) 0;
        Temp = g_EnvMapDatas[i];
        
        float fLength = length(vWorldPos - Temp.vPosition);
        
        if(Temp.fRange >= fLength && fMinDistance > fLength)
        {
            fMinDistance = fLength;
            isFind = true;
            EnvMap = Temp;
        }
    }
    return EnvMap;
}


float4 SampleEnvMap(int iIndex, float3 vUV)
{
    float4 vColor = (float4) 0;
    
    switch (iIndex)
    {
        case 0:
            vColor = g_EnvMapTexture[0].Sample(DefaultSampler, vUV);
            break;
        case 1:
            vColor = g_EnvMapTexture[1].Sample(DefaultSampler, vUV);
            break;
        case 2:
            vColor = g_EnvMapTexture[2].Sample(DefaultSampler, vUV);
            break;
        case 3:
            vColor = g_EnvMapTexture[3].Sample(DefaultSampler, vUV);
            break;
        case 4:
            vColor = g_EnvMapTexture[4].Sample(DefaultSampler, vUV);
            break;
        case 5:
            vColor = g_EnvMapTexture[5].Sample(DefaultSampler, vUV);
            break;
        case 6:
            vColor = g_EnvMapTexture[6].Sample(DefaultSampler, vUV);
            break;
        case 7:
            vColor = g_EnvMapTexture[7].Sample(DefaultSampler, vUV);
            break;
    }
    
    return vColor;
}


float4 Compute_Reflect(float4 vWorldPos, float4 vViewPos, 
                        float4 vViewNormal, float4 vOriginColor)
{
    float4 vColor = vOriginColor;

    if (vViewPos.z == 0.f || false == g_HasEnvMap)
    {
        return vColor;
    }
    
    bool isFind = false;
    ENV_MAP FindMap = Find_EnvMap(vWorldPos, isFind);
    
    if(isFind)
    {
        float3 vLook = normalize(vViewPos.xyz);
    
        float3 vReflect = normalize(reflect(vLook.xyz, vViewNormal.xyz));
               
        float3 vWorldReflect = normalize(mul(float4(vReflect, 0.f), g_ViewMatrixInv).xyz);
        float3 vSafeWorldReflect = max(abs(vWorldReflect), EPSILON) * sign(vWorldReflect);
        
        float3 vLocalPos = vWorldPos.xyz - FindMap.vPosition.xyz;
        
        float3 vExtents = FindMap.fRange;
        
        float3 vToPlane = ((sign(vWorldReflect) * vExtents) - vLocalPos) / vSafeWorldReflect;
        float fPlaneDistance = min(vToPlane.x, min(vToPlane.y, vToPlane.z));
        float3 vHitPoint = vLocalPos + (vWorldReflect * fPlaneDistance);
        
        vColor = SampleEnvMap(FindMap.iIndex, normalize(vHitPoint));
    }
  
    return vColor;
}

float4 Compute_Refract(float4 vWolrdPos, float4 vNormal, float4 vWaterColor, Texture2D<float4> SceneTexture, float fWaterDepth, Texture2D<float4> DepthTexture)
{
    float4 vColor = 0.f;
    
    float4 vLook = normalize(vWolrdPos - g_vCamPosition);
   
    float Eta = 1.f / 1.3f;
    
    float3 vRefract = refract(vLook.xyz, vNormal.xyz, Eta);
    
    float fDepthRatio = saturate(fWaterDepth / g_fMaxDepth);
    
    float fTickness = lerp(g_fMinTickness, g_fMaxTickness, fDepthRatio);
    
    float4 vRefractWorldPos = float4(vWolrdPos.xyz + (vRefract * fTickness), 1.f);
    
    float4x4 matVP = mul(g_CamViewMatrix, g_CamProjMatrix);
    
    float4 vRefractProjPos = mul(vRefractWorldPos, matVP);
    
    float2 vProjXY = vRefractProjPos.xy / vRefractProjPos.w;
    
    float2 vUV = Compute_Texcoord(vProjXY);
    
    float4 vSceneWorldPos = Compute_WorldPos(vUV, DepthTexture);
    
    float3 vDir = normalize(vSceneWorldPos.xyz - vWolrdPos.xyz);
    
    float IsOver = dot(vDir, vRefract) < 0.f;
    
    if(IsOver)
    {
        return vWaterColor;
    }
    
    float4 vRefractvColor = SceneTexture.Sample(DefaultSampler, vUV);
    
    vColor = lerp(vRefractvColor, vWaterColor, fDepthRatio);
    
    return vColor;
}