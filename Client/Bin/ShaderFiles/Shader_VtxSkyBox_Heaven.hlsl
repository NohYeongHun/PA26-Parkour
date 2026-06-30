#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

//float4 vCloudColor = float4(0.9137f, 0.7686f, 0.9882f, 1.f);
float4 vCloudColor = float4(0.7647f, 0.4745f, 0.2549f, 1.f);

float g_fTime;

texture2D   g_DiffuseTexture;
texture2D   g_NormalTexture;
texture2D   g_MaskDiffuseTexture;

texture2D   g_MaskTexture : register(t8);

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix));
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
};

struct PS_OUT_SKYBOX
{
    float4 vDiffuse : SV_TARGET0;
};

PS_OUT_SKYBOX PS_DOME(PS_IN In)
{
    PS_OUT_SKYBOX Out = (PS_OUT_SKYBOX) 0;

    Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord) * 0.5f;
    //Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    return Out;
}

PS_OUT_SKYBOX PS_CLOUD(PS_IN In)
{
    PS_OUT_SKYBOX Out = (PS_OUT_SKYBOX) 0;
    
    float fMask = 0.f;
    
    fMask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord).r;
    
    Out.vDiffuse = ((vCloudColor) * fMask) * 1.5f;
//    Out.vDiffuse = ((vCloudColor * 0.2f) * fMask);
    return Out;
}

PS_OUT_SKYBOX PS_EFFECT(PS_IN In)
{
    PS_OUT_SKYBOX Out = (PS_OUT_SKYBOX) 0;

    float4 vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexcoord);
    
    float fAlpha = smoothstep(0.1f, 1.f, max(max(vColor.r, vColor.g), vColor.b));
    
    float3 vFinalColor = (vCloudColor.xyz * vColor.xyz);
    
    Out.vDiffuse = float4(vFinalColor * 2.5f, vColor.a);
    
    return Out;
}

technique11 DefaultTechnique
{
    pass Dome         //0
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_DOME();
    }

    pass Cloud // 1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CLOUD();
    }
    
    pass FX // 2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EFFECT();
    }
}