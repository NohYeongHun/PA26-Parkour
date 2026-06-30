#include "Engine_Shader_Defines.hlsli"

matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float g_fCloudSpeed;
float2 g_vUVRate = float2(1.f, 1.f);
float3 g_vBackGroundColor = float3(1.f, 1.f, 1.f);
float g_fFXScaleRate; 

texture2D   g_DiffuseTexture[2];
texture2D   g_NormalTexture[2];
texture2D   g_MaskDiffuseTexture;
texture2D   g_MetallicTexture;
vector      g_vMatrlAmbient = vector(1.0f, 1.0f, 1.0f, 1.0f);
vector      g_vMatrlSpecular = vector(0.4f, 0.4f, 0.4f, 0.4f);

texture2D   g_MaskTexture[4] : register(t8);

bool g_HasNormal = false;
bool g_HasMask = false;
bool g_HasMetallic = false;
bool g_IsDynamicObject = false;
int g_iIndex = 0;

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

VS_OUT VS_EFFECT(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    matrix matWV, matWVP;
    
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix));
    Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix));
    Out.vBinormal = normalize(mul(float4(In.vBinormal, 0.f), g_WorldMatrix));
    Out.vTexcoord = In.vTexcoord + (float2(0.5f, 0.5f) - In.vTexcoord) * g_fFXScaleRate;

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
    
    Out.vDiffuse.rgb = g_vBackGroundColor;
    Out.vDiffuse.a = 1.f;

    return Out;
}

PS_OUT_SKYBOX PS_BACKGROUND(PS_IN In)
{
    PS_OUT_SKYBOX Out = (PS_OUT_SKYBOX) 0;
    
    float fLength = length(float2(0.5f, 0.5f) - In.vTexcoord);

    if (fLength > 0.5f)
        Out.vDiffuse.a = 0.f;
    else
    {
        Out.vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord * g_vUVRate);
        Out.vDiffuse.a = Out.vDiffuse.r;
        Out.vDiffuse.rgb += g_vBackGroundColor;
    }

    return Out;
}

PS_OUT_SKYBOX PS_EFFECT(PS_IN In)
{
    PS_OUT_SKYBOX Out = (PS_OUT_SKYBOX) 0;
    
    Out.vDiffuse = g_DiffuseTexture[0].Sample(DefaultSampler, In.vTexcoord);

    Out.vDiffuse.a = Out.vDiffuse.b;
    
    return Out;
}

PS_OUT_SKYBOX PS_CLOUD(PS_IN In)
{
    PS_OUT_SKYBOX Out = (PS_OUT_SKYBOX) 0;
    
    float2 vTexcoord = In.vTexcoord + float2(g_fCloudSpeed, 0.f);
    Out.vDiffuse.rgba = g_DiffuseTexture[0].Sample(DefaultSampler, vTexcoord).b;
    

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

    pass Background //1
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_BACKGROUND();
    }

    pass Effect            //2
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_EFFECT();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_EFFECT();
    }

    pass Cloud          //3
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_CLOUD();
    }
}