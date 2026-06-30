#ifndef Engine_Shader_Function_h__
#define Engine_Shader_Function_h__

#include "Engine_Shader_State.hlsli"

static float PI = 3.1415926535f;

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

matrix g_CamViewMatrix, g_CamProjMatrix;

matrix g_ViewMatrixInv, g_ProjMatrixInv;

matrix g_PrevCamViewMatrix, g_PrevCamProjMatrix;

float g_fFar;
vector g_vCamPosition;

float g_fWidth = 1920.f;
float g_fHeight = 1080.f;

float3 g_vFocusPos;
float g_fFocusMinCoc;
float g_fFocusRange;

float Compute_NDF(float NdotH, float Roughness) // ThrowBridgeReitzNormalDistribution   , 미세면 표면의 거칠기 분포
{
    float RoughnessSqr = pow(Roughness, 2.f);                       
    float Distribution = NdotH * NdotH * (RoughnessSqr - 1.f) + 1.f; // 내적(노말, 반사) * 내적(노말, 반사) * ( 거칠기 - 1.f ) + 1.f 
    
    float fDenom = max((PI * Distribution * Distribution), 1e-4f);
    
    float NDF = RoughnessSqr / fDenom;
    
    return NDF;
}

float Compute_GSF(float NdotL, float NdotV, float Roughness) // SchlickGGXGeometricShadowingFunction    , 미세면끼리의 자기 그림자
{
    //float k = Roughness / 2.f;
    
    float k = (Roughness + 1.f) * (Roughness + 1.f) / 8.f;
    
    float fDenomL = NdotL * (1.f - k) + k;
    float fDenomV = NdotV * (1.f - k) + k;
    
    fDenomL = max(fDenomL, 1e-4f);
    fDenomV = max(fDenomV, 1e-4f);
    float SmithL = (NdotL) / fDenomL;
    float SmithV = (NdotV) / fDenomV;
    
    float GS = (SmithL * SmithV);
    
    return GS;
}

float SchlickFresnel(float i)
{
    float x = clamp(1.f - i, 0.f, 1.f);         // 하프 벡터와 Light가 겹칠수록 낮은 수치
    
    return pow(x, 5.f);
}

float3 Compute_Fresnel(float3 vSpecularColor, float LdotH) // SchlickFresnelFunction    , 입사각에 따른 반사되는 비율
{
    return vSpecularColor + (float3(1.f, 1.f, 1.f) - vSpecularColor) * SchlickFresnel(LdotH); // 입사각에 따른 Specular 수치 ( 하프벡터와 Light가 비슷할수록 Specular Down )
}

void Compute_BRDF_PBR(float3 vNormal, float3 vViewDir, float3 vLightDir, float3 vAlbedo, float fMetallic, float fRoughness, out float3 vOutDiffuse, out float3 vOutSpecular) // vViewDir = Look (WorldPos - CamPos)
{
    float3 vHalf = normalize(vViewDir + vLightDir);
    float NdotL = saturate(dot(vNormal, vLightDir));
    float NdotV = saturate(dot(vNormal, vViewDir));
    float NdotH = saturate(dot(vNormal, vHalf));
    float LdotH = saturate(dot(vLightDir, vHalf));
    
    float3 vF0 = 0.04f;
    vF0 = lerp(vF0, vAlbedo, fMetallic);
    
    float3 Fresnel = Compute_Fresnel(vF0, LdotH);                               // LdotH가 크면 수치가 낮음 ( 수치는 F0, Specular Color )
        
    float fClampRoughness = saturate(max(fRoughness, 0.04f));
        
    float GSF = Compute_GSF(NdotL, NdotV, fClampRoughness);
    
    float NDF = Compute_NDF(NdotH, fClampRoughness);
    
    float3 vSpecular = (NDF * GSF * Fresnel) / max(4.f * NdotL * NdotV, 1e-4f);
    
    float3 kd = (1.f - Fresnel) * (1.f - fMetallic);                            // Diffuse 색상에 기여하는 비율 ( 정면 일수록 Diffuse 색)
    
    float3 vDiffuse = kd * vAlbedo / PI;
    
    vOutDiffuse = vDiffuse * NdotL;
    vOutSpecular = vSpecular * NdotL;
    
    //return (vDiffuse + vSpecular) * NdotL;
}

void Compute_Stylized_PBR(float3 vNormal, float3 vViewDir, float3 vLightDir, float3 vAlbedo, float fMetallic, float fRoughness, out float3 vOutDiffuse, out float3 vOutSpecular)
{
    float3 vHalf = normalize(vViewDir + vLightDir);
    float NdotL = saturate(dot(vNormal, vLightDir));
    float NdotV = saturate(dot(vNormal, vViewDir));
    float NdotH = saturate(dot(vNormal, vHalf));
    float LdotH = saturate(dot(vLightDir, vHalf));
    
    float3 vF0 = 0.04f;
    vF0 = lerp(vF0, vAlbedo, fMetallic);
    
    float3 Fresnel = Compute_Fresnel(vF0, LdotH);
        
    float fClampRoughness = max(fRoughness, 0.04f);
    
    float GSF = Compute_GSF(NdotL, NdotV, fClampRoughness);
    
    float NDF = Compute_NDF(NdotH, fClampRoughness);
    
    float3 vSpecular = (NDF * GSF * Fresnel) / max(4.f * NdotL * NdotV, 0.001f);
    
    float3 kd = (1.f - Fresnel) * (1.f - fMetallic);
    
    float3 vDiffuse = kd * vAlbedo / PI;
        
    vOutDiffuse = vDiffuse;
    vOutSpecular = vSpecular;
    //return (vDiffuse + vSpecular);
}

float Compute_RimPower(float3 vNormal, float3 vLook, float NdotL)
{
    float fRimPower = 0.f;
    
    float fNdoV = dot(vNormal, vLook);
    
    fRimPower = abs(fNdoV);
    
    fRimPower = smoothstep(cos(radians(45.f)), cos(radians(90.f)), fRimPower);
    
    fRimPower *= saturate(NdotL);
    fRimPower *= 0.3f;
    
    return fRimPower;
}

float2 Compute_Texcoord(float2 vProjXY)
{
    float2 vTexcoord = 0.f;
    
    vTexcoord.x = vProjXY.x * 0.5f + 0.5f;
    vTexcoord.y = vProjXY.y * -0.5f + 0.5f;
        
    return vTexcoord;
}

float4 Compute_ViewPos(float2 vTexcoord, Texture2D DepthTexture)
{
    float4 vViewPos = 0.f;
    
    vector vDepthDesc = DepthTexture.Sample(PointSampler, vTexcoord);
    
    if(vDepthDesc.y == 0.f)
        return vViewPos;
        
    vViewPos.x = vTexcoord.x * 2.f - 1.f;
    vViewPos.y = vTexcoord.y * -2.f + 1.f;
    vViewPos.z = vDepthDesc.x;
    vViewPos.w = 1.f;
    
    vViewPos = vViewPos * vDepthDesc.y;
    vViewPos = mul(vViewPos, g_ProjMatrixInv);
    
    vViewPos = float4(vViewPos.xyz, 1.f);
    
    return vViewPos;
}

float4 Compute_WorldPos(float2 vTexcoord, Texture2D DepthTexture)
{
    float4 vWorldPos = 0.f;
    
    float4 vViewPos = Compute_ViewPos(vTexcoord, DepthTexture);
    
    if (all(vViewPos) == 0.f)
        return vWorldPos;
    
    vWorldPos = mul(float4(vViewPos.xyz, 1.f), g_ViewMatrixInv);
    
    return float4(vWorldPos.xyz, 1.f);
}

float4 Compute_ViewPos_Sampler(float2 vTexcoord, Texture2D DepthTexture, sampler Sampler)
{
    float4 vViewPos = 0.f;
    
    vector vDepthDesc = DepthTexture.Sample(Sampler, vTexcoord);
    
    vViewPos.x = vTexcoord.x * 2.f - 1.f;
    vViewPos.y = vTexcoord.y * -2.f + 1.f;
    vViewPos.z = vDepthDesc.x;
    vViewPos.w = 1.f;
    
    vViewPos = vViewPos * vDepthDesc.y;
    vViewPos = mul(vViewPos, g_ProjMatrixInv);
    
    return vViewPos;
}

float4 Compute_ViewPos_SSAO(float2 vTexcoord, Texture2D DepthTexture)
{
    float4 vViewPos = 0.f;
    
    vector vDepthDesc = DepthTexture.Sample(DefaultSampler, vTexcoord);
    
    if(vDepthDesc.z == 1.f)
        return vViewPos;
    
    vViewPos.x = vTexcoord.x * 2.f - 1.f;
    vViewPos.y = vTexcoord.y * -2.f + 1.f;
    vViewPos.z = vDepthDesc.x;
    vViewPos.w = 1.f;
    
    vViewPos = vViewPos * vDepthDesc.y;
    vViewPos = mul(vViewPos, g_ProjMatrixInv);
    
    return vViewPos;
}

float4 Compute_Normal(Texture2D NormalTexture, sampler Sampler, float2 vTexcoord)
{
    float4 vNormalDesc = NormalTexture.Sample(Sampler, vTexcoord);
    float3 vNormal = normalize(vNormalDesc.xyz * 2.f - 1.f);
    
    return float4(vNormal, 0.f);
}

bool Outline(sampler Sampler, float2 UV, float fCompareDepth, float fWeight, Texture2D DepthTexture)
{
    float2 vTexelSize = float2((1.f / g_fWidth), (1.f / g_fHeight));
    
    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float2 vOffset = float2(x, y) * (vTexelSize);
            float2 vTexcoord = UV + vOffset;
            vector DepthDesc = DepthTexture.Sample(Sampler, UV + vOffset);
            
            if(DepthDesc.x == 1.f)
                return true;
                
            vector vWorldPos;
            
            vWorldPos.x = vTexcoord.x * 2.f - 1.f;
            vWorldPos.y = vTexcoord.y * -2.f + 1.f;
            vWorldPos.z = DepthDesc.x;
            vWorldPos.w = 1.f;
    
            vWorldPos *= DepthDesc.y;

            vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
                    
            if (abs(fCompareDepth - vWorldPos.z) >= fWeight)
                return true;
        }
    } 
        
    return false;
}


bool Outline_Normal(sampler Sampler, float2 vUV, float3 vCompareNormal, float fWeightRadians, Texture2D NormalTexture)
{
    float2 vTexelSize = float2((1.f / g_fWidth), (1.f / g_fHeight));
    
    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float2 vOffset = float2(x, y) * (vTexelSize);
            float2 vTexcoord = vUV + vOffset;
            float3 NormalDesc = NormalTexture.Sample(Sampler, vUV + vOffset).xyz;
            float3 vNormal = normalize(vector(NormalDesc.xyz * 2.f - 1.f, 0.f));
   
            if (dot(vNormal, vCompareNormal) <= fWeightRadians)
                return true;
        }
    }
        
    return false;
}

float SSAO_Factor(vector vSampleNormal, vector vNoiseVector, vector vViewNormal, vector vViewPos, float fRadius, float fMaxDistance, Texture2D DepthTexture)
{
    float Occlusion = 0.f;
    
    float3 vTangent = normalize(vNoiseVector.xyz - (vViewNormal.xyz * dot(vNoiseVector, vViewNormal)));
    float3 vNormal = vViewNormal.xyz;
    float3 vBinormal = cross(vTangent, vNormal);
    
    float3x3 TBN = float3x3(vTangent, vBinormal, vNormal);
    
    float3 vRandomVector = (mul(vSampleNormal.xyz, TBN) * fRadius);
    
    float4 vSamplePos = vViewPos + float4((vRandomVector * fRadius), 0.f);
    float fRandomZ = vSamplePos.z;
    
    float4 vProjPos = mul(vSamplePos, g_CamProjMatrix);
    
    float2 vSampleUV = Compute_Texcoord((vProjPos.xy / vProjPos.w));
    
    float4 vSampleViewPos = Compute_ViewPos_Sampler(vSampleUV, DepthTexture, PointClampSampler);
    
    float SampleDepth = vSampleViewPos.z; //DepthTexture.Sample(PointClampSampler, vSampleUV).y;
    
    if (SampleDepth == 0.f || SampleDepth >= fRandomZ) // 안그려져있거나, 랜덤 위치보다 뒤에 있다면
        return 1.f;
    
    float fDistance = abs(SampleDepth - vViewPos.z);
    
    if (fDistance > fMaxDistance)
        return 1.f;
        
    float fDistWeight = smoothstep(fMaxDistance, 0.f, fDistance);

    float fNormalWeight = saturate(dot(vViewNormal, normalize(vSampleViewPos - vViewPos)));
    
    Occlusion = fDistWeight * (1.f - fNormalWeight);
    
    return Occlusion;
}

float Compute_COC(float2 vTexcoord, Texture2D DepthTexture)
{
    float4 vViewPos = Compute_ViewPos(vTexcoord, DepthTexture);
    
    float4 vViewCenter = mul(float4(g_vFocusPos, 1.f), g_CamViewMatrix);
    
    float3 vCamDir = vViewCenter.xyz; //float3(0.f, 0.f, g_fFocusDepth);
    float3 vViewDir = vViewPos.xyz - vCamDir;
    
    float fDepth = length(vViewDir);
    
    float fCoc = 0.f;
    
    fCoc = vViewPos.z == 0.f ? 1.f : saturate(fDepth / g_fFocusRange); //abs(fDepth - g_fFocusDepth) / (g_fFocusRange));
    
    return fCoc;
}

float Random(float2 St)
{
    return frac(sin(dot(St.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

float Noise(float2 St)
{
    float2 i = floor(St);
    float2 f = frac(St);
    
    float a = Random(i);
    float b = Random(i + float2(1.0, 0.0));
    float c = Random(i + float2(0.0, 1.0));
    float d = Random(i + float2(1.0, 1.0));
    
    float2 u = f * f * (3.0 - 2.0 * f);
    
    return lerp(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float3 ToneMap(float3 vInput)
{
    float fA = 2.51f;
    float fB = 0.03f;
    float fC = 2.41f;
    float fD = 0.59f;
    float fE = 0.14f;
    
    return saturate((vInput * (fA * vInput + fB)) / (vInput * (fC * vInput + fD) + fE));
}

float Hash21(float2 vInput)
{
    vInput = frac(vInput * float2(123.32, 456.21));
    vInput += dot(vInput, vInput + 45.32);
    return frac(vInput.x * vInput.y);
}

float Hash13(float3 vInput)
{
    vInput = frac(vInput * 0.1031);
    vInput += dot(vInput, vInput.yzx + 33.33);
    return frac((vInput.x + vInput.y) * vInput.z);
}

#endif //Engine_Shader_Function_h__