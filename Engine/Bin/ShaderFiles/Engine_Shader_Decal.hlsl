#include "Engine_Shader_Defines.hlsli"
#include "Engine_Shader_Function.hlsli"

Texture2D<float4> g_DepthTexture : register(t10);
Texture2D<float4> g_DiffuseTexture: register(t11);
Texture2D<float4> g_NormalTexture : register(t12);
Texture2D<float4> g_MaskTexture: register(t13);
Texture2D<float4> g_EmissiveTexture : register(t14);

bool g_HasDiffuse;
bool g_HasNormal;
bool g_HasMask;
bool g_HasEmissive;

float3 g_EmissiveLuminance;

struct VS_IN
{
    float3 vPosition :  POSITION;
    row_major float4x4 World : WORLD;
    row_major float4x4 WorldInv : INVWORLD;
    float fAlpha : TEXCOORD0;
    float4 vColor :     COLOR;
    float fEmissiveIntenisity : TEXCOORD1;
};

struct VS_OUT
{
    float4 vPosition :  SV_POSITION;
    row_major float4x4 WorldInv : INVWORLD;
    float4 vColor : COLOR;
    float fAlpha : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float fEmissiveIntenisity : TEXCOORD2;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT)0;
    
    matrix matWV, matWVP;
    
    matWV = mul(In.World, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
    Out.WorldInv = In.WorldInv;
    Out.fAlpha = In.fAlpha;
    Out.vColor = In.vColor;
    Out.vProjPos = Out.vPosition;
    Out.fEmissiveIntenisity = In.fEmissiveIntenisity;
    
    return Out;
} 

struct PS_IN
{
    float4 vPosition :  SV_POSITION;
    row_major float4x4 WorldInv : INVWORLD;
    float4 vColor :     COLOR;
    float fAlpha : TEXCOORD0;
    float4 vProjPos : TEXCOORD1;
    float fEmissiveIntenisity : TEXCOORD2;
};

struct PS_OUT
{
    float4 vDiffuse :   SV_TARGET0;
    float4 vNormal :    SV_TARGET1;
    float4 vEmissive :  SV_TARGET2;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    float2 vUV = Compute_Texcoord(In.vProjPos.xy / In.vProjPos.w);
    
    float4 vWorldPos = Compute_WorldPos(vUV, g_DepthTexture);
    
    if (false == all(vWorldPos))
        discard;
        
    float4 vLocalPos = mul(vWorldPos, In.WorldInv);
    
    float3 vLocalVolume = abs(vLocalPos.xyz);

    if (any(vLocalVolume > 0.5f))
        discard;
    
    float2 vDecalUV = clamp((vLocalPos.xz) + 0.5f, 0.f, 1.f); // Decal ¹üÀ§ -0.5~0.5;
    
    float4 vDifffuse = 0.f;
    float4 vNormal = 0.f;
    float4 vMask = 0.f;
    float4 vEmissive = 0.f;
    
    if (g_HasDiffuse)
    {
        vDifffuse = g_DiffuseTexture.Sample(DefaultSampler, vDecalUV);
        if (any(vDifffuse.xyz))
        {
            vDifffuse.a = max(max(vDifffuse.r, vDifffuse.b), vDifffuse.g);
        }
        else
            discard;
    }
    
    if (g_HasEmissive)
    {
        vEmissive = g_EmissiveTexture.Sample(DefaultSampler, vDecalUV);
        if (CustomLuminance(vEmissive.xyz, g_EmissiveLuminance) > g_fEmissiveThreshold)
        {
            if (false == g_HasDiffuse)
                Out.vEmissive = float4(In.vColor.xyz * In.fEmissiveIntenisity, In.vColor.a);
            else
                Out.vEmissive = float4(vDifffuse.xyz * In.fEmissiveIntenisity, vDifffuse.a);
        }
    }
    
    if (g_HasNormal)
    {
        vNormal = g_NormalTexture.Sample(DefaultSampler, vDecalUV);
        vNormal.xyz = normalize(vNormal.xyz * 2.f - 1.f);
    }
    
    if (g_HasMask)
    {
        vMask = g_MaskTexture.Sample(DefaultSampler, vDecalUV);
        if (any(vMask.xyz))
            vMask.a = max(max(vMask.r, vMask.b), vMask.g);
        else
            discard;
    }
    
    float4 vColor = any(vDifffuse) ? vDifffuse : In.vColor;
        
    Out.vDiffuse = any(vMask) ? (vColor * vMask) : vColor;
    
    Out.vDiffuse.a = saturate(Out.vDiffuse.a - In.fAlpha);
    
    if(any(vEmissive.xyz))
    {
    
        Out.vEmissive.a = saturate(Out.vEmissive.a - In.fAlpha);
        
        Out.vEmissive.xyz *= Out.vEmissive.a;
    }
    
    Out.vNormal = vNormal;
    return Out;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_None, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
}