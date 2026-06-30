#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float4 g_vDomeColor = 1.f;
float3 g_vCloudColor = 1.f;

Texture2D g_Texture;

Texture2D   g_DiffuseTexture;
Texture2D   g_NormalTexture;
Texture2D   g_MaskTexture;
Texture2D   g_ColorTexture;

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
    VS_OUT Out = (VS_OUT) 0;
    
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

struct PS_OUT_SKYBOX
{
    float4 vColor : SV_TARGET0;
};

PS_OUT_SKYBOX PS_DOME(PS_IN In)
{
    PS_OUT_SKYBOX Out = (PS_OUT_SKYBOX) 0;
    
    float fY = saturate(0.5f + abs(0.5f - In.vTexcoord.x));
    
    fY = min(fY, 0.95f);
    
    float2 vTexcoord = float2(0.5f, fY);
    
    float4 vBackColor = g_Texture.Sample(DefaultSampler, vTexcoord) * 0.2f;
    
    Out.vColor = vBackColor;
    
    return Out;
}

PS_OUT_SKYBOX PS_CLOUD(PS_IN In)
{
    PS_OUT_SKYBOX Out = (PS_OUT_SKYBOX) 0;
    
    float4 vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord);
    
    float fAlpha = vColor.r; //max(vMask.r, max(vMask.b, vMask.g));
    
    Out.vColor = float4(g_vCloudColor, fAlpha); //    lerp(g_vCloudColor, g_vDomeColor, fAlpha);

    return Out;
}

PS_OUT_SKYBOX PS_CLOUD_SEC(PS_IN In)
{
    PS_OUT_SKYBOX Out = (PS_OUT_SKYBOX) 0;
    
    float2 vTexcoord = float2(In.vTexcoord.x + 0.3f, In.vTexcoord.y);
    
    float4 vColor = g_Texture.Sample(DefaultSampler, vTexcoord);
    
    float fAlpha = vColor.r;
    
    Out.vColor = float4(g_vCloudColor, fAlpha);

    return Out;
}


PS_OUT_SKYBOX PS_EFFECT(PS_IN In)
{
    PS_OUT_SKYBOX Out = (PS_OUT_SKYBOX) 0;
    
    float4 vColor = g_Texture.Sample(DefaultSampler, In.vTexcoord); 
    
    Out.vColor = vColor;
    
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
 
    pass Sky_Cloud
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CLOUD();
    }
    
    pass Sky_Cloud_Sec
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CLOUD_SEC();
    }
    
    pass Sky_Effect
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EFFECT();
    }
}