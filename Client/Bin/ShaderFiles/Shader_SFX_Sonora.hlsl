#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float g_fIntensity;

float3 g_vColor;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT)0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_SFX
{
    float4 vColor : SV_TARGET0;
    float3 vEmissive : SV_TARGET1;
    float4 vDistortion : SV_TARGET2;
};

PS_OUT_SFX PS_SONORA(PS_IN In)
{
    PS_OUT_SFX Out = (PS_OUT_SFX) 0;
    
    Out.vColor.xyz = g_vColor.xyz;
    
    float fAlpha = g_fIntensity * 0.1f;
    
    Out.vColor.a = fAlpha;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass SonoraChange // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_SONORA();
    }
}
