#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

matrix g_BoneMatrices[512];

float4 g_vTrailColor = 1.f;
float2 g_vLifeTime;

float4 g_CamPos;

cbuffer GlobalConstants
{
    int g_iNumBlendWeightsToUse = 2; 
}

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    uint4 vBlendIndex : BLENDINDEX;
    float4 vBlendWeight : BLENDWEIGHT;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float4 vWorldPos : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    matrix matBone, matBW, matWV, matWVP;

    float fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);
    matBone =
    g_BoneMatrices[In.vBlendIndex.x] * In.vBlendWeight.x +
    g_BoneMatrices[In.vBlendIndex.y] * In.vBlendWeight.y +
    g_BoneMatrices[In.vBlendIndex.z] * In.vBlendWeight.z +
    g_BoneMatrices[In.vBlendIndex.w] * fWeightW;
    
    float4 vPosition = mul(float4(In.vPosition, 1.f), matBone);
    float4 vNormal = mul(float4(In.vNormal, 0.f), matBone);
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    
    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;
    Out.vProjPos = Out.vPosition;
    Out.vWorldPos = mul(vPosition, g_WorldMatrix);
    
    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float4 vWorldPos : TEXCOORD2;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vDepth : SV_TARGET1;
    float4 vPBR : SV_TARGET2;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float fAlpha = 1.f - saturate(g_vLifeTime.x / g_vLifeTime.y);
    
    float4 vDiffuse = float4(g_vTrailColor.xyz, fAlpha);
    
    float3 vLook = normalize(g_CamPos.xyz - In.vWorldPos.xyz);
    
    float fRimPower = 0.f;
    
    float fNdoV = dot(normalize(In.vNormal.xyz), vLook);
    
    fRimPower = 1.f - abs(fNdoV);
    
    fRimPower = smoothstep(cos(radians(45.f)), 1.f, fRimPower);
    
//    fRimPower *= 0.5f;
        
    vDiffuse.xyz += vDiffuse.xyz * fRimPower;
    
    Out.vDiffuse = vDiffuse;
    
    Out.vDepth.x = In.vProjPos.z / In.vProjPos.w;
    Out.vDepth.y = In.vProjPos.w;
    Out.vDepth.z = 1.f;
    
    Out.vPBR.z = 1.f;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass Default // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}