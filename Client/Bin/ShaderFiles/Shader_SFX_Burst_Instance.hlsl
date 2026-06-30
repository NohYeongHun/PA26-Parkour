#include "Engine_Shader_Defines.hlsli"

matrix g_ViewMatrix, g_ProjMatrix;

float3 g_vColor;
float g_fMaxRadian;

Texture2D g_MaskTexture;

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD0;
    row_major float4x4 World : WORLD;
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
    
    matWV = mul(In.World, g_ViewMatrix);
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

PS_OUT_SFX PS_GALBRENA_SLASH(PS_IN In)
{
    PS_OUT_SFX Out = (PS_OUT_SFX) 0;
    
    float2 vPivot = float2(0.5f, 1.f);
    float2 vDir = normalize(In.vTexcoord - vPivot);
    float2 vNormal = normalize(float2(0.f, -1.f));
    
    float fDot = dot(vDir, vNormal);
    
    if (fDot < cos(g_fMaxRadian))
        discard;
    
    Out.vColor.xyz = g_vColor.xyz;
    
    Out.vColor.a = 1.f;
    
    Out.vEmissive = Out.vColor;
    
    return Out;
}

PS_OUT_SFX PS_GALBRENA_STAR(PS_IN In)
{
    PS_OUT_SFX Out = (PS_OUT_SFX) 0;

    float4 vMask = g_MaskTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float4 vSFXColor = 0.f;
    
    vSFXColor.xyz = g_vColor * (vMask.xyz);
    vSFXColor.a = max(max(vSFXColor.r, vSFXColor.g), vSFXColor.b);
    
    Out.vColor = vSFXColor;
    
    Out.vEmissive = Out.vColor;
    
    return Out;
}

technique11 DefaultTechnique
{
    pass GalbrenaSlash // 0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GALBRENA_SLASH();
    }
    
    pass GalbrenaStar
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_GALBRENA_STAR();
    }
}
